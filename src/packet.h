#ifndef NETPLEX_PACKET_H
#define NETPLEX_PACKET_H

#include <stdint.h>

#include "opcode.h"

#define NPLX_SEND_FL_APPEND 001
#define NPLX_SEND_FL_CREATE 002
#define NPLX_SEND_FL_EXCL   004
#define NPLX_SEND_FL_NOCTTY 010
#define NPLX_SEND_FL_SYNC   020
#define NPLX_SEND_FL_TRUNC  040

#define NPLX_EXEC_FL_ATTACH_STDIN  01
#define NPLX_EXEC_FL_ATTACH_STDOUT 02
#define NPLX_EXEC_FL_ATTACH_STDERR 04

typedef struct {
	char **argv;
	uint16_t flags;
} nplx_client_exec_packet_t;

typedef struct {
	uint32_t host;
	uint16_t port;
} nplx_client_connect_packet_t;

typedef struct {
	uint32_t socket;
} nplx_client_disconnect_packet_t;

typedef struct {
	uint32_t bind;
	uint16_t port;
} nplx_client_listen_packet_t;

typedef struct {
	char *path;
	uint16_t flags;
} nplx_client_send_packet_t;

typedef struct {
	char *path;
} nplx_client_recv_packet_t;

typedef struct {
	uint32_t stream;
	uint16_t size;
	char *data;
} nplx_client_write_packet_t;

typedef struct {
	uint32_t stream;
} nplx_client_close_packet_t;

typedef struct {
	nplx_client_opcode_t opcode;
	union {
		nplx_client_exec_packet_t exec;
		nplx_client_connect_packet_t connect;
		nplx_client_disconnect_packet_t disconnect;
		nplx_client_listen_packet_t listen;
		nplx_client_send_packet_t send;
		nplx_client_recv_packet_t recv;
		nplx_client_write_packet_t write;
		nplx_client_close_packet_t close;
	} packet;
} nplx_client_packet_t;

typedef struct {
	int32_t error_code;
} nplx_server_failed_packet_t;

typedef struct {
	uint32_t process;
	uint32_t in_stream;
	uint32_t out_stream;
	uint32_t err_stream;
} nplx_server_exec_ok_packet_t;

typedef struct {
	uint32_t socket;
	uint32_t in_stream;
	uint32_t out_stream;
} nplx_server_connect_ok_packet_t;

typedef struct {
	uint32_t socket;
} nplx_server_listen_ok_packet_t;

typedef struct {
	uint32_t stream;
} nplx_server_send_ok_packet_t;

typedef struct {
	uint32_t stream;
} nplx_server_recv_ok_packet_t;

typedef struct {
	uint32_t stream;
	uint16_t size;
	char *data;
} nplx_server_read_data_packet_t;

typedef struct {
	uint32_t process;
	int16_t status;
} nplx_server_exit_packet_t;

typedef struct {
	uint32_t listen_socket;
	uint32_t client_socket;
	uint32_t in_stream;
	uint32_t out_stream;
} nplx_server_accept_client_packet_t;

typedef struct {
	nplx_server_opcode_t opcode;
	union {
		nplx_server_failed_packet_t failed;
		nplx_server_exec_ok_packet_t exec_ok;
		nplx_server_connect_ok_packet_t connect_ok;
		nplx_server_listen_ok_packet_t listen_ok;
		nplx_server_send_ok_packet_t send_ok;
		nplx_server_recv_ok_packet_t recv_ok;
		nplx_server_read_data_packet_t read_data;
		nplx_server_exit_packet_t exit;
		nplx_server_accept_client_packet_t accept_client;
	} packet;
} nplx_server_packet_t;

typedef struct {
	nplx_client_packet_t *packet;
	uint32_t top_bytes;
	uint16_t string_count;
	uint16_t string_length;
	uint16_t string_index;
	uint32_t string_done;
	char piece[4];
} nplx_client_packet_decoder_t;

typedef enum {
	NPLX_CLI_PKT_DEC_RSLT_MORE,
	NPLX_CLI_PKT_DEC_RSLT_DONE,
	NPLX_CLI_PKT_DEC_RSLT_FAILED
} nplx_client_packet_decode_result_t;

void nplx_client_packet_delete(
	nplx_client_packet_t *packet
);

void nplx_client_packet_destroy(
	nplx_client_packet_t *packet
);

int nplx_server_packet_encode(
	const nplx_server_packet_t *packet,
	char **data,
	uint32_t *size,
	char *reuse_buffer,
	uint32_t reuse_size
);

void nplx_server_packet_encode_failed(
	const nplx_server_failed_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_exec_ok(
	const nplx_server_exec_ok_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_connect_ok(
	const nplx_server_connect_ok_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_listen_ok(
	const nplx_server_listen_ok_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_send_ok(
	const nplx_server_send_ok_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_recv_ok(
	const nplx_server_recv_ok_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_read_data(
	const nplx_server_read_data_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_exit(
	const nplx_server_exit_packet_t *packet,
	char *buffer
);

void nplx_server_packet_encode_accept_client(
	const nplx_server_accept_client_packet_t *packet,
	char *buffer
);

nplx_client_packet_decoder_t *nplx_client_packet_decoder_new(void);

int nplx_client_packet_decoder_init(
	nplx_client_packet_decoder_t *decoder,
	int reuse_packet
);

void nplx_client_packet_decoder_delete(
	nplx_client_packet_decoder_t *decoder
);

void nplx_client_packet_decoder_destroy(
	nplx_client_packet_decoder_t *decoder,
	int retain_packet
);

nplx_client_packet_decode_result_t nplx_client_packet_decode(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_exec(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_connect(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_disconnect(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_listen(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_send(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_recv(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_write(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

nplx_client_packet_decode_result_t nplx_client_packet_decode_close(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
);

#endif /* NETPLEX_PACKET_H */
