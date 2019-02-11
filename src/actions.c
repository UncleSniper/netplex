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
