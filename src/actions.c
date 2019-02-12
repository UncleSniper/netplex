#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>

#include "actions.h"

nplx_error_pump_result_t nplx_send_server_packet(
	nplx_driver_t *driver,
	const nplx_server_packet_t *packet,
	nplx_error_t *error
) {
	char *old_buffer;
	uint32_t size;
	int error_code;
	ssize_t count;
	old_buffer = driver->packet_buffer;
	error_code = nplx_server_packet_encode(packet, &driver->packet_buffer, &size, old_buffer,
			driver->packet_buffer_size);
	if(error) {
		error->type = NPLX_ERR_CANNOT_ENCODE_SERVER_PACKET;
		error->error_code = error_code;
		return NPLX_DRVR_PUMP_RECOVERABLE_ERROR;
	}
	if(driver->packet_buffer != old_buffer)
		driver->packet_buffer_size = size;
	while(size) {
		count = write(1, driver->packet_buffer, (size_t)size);
		if(count == (ssize_t)-1) {
			error->type = NPLX_ERR_WRITE_STDOUT;
			error->error_code = errno;
			return NPLX_DRVR_PUMP_FATAL_ERROR;
		}
		size -= (uint32_t)count;
	}
	return NPLX_DRVR_PUMP_OK;
}

nplx_error_pump_result_t nplx_perform_ping(
	nplx_driver_t *driver,
	nplx_error_t *error
) {
	nplx_server_packet_t packet;
	packet.opcode = NPLX_SVR_OPC_PONG;
	return nplx_send_server_packet(driver, &packet, error);
}

nplx_error_pump_result_t nplx_perform_connect(
	nplx_driver_t *driver,
	const nplx_client_connect_packet_t *spec,
	nplx_error_t *error
) {
	int fd, error_code;
	struct sockaddr_in addr;
	nplx_server_packet_t packet;
	nplx_socket_t *sock;
	uint32_t sock_id, in_id, out_id;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		packet.packet.failed.error_code = (int32_t)errno;
	  dietail:
		packet.opcode = NPLX_SVR_OPC_CONNECT_FAILED;
		return nplx_send_server_packet(driver, &packet, error);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = (in_port_t)spec->port;
	addr.sin_addr.s_addr = spec->host;
	if(connect(fd, (const struct sockaddr*)&addr, (socklen_t)sizeof(addr))) {
		packet.packet.failed.error_code = (int32_t)errno;
	  custdie:
		close(fd);
		goto dietail;
	}
	sock = (nplx_socket_t*)malloc(sizeof(nplx_socket_t));
	if(!sock) {
		packet.packet.failed.error_code = (int32_t)ENOMEM;
		goto custdie;
	}
	error_code = nplx_socket_deep_init(sock, fd);
	if(error_code) {
		free(sock);
	  subdie:
		packet.packet.failed.error_code = (int32_t)error_code;
		goto custdie;
	}
	error_code = nplx_pool_alloc(&driver->sockets, (nplx_poolable_t*)sock, &sock_id);
	if(error_code) {
	  sockdie:
		nplx_socket_destroy(sock);
		free(sock);
		goto subdie;
	}
	error_code = nplx_pool_alloc(&driver->streams, (nplx_poolable_t*)sock->input_stream, &in_id);
	if(error_code) {
	  insdie:
		nplx_pool_free(&driver->sockets, sock_id, 0);
		goto sockdie;
	}
	error_code = nplx_pool_alloc(&driver->streams, (nplx_poolable_t*)sock->output_stream, &out_id);
	if(error_code) {
	  outsdie:
		nplx_pool_free(&driver->streams, in_id, NPLX_POOL_FREE_DELETE);
		goto insdie;
	}
	error_code = nplx_fd_hashtable_put(&driver->select_fds, fd, in_id, nplx_driver_handle_stream_read);
	if(error_code) {
	  htbldie:
		nplx_pool_free(&driver->streams, out_id, NPLX_POOL_FREE_DELETE);
		goto outsdie;
	}
	error_code = nplx_poll_set_append(&driver->poll_set, fd);
	if(error_code) {
		nplx_fd_hashtable_erase(&driver->select_fds, fd);
		goto htbldie;
	}
	packet.opcode = NPLX_SVR_OPC_CONNECT_OK;
	packet.packet.connect_ok.socket = sock_id;
	packet.packet.connect_ok.in_stream = in_id;
	packet.packet.connect_ok.out_stream = out_id;
	return nplx_send_server_packet(driver, &packet, error);
}
