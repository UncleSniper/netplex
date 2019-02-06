#include "driver.h"
#include "tweaks.h"

int nplx_driver_init(
	nplx_driver_t *driver
) {
	int error;
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
