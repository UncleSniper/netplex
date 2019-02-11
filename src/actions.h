#ifndef NETPLEX_ACTIONS_H
#define NETPLEX_ACTIONS_H

#include "driver.h"

nplx_error_pump_result_t nplx_send_server_packet(
	nplx_driver_t *driver,
	const nplx_server_packet_t *packet,
	nplx_error_t *error
);

nplx_error_pump_result_t nplx_perform_ping(
	nplx_driver_t *driver,
	nplx_error_t *error
);

#endif /* NETPLEX_ACTIONS_H */
