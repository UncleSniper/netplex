#ifndef NETPLEX_ERROR_H
#define NETPLEX_ERROR_H

typedef enum nplx_error_pump_result {
	NPLX_DRVR_PUMP_OK,
	NPLX_DRVR_PUMP_RECOVERABLE_ERROR,
	NPLX_DRVR_PUMP_FATAL_ERROR
} nplx_error_pump_result_t;

typedef enum nplx_error_type {
	NPLX_ERR_DRIVER_POLL,
	NPLX_ERR_FD_DISPATCH,
	NPLX_ERR_READ_STDIN,
	NPLX_ERR_INCOMPLETE_CLIENT_PACKET,
	NPLX_ERR_INVALID_CLIENT_PACKET,
	NPLX_ERR_CANNOT_ENCODE_SERVER_PACKET,
	NPLX_ERR_WRITE_STDOUT,
	NPLX_ERR_ACTIVITY_ON_UNKNOWN_STREAM,
	NPLX_ERR_READING_OUTPUT_STREAM,
	NPLX_ERR_CLOSE_STREAM
} nplx_error_type_t;

typedef struct nplx_error_poolable {
	uint32_t id;
} nplx_error_poolable_t;

typedef struct nplx_error {
	nplx_error_type_t type;
	int error_code;
	union {
		nplx_error_poolable_t poolable;
	} details;
} nplx_error_t;

#endif /* NETPLEX_ERROR_H */
