#define _DEFAULT_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>

#include "packet.h"

void nplx_client_packet_delete(
	nplx_client_packet_t *packet
) {
	nplx_client_packet_destroy(packet);
	free(packet);
}

void nplx_client_packet_destroy(
	nplx_client_packet_t *packet
) {
	char **strings;
	switch(packet->opcode) {
		case NPLX_CLI_OPC_PING:
		case NPLX_CLI_OPC_CONNECT:
		case NPLX_CLI_OPC_DISCONNECT:
		case NPLX_CLI_OPC_LISTEN:
		case NPLX_CLI_OPC_CLOSE:
		default:
			break;
		case NPLX_CLI_OPC_EXEC:
			strings = packet->packet.exec.argv;
			if(strings) {
				for(; *strings; ++strings)
					free(*strings);
				free(packet->packet.exec.argv);
			}
			break;
		case NPLX_CLI_OPC_SEND:
			free(packet->packet.send.path);
			break;
		case NPLX_CLI_OPC_RECV:
			free(packet->packet.recv.path);
			break;
		case NPLX_CLI_OPC_WRITE:
			free(packet->packet.write.data);
	}
}

int nplx_server_packet_encode(
	const nplx_server_packet_t *packet,
	char **data,
	uint32_t *size,
	char *reuse_buffer,
	uint32_t reuse_size
) {
	switch(packet->opcode) {
#define fixed(opc, sz) \
		case NPLX_SVR_OPC_ ## opc: \
			*size = (uint32_t)(sz); \
			break;
		fixed(PONG, 1)
		fixed(EXEC_OK, 1 + 4 * 4)
		fixed(CONNECT_OK, 1 + 3 * 4)
		fixed(DISCONNECT_OK, 1)
		fixed(LISTEN_OK, 1 + 4)
		fixed(SEND_OK, 1 + 4)
		fixed(RECV_OK, 1 + 4)
		fixed(WRITE_OK, 1)
		fixed(CLOSE_OK, 1)
		case NPLX_SVR_OPC_READ_DATA:
			*size = (uint32_t)((uint32_t)(1 + 4 + 2) + packet->packet.read_data.size);
			break;
		fixed(EXIT, 1 + 4 + 2)
		fixed(ACCEPT_CLIENT, 1 + 4 * 4)
		case NPLX_SVR_OPC_ACCEPT_FAILED:
		case NPLX_SVR_OPC_EXEC_FAILED:
		case NPLX_SVR_OPC_CONNECT_FAILED:
		case NPLX_SVR_OPC_DISCONNECT_FAILED:
		case NPLX_SVR_OPC_LISTEN_FAILED:
		case NPLX_SVR_OPC_SEND_FAILED:
		case NPLX_SVR_OPC_RECV_FAILED:
		case NPLX_SVR_OPC_WRITE_FAILED:
		case NPLX_SVR_OPC_CLOSE_FAILED:
		case NPLX_SVR_OPC_READ_FAILED:
			*size = (uint32_t)(1 + 4);
			break;
#undef fixed
		default:
			errno = 0;
			return -1;
	}
	if(reuse_buffer && reuse_size >= *size)
		*data = reuse_buffer;
	else {
		*data = (char*)malloc((size_t)*size);
		if(!*data)
			return -1;
	}
	**data = (char)(unsigned char)(unsigned)packet->opcode;
	switch(packet->opcode) {
#define empty(opc) \
		case NPLX_SVR_OPC_ ## opc: \
			break;
#define byfunc(opc, func) \
		case NPLX_SVR_OPC_ ## opc: \
			nplx_server_packet_encode_ ## func(&packet->packet.func, *data + 1); \
			break;
		empty(PONG)
		byfunc(EXEC_OK, exec_ok)
		byfunc(CONNECT_OK, connect_ok)
		empty(DISCONNECT_OK)
		byfunc(LISTEN_OK, listen_ok)
		byfunc(SEND_OK, send_ok)
		byfunc(RECV_OK, recv_ok)
		empty(WRITE_OK)
		empty(CLOSE_OK)
		byfunc(READ_DATA, read_data)
		byfunc(EXIT, exit)
		byfunc(ACCEPT_CLIENT, accept_client)
		case NPLX_SVR_OPC_ACCEPT_FAILED:
		case NPLX_SVR_OPC_EXEC_FAILED:
		case NPLX_SVR_OPC_CONNECT_FAILED:
		case NPLX_SVR_OPC_DISCONNECT_FAILED:
		case NPLX_SVR_OPC_LISTEN_FAILED:
		case NPLX_SVR_OPC_SEND_FAILED:
		case NPLX_SVR_OPC_RECV_FAILED:
		case NPLX_SVR_OPC_WRITE_FAILED:
		case NPLX_SVR_OPC_CLOSE_FAILED:
		case NPLX_SVR_OPC_READ_FAILED:
			nplx_server_packet_encode_failed(&packet->packet.failed, *data + 1);
			break;
#undef empty
#undef byfunc
		default:
			/* Of course this would be a classic case of 'programmer fsck(8)ed up' at this point... */
			if(*data != reuse_buffer)
				free(*data);
			errno = 0;
			return -1;
	}
	return 0;
}

#define tobe16(member) uint16_t member = htobe16((uint16_t)packet->member)
#define put16(member) do { \
		memcpy(buffer, &member, (size_t)2u); \
		buffer += 2; \
	} while(0)
#define tobe32(member) uint32_t member = htobe32((uint32_t)packet->member)
#define put32(member) do { \
		memcpy(buffer, &member, (size_t)4u); \
		buffer += 4; \
	} while(0)

void nplx_server_packet_encode_failed(
	const nplx_server_failed_packet_t *packet,
	char *buffer
) {
	tobe32(error_code);
	put32(error_code);
}

void nplx_server_packet_encode_exec_ok(
	const nplx_server_exec_ok_packet_t *packet,
	char *buffer
) {
	tobe32(process);
	tobe32(in_stream);
	tobe32(out_stream);
	tobe32(err_stream);
	put32(process);
	put32(in_stream);
	put32(out_stream);
	put32(err_stream);
}

void nplx_server_packet_encode_connect_ok(
	const nplx_server_connect_ok_packet_t *packet,
	char *buffer
) {
	tobe32(socket);
	tobe32(in_stream);
	tobe32(out_stream);
	put32(socket);
	put32(in_stream);
	put32(out_stream);
}

void nplx_server_packet_encode_listen_ok(
	const nplx_server_listen_ok_packet_t *packet,
	char *buffer
) {
	tobe32(socket);
	put32(socket);
}

void nplx_server_packet_encode_send_ok(
	const nplx_server_send_ok_packet_t *packet,
	char *buffer
) {
	tobe32(stream);
	put32(stream);
}

void nplx_server_packet_encode_recv_ok(
	const nplx_server_recv_ok_packet_t *packet,
	char *buffer
) {
	tobe32(stream);
	put32(stream);
}

void nplx_server_packet_encode_read_data(
	const nplx_server_read_data_packet_t *packet,
	char *buffer
) {
	tobe32(stream);
	tobe16(size);
	put32(stream);
	put32(size);
	if(size)
		memcpy(buffer, packet->data, (size_t)packet->size);
}

void nplx_server_packet_encode_exit(
	const nplx_server_exit_packet_t *packet,
	char *buffer
) {
	tobe32(process);
	tobe16(status);
	put32(process);
	put16(status);
}

void nplx_server_packet_encode_accept_client(
	const nplx_server_accept_client_packet_t *packet,
	char *buffer
) {
	tobe32(listen_socket);
	tobe32(client_socket);
	tobe32(in_stream);
	tobe32(out_stream);
	put32(listen_socket);
	put32(client_socket);
	put32(in_stream);
	put32(out_stream);
}

#undef tobe16
#undef put16
#undef tobe32
#undef put32

nplx_client_packet_decoder_t *nplx_client_packet_decoder_new(void) {
	nplx_client_packet_decoder_t *decoder;
	decoder = (nplx_client_packet_decoder_t*)malloc(sizeof(nplx_client_packet_decoder_t));
	if(!decoder)
		return NULL;
	if(nplx_client_packet_decoder_init(decoder, 0)) {
		free(decoder);
		return NULL;
	}
	return decoder;
}

int nplx_client_packet_decoder_init(
	nplx_client_packet_decoder_t *decoder,
	int reuse_packet
) {
	if(!reuse_packet) {
		decoder->packet = (nplx_client_packet_t*)malloc(sizeof(nplx_client_packet_t));
		if(!decoder->packet)
			return -1;
	}
	decoder->top_bytes = (uint32_t)0u;
	decoder->string_done = (uint32_t)0u;
	return 0;
}

void nplx_client_packet_decoder_delete(
	nplx_client_packet_decoder_t *decoder
) {
	nplx_client_packet_decoder_destroy(decoder, 0);
	free(decoder);
}

void nplx_client_packet_decoder_destroy(
	nplx_client_packet_decoder_t *decoder,
	int retain_packet
) {
	if(!retain_packet)
		nplx_client_packet_delete(decoder->packet);
}

nplx_client_packet_decode_result_t nplx_client_packet_decode(
	nplx_client_packet_decoder_t *decoder,
	const char *data,
	uint32_t *size
) {
	if(!decoder->top_bytes) {
		if(!*size)
			return NPLX_CLI_PKT_DEC_RSLT_MORE;
		decoder->packet->opcode = (nplx_client_opcode_t)(unsigned)(unsigned char)*data;
		decoder->top_bytes = (uint32_t)1u;
		--*size;
		++data;
	}
	switch(decoder->packet->opcode) {
#define empty(opc) \
		case NPLX_CLI_OPC_ ## opc: \
			return NPLX_CLI_PKT_DEC_RSLT_DONE;
#define byfunc(opc, func) \
		case NPLX_CLI_OPC_ ## opc: \
			return nplx_client_packet_decode_ ## func(decoder, data, size);
#define byfuncnull(opc, func, member) \
		case NPLX_CLI_OPC_ ## opc: \
			decoder->packet->packet.member = NULL; \
			return nplx_client_packet_decode_ ## func(decoder, data, size);
		empty(PING)
		byfuncnull(EXEC, exec, exec.argv)
		byfunc(CONNECT, connect)
		byfunc(DISCONNECT, disconnect)
		byfunc(LISTEN, listen)
		byfuncnull(SEND, send, send.path)
		byfuncnull(RECV, recv, recv.path)
		byfunc(WRITE, write)
		byfunc(CLOSE, close)
#undef empty
#undef byfunc
#undef byfuncnull
		default:
			errno = 0;
			return NPLX_CLI_PKT_DEC_RSLT_FAILED;
	}
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#define def_client_packet_decoder(type) \
	nplx_client_packet_decode_result_t nplx_client_packet_decode_ ## type( \
		nplx_client_packet_decoder_t *decoder, \
		const char *data, \
		uint32_t *size \
	)

#define decls \
	uint32_t threshold = (uint32_t)1u; \
	uint32_t draw; \
	int just_set;
#define declstrvec \
	decls \
	size_t buf_size;
#define reqprim(bytes, bits, member, type) \
	threshold += (uint32_t)(bytes); \
	if(decoder->top_bytes < threshold) { \
		draw = threshold - decoder->top_bytes; \
		if(draw > *size) \
			draw = *size; \
		if(draw) \
			memcpy((char*)&decoder->member + \
					((uint32_t)(bytes) - (threshold - decoder->top_bytes)), data, (size_t)draw); \
		*size -= draw; \
		decoder->top_bytes += draw; \
		if(decoder->top_bytes < threshold) \
			return NPLX_CLI_PKT_DEC_RSLT_MORE; \
		data += draw; \
		decoder->member = (type)htobe ## bits((uint ## bits ## _t)decoder->member); \
		just_set = 1; \
	} \
	else \
		just_set = 0;
#define reqi16(member) reqprim(2, 16, member, int16_t)
#define requ16(member) reqprim(2, 16, member, uint16_t)
#define reqi32(member) reqprim(4, 32, member, int32_t)
#define requ32(member) reqprim(4, 32, member, uint32_t)
#define reqstr(member) \
	requ16(string_length) \
	if(just_set) { \
		decoder->member = (char*)malloc((size_t)decoder->string_length + (size_t)1u); \
		if(!decoder->member) \
			return NPLX_CLI_PKT_DEC_RSLT_FAILED; \
	} \
	threshold += (uint32_t)decoder->string_length; \
	if(decoder->top_bytes < threshold) { \
		draw = threshold - decoder->top_bytes; \
		if(draw > *size) \
			draw = *size; \
		if(draw) \
			memcpy(decoder->member + (decoder->string_length - (threshold - decoder->top_bytes)), \
					data, (size_t)draw); \
		*size -= draw; \
		decoder->top_bytes += draw; \
		if(decoder->top_bytes < threshold) \
			return NPLX_CLI_PKT_DEC_RSLT_MORE; \
		data += draw; \
		decoder->member[decoder->string_length] = '\0'; \
		just_set = 1; \
	} \
	else \
		just_set = 0;
#define reqstrvec(member) \
	requ16(string_count) \
	if(just_set) { \
		buf_size = ((size_t)decoder->string_count + (size_t)1u) * sizeof(char*); \
		decoder->member = (char**)malloc(buf_size); \
		if(!decoder->member) \
			return NPLX_CLI_PKT_DEC_RSLT_FAILED; \
		memset(decoder->member, 0, buf_size); \
		decoder->string_index = (uint16_t)0u; \
		decoder->string_done = (uint32_t)0u; \
		just_set = !decoder->string_count; \
	} \
	else \
		just_set = 0; \
	threshold += decoder->string_done; \
	while(decoder->string_index < decoder->string_count) { \
		reqstr(member[decoder->string_index]) \
		++decoder->string_index; \
		decoder->string_done += (uint32_t)2u + (uint32_t)decoder->string_length; \
		just_set = decoder->string_index == decoder->string_count; \
	}

def_client_packet_decoder(exec) {
	declstrvec
	reqstrvec(packet->packet.exec.argv)
	requ16(packet->packet.exec.flags)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(connect) {
	decls
	requ32(packet->packet.connect.host)
	requ16(packet->packet.connect.port)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(disconnect) {
	decls
	requ32(packet->packet.disconnect.socket)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(listen) {
	decls
	requ32(packet->packet.listen.bind)
	requ16(packet->packet.listen.port)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(send) {
	decls
	reqstr(packet->packet.send.path)
	requ16(packet->packet.send.flags)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(recv) {
	decls
	reqstr(packet->packet.recv.path)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(write) {
	decls
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

def_client_packet_decoder(close) {
	decls
	requ32(packet->packet.close.stream)
	return NPLX_CLI_PKT_DEC_RSLT_DONE;
}

#undef def_client_packet_decoder
#undef decls
#undef declstrvec
#undef reqprim
#undef reqi16
#undef requ16
#undef reqi32
#undef requ32
#undef reqstr
#undef reqstrvec

#pragma GCC diagnostic pop
