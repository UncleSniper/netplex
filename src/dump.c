#include <stdio.h>

#include "dump.h"

#define INDENT "    "

void nplx_dump_client_packet(
	nplx_client_packet_t *packet
) {
	unsigned u;
	switch(packet->opcode) {
		case NPLX_CLI_OPC_PING:
			fprintf(stderr, "ping\n");
			break;
		case NPLX_CLI_OPC_EXEC:
			fprintf(stderr, "exec {\n");
			fprintf(stderr, INDENT "argv = [");
			for(u = 0u; packet->packet.exec.argv[u]; ++u)
				fprintf(stderr, ",\n" INDENT INDENT "\"%s\"" + !u, packet->packet.exec.argv[u]);
			if(u)
				fprintf(stderr, "\n" INDENT);
			fprintf(stderr, "]\n");
			fprintf(stderr, INDENT "flags = 0x%04X\n", (unsigned)packet->packet.exec.flags);
			break;
		case NPLX_CLI_OPC_CONNECT:
			/*TODO*/
		case NPLX_CLI_OPC_DISCONNECT:
			/*TODO*/
		case NPLX_CLI_OPC_LISTEN:
			/*TODO*/
		case NPLX_CLI_OPC_SEND:
			/*TODO*/
		case NPLX_CLI_OPC_RECV:
			/*TODO*/
		case NPLX_CLI_OPC_WRITE:
			/*TODO*/
		case NPLX_CLI_OPC_CLOSE:
			/*TODO*/
		default:
			fprintf(stderr, "<illegal opcode %d in client packet>\n", (int)packet->opcode);
			break;
	}
}

void nplx_dump_server_packet(
	nplx_server_packet_t *packet
);
