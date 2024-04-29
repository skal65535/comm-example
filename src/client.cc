// Licensed under the Apache License, Version 2.0 (the "License");

#include "./protocol.h"

static uint32_t timer = 0;
bool SendTimer(uint32_t client_id, int fd) {
  return Message(CMD_DATA, client_id, sizeof(timer), (void*)&timer).Write(fd);
}

int main(int argc, const char* argv[]) {
  uint32_t client_id = 0x4532642;
  uint32_t port = 8080;
  const char* server_ip = "127.0.0.1";   // localhost

  if (argc > 1) client_id = atoi(argv[1]);
  if (argc > 2) port = atoi(argv[2]);
  if (argc > 3) server_ip = argv[3];

  const int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == 0) {
    fprintf(stderr, "socket() failed");
    return 1;
  }
  // Construct the address for the server
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
    fprintf(stderr, "Invalid server address '%s'\n", server_ip);
    close(fd);
    return 1;
  }

  if (connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "connect() failed. Is server '%s:%d' running?\n",
                    server_ip, port);
    close(fd);
    return 1;
  }
  // we turn the descriptor 'fd' into a non-blocking socket
  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 500 * 1000;  // timeout read every 500ms

  printf("Connection to server %s:%u established.\n", server_ip, port);
  fflush(stdout);

  // Client starts now, first by signaling its id.
  Message(CMD_ID, client_id).Write(fd);

  // Main loop example: using CMD_DATA, we'll supply a timer value when CMD_GET
  // is received from server.
  bool finished = false;
  while (!finished) {
    fd_set all_fd;
    FD_ZERO(&all_fd);
    FD_SET(fd, &all_fd);
    const int err = select(fd + 1, &all_fd, NULL, NULL, &timeout);

    if (err == -1) {
      fprintf(stderr, "select() failed with error -1\n");
      finished = true;
      continue;
    } else if (err == 0) {
      fprintf(stderr, "No special request from server, continuing loop.\n");
      finished = !SendTimer(client_id, fd);
      ++timer;
      if (rand() % 10 == 0) {   // from time to time, we request some data
        fprintf(stderr, "Requesting some data through CMD_GET\n");
        Message(CMD_GET, client_id).Write(fd);
      }
      continue;
    }
    if (!FD_ISSET(fd, &all_fd)) continue;  // hmm... something wrong probably

    // Some data / request is available for read from 'fd'
    Message msg;
    if (!msg.Read(fd)) continue;
    if (msg.Type() == CMD_DATA) {
      printf("Server send data: %s\n", (const char*)msg.Payload());
    }
  }

  Message(CMD_END, client_id).Write(fd);  // signal that we'd closing shop

  close(fd);
  return 0;
}
