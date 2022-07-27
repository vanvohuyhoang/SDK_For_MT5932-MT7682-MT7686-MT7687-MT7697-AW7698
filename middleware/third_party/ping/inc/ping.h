
#ifndef __PING_H__
#define __PING_H__


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * PING_USE_SOCKETS: Set to 1 to use sockets, otherwise the raw api is used
 */
#ifndef PING_USE_SOCKETS
#define PING_USE_SOCKETS    LWIP_SOCKET
#endif

#define PING_IP_ADDR_V4      0
#define PING_IP_ADDR_V6      1

typedef struct _ping_result {
    uint32_t min_time;
    uint32_t max_time;
    uint32_t avg_time;
    uint32_t total_num;
    uint32_t lost_num;
    uint32_t recv_num;
} ping_result_t;

typedef void (* ping_request_result_t)(ping_result_t *result);

typedef struct _ping_para {
    uint32_t count;
    uint32_t size;
    uint32_t recv_timeout;
    ping_request_result_t callback;
    char *addr;
} ping_para_t;

//addr_type:PING_IP_ADDR_V4 or PING_IP_ADDR_V6, current only support PING_IP_ADDR_V4
void ping_request(uint32_t count, char *addr, uint8_t addr_type, uint32_t ping_size, ping_request_result_t callback);

void ping_request_ex(uint8_t addr_type, ping_para_t *para);

void ping_init(uint32_t count, char *name_addr, uint8_t addr_len, uint32_t ping_size);

uint32_t get_ping_done(void);


#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */


#ifdef __cplusplus
}
#endif

#endif /* __PING_H__ */

