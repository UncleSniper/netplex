#include "api.h"
#include "driver.h"
#include "tweaks.h"
#include "actions.h"

int nplx_driver_init(
	nplx_driver_t *driver
) {
	int error;
	driver->shutdown = 0;
	error = nplx_fd_hashtable_init(&driver->select_fds, (uint32_t)NETPLEX_SELECT_FDS_HASHTABLE_MODULUS);
	if(error)
		return error;
	nplx_pool_init(&driver->streams);
	nplx_pool_init(&driver->sockets);
	nplx_pool_init(&driver->processes);
	nplx_poll_set_init(&driver->poll_set);
	error = nplx_client_packet_decoder_init(&driver->packet_decoder, 0);
	if(error) {
		nplx_pool_destroy(&driver->streams);
		nplx_pool_destroy(&driver->sockets);
		nplx_pool_destroy(&driver->processes);
		nplx_fd_hashtable_destroy(&driver->select_fds);
		nplx_poll_set_destroy(&driver->poll_set);
		return error;
	}
	driver->packet_buffer = (char*)malloc((size_t)NPLX_SERVER_PACKET_MAX_SIZE);
	if(driver->packet_buffer)
		driver->packet_buffer_size = (uint32_t)NPLX_SERVER_PACKET_MAX_SIZE;
	else
		driver->packet_buffer_size = (uint32_t)0u;
	/* now seed the I/O to stdin/stdout */
	error = nplx_fd_hashtable_put(&driver->select_fds, 0, (uint32_t)0u, nplx_driver_handle_stdin);
	if(error) {
		nplx_driver_destroy(driver);
		return error;
	}
	return 0;
}

void nplx_driver_destroy(
	nplx_driver_t *driver
) {
	nplx_pool_destroy(&driver->streams);
	nplx_pool_destroy(&driver->sockets);
	nplx_pool_destroy(&driver->processes);
	nplx_fd_hashtable_destroy(&driver->select_fds);
	nplx_poll_set_destroy(&driver->poll_set);
	nplx_client_packet_decoder_destroy(&driver->packet_decoder, 0);
}

nplx_error_pump_result_t nplx_driver_pump(
	nplx_driver_t *driver,
	nplx_error_t *error
) {
	int error_code, fd;
	nplx_poll_getptr_t getter;
	nplx_error_pump_result_t total_status, single_status;
	error_code = nplx_poll_set_poll(&driver->poll_set);
	if(error) {
		error->type = NPLX_ERR_DRIVER_POLL;
		error->error_code = error_code;
		return NPLX_DRVR_PUMP_FATAL_ERROR;
	}
	nplx_poll_set_begin_get(&driver->poll_set, &getter);
	total_status = NPLX_DRVR_PUMP_OK;
	while((fd = nplx_poll_set_get(&getter)) != -1) {
		single_status = nplx_driver_pump_fd(driver, fd, error);
		switch(single_status) {
			case NPLX_DRVR_PUMP_OK:
				break;
			case NPLX_DRVR_PUMP_RECOVERABLE_ERROR:
				total_status = NPLX_DRVR_PUMP_RECOVERABLE_ERROR;
				break;
			case NPLX_DRVR_PUMP_FATAL_ERROR:
			default:
				return NPLX_DRVR_PUMP_FATAL_ERROR;
		}
	}
	return total_status;
}

nplx_error_pump_result_t nplx_driver_pump_fd(
	nplx_driver_t *driver,
	int fd,
	nplx_error_t *error
) {
	nplx_error_pump_result_t status;
	status = nplx_fd_hashtable_dispatch(&driver->select_fds, fd, driver, error);
	if(status != NPLX_DRVR_PUMP_OK)
		return status;
	/*TODO: waitpid */
	return NPLX_DRVR_PUMP_OK;
}

nplx_error_pump_result_t nplx_driver_handle_stdin(
	int fd,
	NETPLEX_UNUSED(uint32_t, id),
	struct nplx_driver *driver,
	nplx_error_t *error
) {
	char buffer[NETPLEX_READ_STREAM_BUFFER_SIZE];
	ssize_t count;
	uint32_t size;
	count = read(fd, buffer, sizeof(buffer));
	if(count == (ssize_t)-1) {
		error->type = NPLX_ERR_READ_STDIN;
		error->error_code = errno;
		return NPLX_DRVR_PUMP_FATAL_ERROR;
	}
	if(!count) {
		driver->shutdown = 1;
		if(nplx_client_packet_decoder_is_clean(&driver->packet_decoder))
			return NPLX_DRVR_PUMP_OK;
		error->type = NPLX_ERR_INCOMPLETE_CLIENT_PACKET;
		error->error_code = 0;
		return NPLX_DRVR_PUMP_FATAL_ERROR;
	}
	size = (uint32_t)count;
	do {
		switch(nplx_client_packet_decode(&driver->packet_decoder, buffer, &size)) {
			case NPLX_CLI_PKT_DEC_RSLT_MORE:
				return NPLX_DRVR_PUMP_OK;
			case NPLX_CLI_PKT_DEC_RSLT_DONE:
				switch(nplx_driver_dispatch_client_packet(driver, error)) {
					case NPLX_DRVR_PUMP_OK:
						nplx_client_packet_decoder_destroy(&driver->packet_decoder, 1);
						nplx_client_packet_decoder_init(&driver->packet_decoder, 1);
						break;
					case NPLX_DRVR_PUMP_RECOVERABLE_ERROR:
						/*TODO: print error*/
					case NPLX_DRVR_PUMP_FATAL_ERROR:
					default:
						return NPLX_DRVR_PUMP_FATAL_ERROR;
				}
				break;
			case NPLX_CLI_PKT_DEC_RSLT_FAILED:
			default:
				error->type = NPLX_ERR_INVALID_CLIENT_PACKET;
				error->error_code = 0;
				return NPLX_DRVR_PUMP_FATAL_ERROR;
		}
	} while(size);
	return NPLX_DRVR_PUMP_OK;
}

nplx_error_pump_result_t nplx_driver_dispatch_client_packet(
	nplx_driver_t *driver,
	nplx_error_t *error
) {
	const nplx_client_packet_t *packet;
	packet = driver->packet_decoder.packet;
	switch(packet->opcode) {
		case NPLX_CLI_OPC_PING:
			return nplx_perform_ping(driver, error);
		case NPLX_CLI_OPC_EXEC:
			//TODO
		case NPLX_CLI_OPC_CONNECT:
			//TODO
		case NPLX_CLI_OPC_DISCONNECT:
			//TODO
		case NPLX_CLI_OPC_LISTEN:
			//TODO
		case NPLX_CLI_OPC_SEND:
			//TODO
		case NPLX_CLI_OPC_RECV:
			//TODO
		case NPLX_CLI_OPC_WRITE:
			//TODO
		case NPLX_CLI_OPC_CLOSE:
			//TODO
		default:
			error->type = NPLX_ERR_INVALID_CLIENT_PACKET;
			error->error_code = EINVAL;
			return NPLX_DRVR_PUMP_FATAL_ERROR;
	}
}
