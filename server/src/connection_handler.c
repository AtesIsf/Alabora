#include "../include/connection_handler.h"
#include "../include/hashbst.h"

#include <assert.h>
#include <errno.h>
#include <openssl/ssl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_BUF_LEN (256)

volatile bool terminate_program = false;

void terminate_signal(int n) {
  terminate_program = true;
}

void init_sockets() {
  signal(SIGINT, terminate_signal);

  int endpoint = socket(AF_INET6, SOCK_STREAM, 0);
  if (endpoint == 0) {
    fprintf(stderr, "Failed initializing endpoint socket (%d)\n", errno);
    return;
  }

  int opt = 1;
  if (setsockopt(endpoint, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
    fprintf(stderr, "setsockopt() failed (%d)\n", errno);
    return;
  }

  struct sockaddr_in6 addr = { 0 };
  int addr_size = sizeof(addr);
  addr.sin6_family = AF_INET6;
  addr.sin6_addr = in6addr_any;
  addr.sin6_port = htons(PORT);

  if (bind(endpoint, (struct sockaddr *) &addr, (socklen_t) addr_size) < 0) {
    fprintf(stderr, "bind() failed (%d)\n", errno);
    return;
  }

  if (listen(endpoint, 10) < 0) {
    fprintf(stderr, "listen() failed (%d)\n", errno);
    return;
  }
  printf("Listening on port %d...\n", PORT);

  hashbst_node_t *root = generate_hashbst();
  print_hashbst(root);

  while (!terminate_program) {
    handle_connection(endpoint, (struct sockaddr *) &addr, (socklen_t *) &addr_size, root); 
  }
  free_hashbst(&root);
  root = NULL;
  close(endpoint);
}

/*
 * Parses the received http request. Returns EMPTY_REQ if there was a problem with
 * parsing the request.
 */

http_request_t parse_request(char *buf) {
  assert(buf != NULL);
  http_request_t req = { 0 };
  int status = sscanf(buf, "%7[^/]%63[^ ]", req.method, req.path);
  return status == 2 ? req : EMPTY_REQ;
}

const char *get_code_string(int *code) {
  switch (*code) {
    case HTTP_OK: return "OK";
    case HTTP_BAD_REQUEST: return "Bad Request"; 
    case HTTP_NOT_FOUND: return "Not Found"; 
    case HTTP_INTERNAL_SERVER_ERROR: return "Internal Server Error";
  }
  *code = HTTP_INTERNAL_SERVER_ERROR;
  return "Internal Server Error";
}

/*
 * Wraps the given response string in HTTP format. The returned string must
 * be freed afterwards. The given code int may be modified.
 */

char *wrap_in_http(const char *str, int code, const char *file_type) {
  assert(str != NULL && file_type != NULL);
  char *response = calloc(1024, 1);
  assert(response != NULL);

  const char *code_str = get_code_string(&code);
  sprintf(response, 
          "HTTP/1.1 %d %s\r\n"
          "Content-Type: text/%s\r\n"
          "Content-length: %lu\r\n\r\n"
          "%s",
          code, code_str, file_type, strlen(str), str);
  return response;
}

void handle_connection(int fd, struct sockaddr *addr, socklen_t *len, hashbst_node_t *root) {
  int handler_fd = accept(fd, addr, len);
  if (handler_fd < 0) return;

  SSL *ssl = SSL_new(ctx);
  if (SSL_set_fd(ssl, handler_fd) != 1) {
    close(handler_fd);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return;
  }
  if (SSL_accept(ssl) <= 0) {
    close(handler_fd);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    return;
  }

  char buf[MAX_BUF_LEN] = { '\0' };
  int bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
  buf[bytes] = '\0';

  http_request_t req = parse_request(buf);
  printf("%s %s\n", req.method, req.path);
  char *(*handler)(void) = find_hashbst_node(root, req.path);

  handler = handler == NULL ? (char *(*)(void)) find_hashbst_node(root, "/pagenotfound") : handler;

  char *response = handler();
  SSL_write(ssl, response, strlen(response));

  close(handler_fd);
  SSL_shutdown(ssl);
  SSL_free(ssl);
  free(response);
  response = NULL;
}

