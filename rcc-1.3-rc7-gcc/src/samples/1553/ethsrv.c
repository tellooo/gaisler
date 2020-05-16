#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <rtems.h>

#include "ethsrv.h"

struct bm_eth_server
{
	rtems_id taskEthid;
	rtems_name taskEthname;

	unsigned int debug_log[50000];
	unsigned int debug_index;

	int ssock;
	int sock;

	server_get_func get;
	void *arg;

	volatile int client_avail;
};


void debug_add(bm_eth_server_t eth, unsigned int word)
{
	if ( eth->debug_index >= 50000 )
		eth->debug_index = 0;
	eth->debug_log[eth->debug_index++] = word;
}

int cmd_get_log(bm_eth_server_t eth, struct cmd_get_log *arg)
{
	static struct cmd_resp_get_log resp;
	int length;

	/* Only one device supported */
	if ( arg->devno != 0 ) {
		return -1;
	}

	/* Prepare Response */
	resp.hdr.cmdno = arg->hdr.cmdno;
	resp.devno = arg->devno;
	resp.status = 0;
	resp.log_cnt = eth->get(eth->arg, &resp.log[0], 250);

	debug_add(eth, resp.log_cnt);

	length = offsetof(struct cmd_resp_get_log, log) +
		resp.log_cnt*sizeof(unsigned int);
	resp.hdr.length = length - sizeof(struct cmd_hdr);

	/* Send back result */
	if ( write(eth->sock, &resp, length) != length ) {
		return -1;
	}

	debug_add(eth, length);

	return 0;
}

int server_init(bm_eth_server_t eth, const char *host, int port)
{
	int status;
	char name[255];
	struct sockaddr_in addr;
	struct hostent *he;
	int optval;

	eth->ssock = socket(AF_INET, SOCK_STREAM, 0);
	if ( eth->ssock < 0 ) {
		printf("ERROR CREATING SERVER SOCKET: %d, %d (%s)\n", eth->ssock, errno, strerror(errno));
		return -1;
	}

	optval = 1;
	if ( setsockopt(eth->ssock, SOL_SOCKET, SO_REUSEADDR, &optval, 4) < 0 ) {
		printf("setsockopt: %d (%s)\n", errno, strerror(errno));
	}

	memset(&addr, 0, sizeof(addr));
	
	if ( host ) {
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = inet_addr(host);
	} else {
		gethostname(name, 255);
		he = gethostbyname(name);
		if ( he == NULL ) {
			printf("ERROR Failed to get host!\n");
			return -2;
		}
		addr.sin_family = he->h_addrtype;
	}
	addr.sin_port = htons(port);

	status = bind(eth->ssock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if ( status < 0 ) {
		close(eth->ssock);
		printf("ERROR Failed to bind socket: %d, %d (%s)\n", eth->ssock, errno, strerror(errno));
		return -3;
	}

	status = listen(eth->ssock, 2);
	if ( status < 0 ) {
		close(eth->ssock);
		printf("ERROR Failed to listen to socket: %d, %d (%s)\n", eth->ssock, errno, strerror(errno));
		return -4;
	}

	return 0;
}

int server_wait_client(bm_eth_server_t eth)
{
	/* Block waiting for new clients */
	eth->sock = accept(eth->ssock, NULL, NULL);
	if ( eth->sock < 0 ) {
		printf("Failed to accept incomming client\n");
		close(eth->ssock);
		eth->ssock = -1;
		return -1;
	}
	return 0;
}

/* TCP/IP server loop, answers client's requests. It executes
 * until an error is detected or until the client disconnect.
 */
int server_loop(bm_eth_server_t eth)
{
	int err;
	int len;
	static unsigned int buf[256];
	struct cmd_hdr *hdr;

	err = 0;
	while ( err == 0 ) {

		/* Let other task have cpu between every request */
		sched_yield();

		len = read(eth->sock, buf, sizeof(struct cmd_hdr));
		if ( len <= 0 ) {
			break;
		}

		hdr = (struct cmd_hdr *)&buf[0];
		if ( (hdr->cmdno == 0)  || (hdr->cmdno > MAX_COMMAND_NUM) ) {
			printf("Invalid command number\n");
			break;
		}

		if ( hdr->length > MAX_COMMAND_SIZE ) {
			printf("Invalid length of command: %d (MAX: %d) C:%d\n",
				hdr->length, MAX_COMMAND_SIZE, hdr->cmdno);
			break;
		}

		if ( hdr->length > 0 ) {
			/* This is not fail safe, should loop around read() until
			 * a complete command has been read.
			 */
			len = read(eth->sock, (void *)(hdr+1), hdr->length);
			if ( len < 0 ) {
				break;
			} else if ( len != hdr->length ) {
				printf("Invalid read command length\n");
				break;
			}
		}

		/* Command handling */
		switch ( hdr->cmdno ) {

			case CMD_STATUS:
			{
				/* Not implemented */
				err = 1;
				break;
			}

			case CMD_GET_INFO:
			{
				/* Not implmented */
				err = 1;
				break;
			}

			case CMD_GET_LOG:
			{
				err = cmd_get_log(eth, (struct cmd_get_log *)hdr);
				break;
			}

			default:
				err = 1;
				break;
		}
	}

	return 0;
}

/* Ethernet TCP/IP Server Task */
void task_bm_eth_server(rtems_task_argument argument)
{
	int err = 0;
	bm_eth_server_t eth = (bm_eth_server_t)argument;
	printf("ETH Server task started\n");

	while ( 1 ) {

		/* Wait for a new client to connect to ETH server */
		printf("ETH: Waiting for remote client to connect\n");

		err = server_wait_client(eth);
		if ( err ) {
			printf("ETH: Ethernet server wait client failure: %d\n", err);
			break;
		}

		eth->client_avail = 1;

		/* A new client connected, ready to be served */
		printf("ETH: Client connected to ETH Server\n");

		err = server_loop(eth);
		eth->client_avail = 0;
		if ( err ) {
			printf("ETH: Ethernet server loop failure: %d\n", err);
			break;
		}

		/* A client has disconnected, wait for a new connection */
		printf("ETH: Client disconnected from ETH Server\n");
	}

	if (err) {
		/* Stop any open socket befor quitting */
		server_stop(eth);

		/* Failure */
		exit(-1);
	}
}

/* Setup Ethernet TCP/IP server */
int server_setup(bm_eth_server_t *eth, const char *host, int port, int wait_for_client, server_get_func get, void *data)
{
	rtems_status_code status;
	int err;


	*eth = calloc(1, sizeof(struct bm_eth_server));
	if (*eth == NULL) {
		printf("ERROR ALLOCATING SERVER\n");
		return -1;
	}

	/* Create Ethernet Server task */
	(*eth)->taskEthname = rtems_build_name( 'S', 'E', 'R', 'V' );

	status = rtems_task_create (
			(*eth)->taskEthname,
			1,
			64*1024,
			0,
			RTEMS_LOCAL | RTEMS_FLOATING_POINT,
			&(*eth)->taskEthid);
	if (status != RTEMS_SUCCESSFUL) {
		free(*eth);
		*eth = NULL;
		printf ("Can't create task: %d\n", status);
		return -1;
	}

	err = server_init(*eth, host, port);
	if ( err ) {
		free(*eth);
		*eth = NULL;
		printf("Error initializing Ethernet Server: %d\n", err);
		return -1;
	}

	/* Start Ethernet server thread */
	status = rtems_task_start((*eth)->taskEthid, task_bm_eth_server, (rtems_task_argument)(*eth));
	if ( status != RTEMS_SUCCESSFUL ) {
		close((*eth)->ssock);
		free(*eth);
		*eth = NULL;
		printf("Failed to start task A\n");
		exit(-1);
	}

	if (wait_for_client) {
		/* Wait for client to connect before proceeding */
		printf("Waiting for TCP/IP client to connect\n");
		while ( (*eth)->client_avail == 0 ) {
			/* Wait 10 ticks */
			rtems_task_wake_after(10);
		}
	}

	return 0;
}

void server_stop(bm_eth_server_t eth)
{
	if (eth->taskEthid) {
		rtems_task_delete(eth->taskEthid);
		eth->taskEthid = 0;
	}
	if ( eth->ssock >= 0 ) {
		close(eth->ssock);
		eth->ssock = -1;
	}
	if ( eth->sock >= 0 ) {
		close(eth->sock);
		eth->sock = -1;
	}

}

void server_free(bm_eth_server_t eth)
{
		if (eth) {
			server_stop(eth);
			free(eth);
		}
}
