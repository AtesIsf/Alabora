#include "./include/connection_handler.h"
#include "./include/game.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>

SSL_CTX *ctx = NULL;
unsigned short PORT = 1923;

void init_openssl() {
  SSL_load_error_strings();
  OpenSSL_add_ssl_algorithms();
  ctx = SSL_CTX_new(TLS_server_method());
  SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5:!RC4");
  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
  SSL_CTX_set_ecdh_auto(ctx, 1);

  if (SSL_CTX_use_certificate_file(ctx, "./ssl/cert.pem", SSL_FILETYPE_PEM) < 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  if (SSL_CTX_use_PrivateKey_file(ctx, "./ssl/key.pem", SSL_FILETYPE_PEM) < 0) {
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {
  if (argc == 2) {
    sscanf(argv[1], "%hu", &PORT);
  }

  game_t game = init_game();
  printf("Initializing SSL...\n");
  init_openssl();
  printf("Initializing server...\n");
  init_sockets(&game);

  printf("\nClosing server...\n");
  SSL_CTX_free(ctx);
  free_game(&game);
  ctx = NULL;
  return 0;
}
