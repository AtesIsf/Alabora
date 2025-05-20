#ifndef CONNECTION_HANDLERH_H
#define CONNECTION_HANDLERH_H

#include "game.h"
#include "hashbst.h"

#include <netinet/in.h>
#include <openssl/crypto.h>

#define HTTP_OK (200)
#define HTTP_BAD_REQUEST (400)
#define HTTP_NOT_FOUND (404)
#define HTTP_INTERNAL_SERVER_ERROR (500)

typedef struct http_request {
  char cookies[128];
  char path[64];
  char method[8];
} http_request_t;

#define EMPTY_REQ ((http_request_t) { 0 })

extern SSL_CTX *ctx;
extern unsigned short PORT;

void init_sockets(game_t *);

http_request_t parse_request(char *);

const char *get_code_string(int *code);

char *wrap_in_http(const char *, int, const char*);

char *create_cookie_str(const char *, const char *, unsigned int);

void handle_connection(int, game_t *, struct sockaddr*, socklen_t *, hashbst_node_t *);

#endif
