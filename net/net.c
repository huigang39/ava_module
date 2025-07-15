#ifdef __linux__
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <ws2tcpip.h>
#endif

#include <string.h>

#include "net.h"

static int net_manager_init(net_t *self);

void
net_init(net_t *self, const net_config_t config) {
  self->config = config;

  self->func.f_init      = net_init;
  self->func.f_config    = net_config;
  self->func.f_send_recv = net_send_recv;
  self->func.f_reset     = net_reset;

  net_manager_init(self);
}

void
net_config(net_t *self, const net_config_t config) {
  self->config = config;
}

void
net_send_recv(net_t *self) {
  const net_input_data_t *input  = &self->data.input;
  net_output_data_t      *output = &self->data.output;

  int ret = 0;
  if (input->flag == SEND || input->flag == SEND_RECV) {
    ret = send(input->ch->socket_fd, (const char *)input->buf, (int)input->len, 0);
    if (ret <= 0)
      return;
  }

  if (input->flag == RECV || input->flag == SEND_RECV) {
    ret = recv(input->ch->socket_fd, (char *)output->buf, MAX_BUF_SIZE, 0);
    if (ret <= 0)
      return;
    output->len = ret;
  }
}

void
net_reset(net_t *self) {}

static int
net_manager_init(net_t *self) {
  net_manager_t *mgr = &self->data.temp.net_manager;
  net_dev_t     *dev = &self->data.temp.net_manager.dev;
  net_channel_t *ch  = &self->data.temp.net_manager.dev.ch;

  memcpy(dev->ip, self->config.ip, strlen(self->config.ip));
  ch->port = self->config.port;

#ifdef _WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

  switch (self->config.net_type) {
  case NET_TYPE_UDP:
    ch->socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    break;
  case NET_TYPE_TCP:
    ch->socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    break;
  default:
    break;
  }

  struct sockaddr_in addr = { 0 };
  addr.sin_family         = AF_INET;
  addr.sin_port           = htons(ch->port);
  addr.sin_addr.s_addr    = inet_addr(dev->ip);

  int ret = 0;
#ifdef __linux__
  int flags = fcntl(ch->socket_fd, F_GETFL, 0);
  if (flags < 0)
    return -1;
  flags |= O_NONBLOCK;
  ret = fcntl(ch->socket_fd, F_SETFL, flags);
  if (ret < 0)
    return -1;
#elif defined(_WIN32)
  unsigned long mode = 1;
  ret                = ioctlsocket(ch->socket_fd, FIONBIO, &mode);
  if (ret < 0)
    return -1;
#endif

  ret = connect(ch->socket_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0)
    return -1;

  return 0;
}

int
net_broadcast(const char *ip,
              const U16   port,
              const U8   *data,
              const U32   data_len,
              broadcast_t (*responses)[MAX_IP_NUM],
              const U32 max_ips,
              const U32 timeout_ms) {
#ifdef _WIN32
  {
    WSADATA   wsaData;
    const int wsa_ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsa_ret != 0)
      return -1;
  }
#endif

  const socket_t socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  int            ret       = 0;
  U32            idx       = 0;

  if (socket_fd <= 0) {
    ret = -1;
    goto cleanup;
  }

  const int enable = 1;
  ret = setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&enable, sizeof(enable));
  if (ret < 0)
    goto cleanup;

  struct sockaddr_in addr = { 0 };
  addr.sin_family         = AF_INET;
  addr.sin_port           = htons(port);
  addr.sin_addr.s_addr    = inet_addr(ip);

  ret = sendto(
      socket_fd, (const char *)data, (int)data_len, 0, (struct sockaddr *)&addr, sizeof(addr));
  if (ret < 0)
    goto cleanup;

  struct timeval tv;
  tv.tv_sec  = (int)timeout_ms / 1000;
  tv.tv_usec = (int)(timeout_ms % 1000) * 1000;
  ret        = setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));
  if (ret < 0)
    goto cleanup;

  while (idx < max_ips) {
    struct sockaddr_in resp_addr;
    socklen_t          addr_len = sizeof(resp_addr);
    ret                         = recvfrom(socket_fd,
                   responses[idx]->buf,
                   sizeof(responses[idx]->buf),
                   0,
                   (struct sockaddr *)&resp_addr,
                   &addr_len);
    if (ret < 0)
      goto cleanup;

    const char *ip_str = inet_ntoa(resp_addr.sin_addr);
    if (ip_str) {
      strncpy(responses[idx]->ip, ip_str, strlen(ip_str));
      idx++;
    }
  }

cleanup:
  CLOSE_SOCKET(socket_fd);
  if (idx != 0)
    return (int)idx;
  return ret;
}
