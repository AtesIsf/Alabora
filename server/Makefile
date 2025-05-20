CC = gcc
CFLAGS = -O2 -std=c23 -Wall -Werror -lssl -lcrypto
BIN_DIR = ./bin
SRC_DIR = ./src
OBJ = $(BIN_DIR)/connection_handler.o $(BIN_DIR)/hashbst.o $(BIN_DIR)/entity_hashmap.o $(BIN_DIR)/game.o

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: server keygen clean

server: main.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

keygen:
	yes AI | openssl req -x509 -newkey rsa:4096 -keyout ssl/key.pem -out ssl/cert.pem -days 365 -nodes

clean:
	rm -f ./bin/*
	rm -f server
	rm -f vgcore.*
