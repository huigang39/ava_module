#ifndef NET_H
#define NET_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
#include <sys/epoll.h>
typedef int socket_t;
#define CLOSE_SOCKET close
#elif defined(_WIN32)
#include <winsock2.h>
typedef SOCKET socket_t;
#define CLOSE_SOCKET closesocket
#endif

#include <stdio.h>

#include "util/typedef.h"

typedef enum {
  NET_TYPE_NONE,
  NET_TYPE_UDP,
  NET_TYPE_TCP,
} net_type_e;

/* 日志事件类型 */
typedef enum {
  NET_LOG_SEND, // 发送
  NET_LOG_RECV, // 接收
} net_log_type_e;

/* 日志元数据(时间戳、设备/通道信息) */
#pragma pack(push, 1)
typedef struct {
  U64 timestamp; // 时间戳
  U8  type;      // 事件类型
  U32 ip;        // 设备 IP
  U16 port;      // 设备端口
  U32 len;       // 数据长度
} net_log_meta_t;
#pragma pack(pop)

/* 日志回调函数类型 */
typedef void (*net_log_cb_t)(FILE *fp, const net_log_meta_t *log_meta, const void *log_data);

void timestamp_ns_to_readable(U64 timestamp_ns, char *buf, U32 cap);
void net_file_logger(FILE *fp, const net_log_meta_t *log_meta, const void *log_data);

typedef enum {
  SEND,
  RECV,
  SEND_RECV,
} net_flag_e;

typedef struct net_manager net_manager_t;
typedef struct net_dev     net_dev_t;
typedef struct net_channel net_channel_t;

struct net_channel {
  U16 port;
#ifdef __linux__
  struct epoll_event epoll_ev;
#elif defined(_WIN32)
#endif
  socket_t       socket_fd;
  int            flags;
  int            status;
  net_channel_t *next;
  net_dev_t     *dev;
};

#define MAX_IP_SIZE  16
#define MAX_BUF_SIZE 1024

struct net_dev {
  char           ip[MAX_IP_SIZE];
  net_channel_t  ch;
  U32            ch_num;
  net_dev_t     *next;
  net_manager_t *mgr;
};

struct net_manager {
  net_dev_t    dev;
  U32          dev_num;
  FILE        *log_fp;
  net_log_cb_t log_cb;
#ifdef __linux__
  int epoll_fd;
#elif defined(_WIN32)
  // TODO: Windows
#endif
};

typedef struct {
  net_type_e net_type;
  char       ip[MAX_IP_SIZE];
  U16        port;
} net_config_t;

typedef struct {
  net_channel_t *ch;
  U8            *buf;
  U32            len;
  net_flag_e     flag;
} net_input_data_t;

typedef struct {
  U8 *buf;
  U32 len;
} net_output_data_t;

typedef struct {
  net_manager_t  net_manager;
  net_log_meta_t log_meta;
} net_temp_data_t;

typedef struct {
  net_input_data_t  input;
  net_output_data_t output;
  net_temp_data_t   temp;
} net_data_t;

typedef struct net {
  net_config_t config;
  net_data_t   data;

  struct {
    void (*f_init)(struct net *self, net_config_t config);
    void (*f_config)(struct net *self, net_config_t config);
    void (*f_send_recv)(struct net *self);
    void (*f_reset)(struct net *self);
  } func;

  struct {
    struct {
      U32 NONE : 1;
    } warn;

    struct {
      U32 NONE : 1;
    } err;
  } status;
} net_t;

void net_init(net_t *self, net_config_t config);
void net_config(net_t *self, net_config_t config);
void net_send_recv(net_t *self);
void net_reset(net_t *self);

#define MAX_IP_NUM 255
typedef struct {
  char ip[MAX_IP_SIZE];
  char buf[MAX_BUF_SIZE];
} broadcast_t;
int net_broadcast(const char *ip,
                  U16         port,
                  const U8   *data,
                  U32         data_len,
                  broadcast_t (*responses)[MAX_IP_NUM],
                  U32 max_ips,
                  U32 timeout_ms);

#ifdef __cplusplus
}
#endif

#endif // !NET_H
