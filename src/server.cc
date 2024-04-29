// Licensed under the Apache License, Version 2.0 (the "License");

#include "./protocol.h"
#include <thread>
using std::thread;

int Error(const char* msg) {  // for convenience / future logging
  fprintf(stderr, "Error: %s\n", msg);
  return 1;
}

struct sockaddr_in FillAddress(int port) {
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  memset(address.sin_zero, '\0', sizeof(address.sin_zero));
  return address;
}

int ListenToClient(int fd) {
  const uint32_t myself = std::hash<std::thread::id>{}(std::this_thread::get_id());
  fprintf(stderr, " => Thread #%u listening to socket %d\n", myself, fd);
  bool finished = false;
  while (!finished) {
    Message msg;
    if (msg.Read(fd)) {
      printf("CMD: %s\n", msg.TypeName());
    }
    if (msg.Type() == CMD_ID) {
      printf("CMD_ID: %u\n", msg.ClientId());
    }
    finished = true;
  }
  close(fd);
  return 0;
}

int main(int argc, const char* argv[]) {
  uint32_t port = 8080;
  if (argc > 1) port = atoi(argv[1]);

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    return Error("socket() failed");
  }
  // to avoid 'Address already in use' error:
  int yes = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    close(server_fd);
    return Error("setsockopt() failed");
  }

  struct sockaddr_in address = FillAddress(port);
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    close(server_fd);
    return Error("bind() failed");
  }
  if (listen(server_fd, /*backlog=*/5) < 0) {
    close(server_fd);
    return Error("listen() failed");
  }
  printf("Listening at port: %d\n", port);
  bool finished = false;
  socklen_t addrlen = sizeof(address);
  std::vector<std::thread> clients;
  while (!finished) {   // main loop.
    const int client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_socket < 0) {
      Error("accept() failed");
    } else {
      printf("New connection! (%d)\n", client_socket);
      // we launch one thread per client
      clients.emplace_back(std::thread(ListenToClient, client_socket));
    }
  }
  // server is closing...
  for (auto& t : clients) t.join();
  close(server_fd);
  return 0;
}
