#ifndef __ETHSRV_H__
#define __ETHSRV_H__

enum {
	CMD_STATUS = 1,
	CMD_GET_INFO = 2,
	CMD_GET_LOG = 3,
};

struct cmd_hdr {
	unsigned short	length;
	unsigned char	cmdno;
	unsigned char	reserved;
} __attribute__ ((packed));


/* GET LOG ENTRIES FROM A SPECIFIC DEVICE */
struct cmd_get_log {
	struct cmd_hdr		hdr;
	char			devno;
} __attribute__ ((packed));

struct cmd_resp_get_info {
	struct cmd_hdr		hdr;
	char			dev_cnt;	/* Number of Devices */
	unsigned char		status[16];	/* Max 16 devices */
} __attribute__ ((packed));

struct cmd_resp_get_log {
	struct cmd_hdr		hdr;
	char			devno;
	unsigned char		status;
	unsigned char		log_cnt;
	unsigned char		pad;
	unsigned int		log[250];	/* Up to 250 entries */
} __attribute__ ((packed));

#define MAX_COMMAND_NUM CMD_GET_LOG
#define MAX_COMMAND_SIZE (sizeof(struct cmd_get_log)-sizeof(struct cmd_hdr))

typedef struct bm_eth_server* bm_eth_server_t;

/* Ethernet Server functions */

typedef int (*server_get_func)(void *arg, void *data, int cnt);

/**
 * Initilize a bm_eth_server object
 *
 * eth             - Object to be initilized
 * host            - If ip is set to non-NULL, then a ethernet server task will spawn.
 * port            - Port to listen to incomming connections.
 * wait_for_client - Wait for a client to connect to the ethernet server.
 * get             - Retrieve data
 * data	           - Argument to get function
 */
extern int server_setup(bm_eth_server_t *eth, const char *host, int port, int wait_for_client, server_get_func get, void *data);

/**
 * Shutdown
 */
extern void server_stop(bm_eth_server_t eth);

/**
 * Clean-up a bm_eth_server object
 */
extern void server_free(bm_eth_server_t eth);

#endif /* __ETHSRV_H__ */
