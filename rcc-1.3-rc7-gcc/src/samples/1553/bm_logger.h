#ifndef __BM_LOGGER_H_
#define __BM_LOGGER_H_

typedef struct bm_logger* bm_logger_t;

/**
 * Initilize a bm_log object, start BM and optionally start an ethernet server.
 *
 * bm_log   - Object to be initalized
 * log_base - Base address for the BM driver
 * ip       - If ip is set to non-NULL, then a ethernet server task will spawn.
 * port     - Port to listen to incomming connections.
 * wait     - Wait for a client to connect to the ethernet server.
 */
int bm_init(bm_logger_t *bm, void* log_base, const char *ip, int port, int wait);

/**
 * Collect BM data
 */
int bm_log(bm_logger_t bm);

/**
 * Returns current count
 */
int bm_count(bm_logger_t bm);

/**
 * Shutdown and clean-up a bm_log object
 */
void bm_stop(bm_logger_t bm);

#endif /* __BM_LOGGER_H_ */
