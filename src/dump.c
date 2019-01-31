#include <stdio.h>

#include "dump.h"

#define INDENT "    "
#define SEND_MASK \
	(NPLX_SEND_FL_APPEND \
	| NPLX_SEND_FL_CREATE \
	| NPLX_SEND_FL_EXCL \
	| NPLX_SEND_FL_NOCTTY \
	| NPLX_SEND_FL_SYNC \
	| NPLX_SEND_FL_TRUNC)

#define dumpfl(member, real, cosmetic) \
	if(packet->packet.member & real) { \
		fprintf(stderr, "| " cosmetic + !had * 2); \
		had = 1; \
	}
#define dumpsendfl(real, cosmetic) dumpfl(send.flags, real, cosmetic)

#define dumptriv(opc, name) \
	case opc: \
		fprintf(stderr, #name "\n"); \
		break;

void nplx_dump_packet_bytes(
	const char *bytes,
	uint16_t size
) {
	for(; size; --size, ++bytes) {
		switch(*bytes) {
#define fixed(raw, escaped) \
	case raw: \
		fprintf(stderr, escaped); \
		break;
			fixed('\r', "\\r")
			fixed('\n', "\\n")
			fixed('\b', "\\b")
			fixed('\t', "\\t")
			fixed('\v', "\\v")
			fixed('\f', "\\f")
			fixed('\a', "\\a")
			fixed('\\', "\\\\")
#undef fixed
			default:
				if(*bytes >= ' ' && *bytes <= '~')
					fprintf(stderr, "%c", *bytes);
				else
					fprintf(stderr, "\\x%02X", (unsigned)(unsigned char)*bytes);
				break;
		}
	}
}

void nplx_dump_client_packet(
	nplx_client_packet_t *packet
) {
	unsigned u;
	int had;
	switch(packet->opcode) {
		dumptriv(NPLX_CLI_OPC_PING, ping)
		case NPLX_CLI_OPC_EXEC:
			fprintf(stderr, "exec {\n");
			fprintf(stderr, INDENT "argv = [");
			for(u = 0u; packet->packet.exec.argv[u]; ++u)
				fprintf(stderr, ",\n" INDENT INDENT "\"%s\"" + !u, packet->packet.exec.argv[u]);
			if(u)
				fprintf(stderr, "\n" INDENT);
			fprintf(stderr, "]\n");
			fprintf(stderr, INDENT "flags = 0x%04X\n", (unsigned)packet->packet.exec.flags);
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_CONNECT:
			fprintf(stderr, "connect {\n");
			fprintf(stderr, INDENT "host = 0x%08X\n", (unsigned)packet->packet.connect.host);
			fprintf(stderr, INDENT "port = %u\n", (unsigned)packet->packet.connect.port);
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_DISCONNECT:
			fprintf(stderr, "disconnect {\n");
			fprintf(stderr, INDENT "socket = %u\n", (unsigned)packet->packet.disconnect.socket);
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_LISTEN:
			fprintf(stderr, "listen {\n");
			fprintf(stderr, INDENT "bind = 0x%08X\n", (unsigned)packet->packet.listen.bind);
			fprintf(stderr, INDENT "port = %u\n", (unsigned)packet->packet.listen.port);
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_SEND:
			fprintf(stderr, "send {\n");
			fprintf(stderr, INDENT "path =\"%s\"\n", packet->packet.send.path);
			if(packet->packet.send.flags & SEND_MASK) {
				fprintf(stderr, INDENT "flags = ");
				had = 0;
				dumpsendfl(NPLX_SEND_FL_APPEND, "O_APPEND")
				dumpsendfl(NPLX_SEND_FL_CREATE, "O_CREAT")
				dumpsendfl(NPLX_SEND_FL_EXCL, "O_EXCL")
				dumpsendfl(NPLX_SEND_FL_NOCTTY, "O_NOCTTY")
				dumpsendfl(NPLX_SEND_FL_SYNC, "O_SYNC")
				dumpsendfl(NPLX_SEND_FL_TRUNC, "O_TRUNC")
			}
			else
				fprintf(stderr, INDENT "flags = 0\n");
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_RECV:
			fprintf(stderr, "recv {\n");
			fprintf(stderr, INDENT "path = \"%s\"\n", packet->packet.recv.path);
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_WRITE:
			fprintf(stderr, "write {\n");
			fprintf(stderr, INDENT "stream = %u\n", (unsigned)packet->packet.write.stream);
			fprintf(stderr, INDENT "size = %u\n", (unsigned)packet->packet.write.size);
			fprintf(stderr, INDENT "data = \"");
			nplx_dump_packet_bytes(packet->packet.write.data, packet->packet.write.size);
			fprintf(stderr, "\"\n");
			fprintf(stderr, "}\n");
			break;
		case NPLX_CLI_OPC_CLOSE:
			fprintf(stderr, "close {\n");
			fprintf(stderr, INDENT "stream = %u\n", (unsigned)packet->packet.close.stream);
			fprintf(stderr, "}\n");
			break;
		default:
			fprintf(stderr, "<illegal opcode %d in client packet>\n", (int)packet->opcode);
			break;
	}
}

#define dumpfailed(opc, name) \
	case opc: \
		fprintf(stderr, #name " {\n"); \
		fprintf(stderr, INDENT "error_code = %d\n", (int)packet->packet.failed.error_code); \
		fprintf(stderr, "}\n"); \
		break;

void nplx_dump_server_packet(
	nplx_server_packet_t *packet
) {
	switch(packet->opcode) {
		dumptriv(NPLX_SVR_OPC_PONG, pong)
		case NPLX_SVR_OPC_EXEC_OK:
			fprintf(stderr, "exec_ok {\n");
			fprintf(stderr, INDENT "process = %u\n", (unsigned)packet->packet.exec_ok.process);
			fprintf(stderr, INDENT "in_stream = %u\n", (unsigned)packet->packet.exec_ok.in_stream);
			fprintf(stderr, INDENT "out_stream = %u\n", (unsigned)packet->packet.exec_ok.out_stream);
			fprintf(stderr, INDENT "err_stream = %u\n", (unsigned)packet->packet.exec_ok.err_stream);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_EXEC_FAILED, exec_failed)
		case NPLX_SVR_OPC_CONNECT_OK:
			fprintf(stderr, "connect_ok {\n");
			fprintf(stderr, INDENT "socket = %u\n", (unsigned)packet->packet.connect_ok.socket);
			fprintf(stderr, INDENT "in_stream = %u\n", (unsigned)packet->packet.connect_ok.in_stream);
			fprintf(stderr, INDENT "out_stream = %u\n", (unsigned)packet->packet.connect_ok.out_stream);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_CONNECT_FAILED, connect_failed)
		dumptriv(NPLX_SVR_OPC_DISCONNECT_OK, disconnect_ok)
		dumpfailed(NPLX_SVR_OPC_DISCONNECT_FAILED, "disconnect_failed")
		case NPLX_SVR_OPC_LISTEN_OK:
			fprintf(stderr, "listen_ok {\n");
			fprintf(stderr, INDENT "socket = %u\n", (unsigned)packet->packet.listen_ok.socket);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_LISTEN_FAILED, listen_failed)
		case NPLX_SVR_OPC_SEND_OK:
			fprintf(stderr, "send_ok {\n");
			fprintf(stderr, INDENT "stream = %u\n", (unsigned)packet->packet.send_ok.stream);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_SEND_FAILED, send_failed)
		case NPLX_SVR_OPC_RECV_OK:
			fprintf(stderr, "recv_ok {\n");
			fprintf(stderr, INDENT "stream = %u\n", (unsigned)packet->packet.recv_ok.stream);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_RECV_FAILED, recv_failed)
		dumptriv(NPLX_SVR_OPC_WRITE_OK, write_ok)
		dumpfailed(NPLX_SVR_OPC_WRITE_FAILED, write_failed)
		dumptriv(NPLX_SVR_OPC_CLOSE_OK, close_ok)
		dumpfailed(NPLX_SVR_OPC_CLOSE_FAILED, close_failed)
		case NPLX_SVR_OPC_READ_DATA:
			fprintf(stderr, "read_data {\n");
			fprintf(stderr, INDENT "stream = %u\n", (unsigned)packet->packet.read_data.stream);
			fprintf(stderr, INDENT "size = %u\n", (unsigned)packet->packet.read_data.size);
			fprintf(stderr, INDENT "data = \"");
			nplx_dump_packet_bytes(packet->packet.read_data.data, packet->packet.read_data.size);
			fprintf(stderr, "\"\n");
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_READ_FAILED, read_failed)
		case NPLX_SVR_OPC_EXIT:
			fprintf(stderr, "exit {\n");
			fprintf(stderr, INDENT "process = %u\n", (unsigned)packet->packet.exit.process);
			fprintf(stderr, INDENT "status = %u\n", (unsigned)packet->packet.exit.status);
			fprintf(stderr, "}\n");
			break;
		case NPLX_SVR_OPC_ACCEPT_CLIENT:
			fprintf(stderr, "accept_client {\n");
			fprintf(stderr, INDENT "listen_socket = %u\n", (unsigned)packet->packet.accept_client.listen_socket);
			fprintf(stderr, INDENT "client_socket = %u\n", (unsigned)packet->packet.accept_client.client_socket);
			fprintf(stderr, INDENT "in_stream = %u\n", (unsigned)packet->packet.accept_client.in_stream);
			fprintf(stderr, INDENT "out_stream = %u\n", (unsigned)packet->packet.accept_client.out_stream);
			fprintf(stderr, "}\n");
			break;
		dumpfailed(NPLX_SVR_OPC_ACCEPT_FAILED, accept_failed)
		default:
			fprintf(stderr, "<illegal opcode %d in server packet>\n", (int)packet->opcode);
			break;
	}
}
