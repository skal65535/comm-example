// Licensed under the Apache License, Version 2.0 (the "License");

#include "./protocol.h"

int main(int argc, const char* argv[]) {
  uint32_t port = 8080;
  const char* server_ip = "127.0.0.1";   // localhost

  uint32_t client_id = 0x4532642;
  if (argc > 1) port = atoi(argv[1]);
  if (argc > 2) server_ip = argv[2];
  if (argc > 3) client_id = atoi(argv[3]);

  const int client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == 0) {
    fprintf(stderr, "socket() failed");
    return 1;
  }
  // Construct the address for the server
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Invalid server address '%s'\n", server_ip);
    close(client_fd);
    return 1;
  }

  if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "connect() failed. Is server '%s:%d' running?\n",
                    server_ip, port);
    close(client_fd);
    return 1;
  }
  Message msg(CMD_ID, client_id);
  msg.Write(client_fd);
  close(client_fd);
  return 0;
}
