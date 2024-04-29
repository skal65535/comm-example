// Licensed under the Apache License, Version 2.0 (the "License");


// Good read: https://www.geeksforgeeks.org/socket-programming-cc/

#include <sys/socket.h>
#include <unistd.h>      // write() / read()
//#include <netinet/in.h>  
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#include <vector>
typedef std::vector<uint8_t> data_t;

// PROTOCOL

typedef enum {
  CMD_ID = 0,
  CMD_SET,
  CMD_GET,

  CMD_LAST,  // for safety checks
} MessageType;

struct CmdId {
  CmdId(uint32_t id) : id_(htonl(id)) {}
  uint32_t id() const { return ntohl(id_); }
 private:
  uint32_t id_;  // in network format
};

// Message always contain a client_id and a type.
struct Message {
 public:
  Message() = default;
  Message(MessageType type, uint32_t client_id, uint32_t len = 0, void* data = nullptr)
      : type_(type), client_id_(client_id) {
    if (len > 0) {
      payload_.resize(len);  // TODO: could fail memory allocation...
      memcpy(payload_.data(), data, len * sizeof(uint8_t)); 
    }
  }

  MessageType Type() const { return type_; }
  const char* TypeName() const {
    static const char* kTypeNames[] = { "CMD_ID", "CMD_SET", "CMD_GET", "CMD_LAST" };
    assert(type_ <= CMD_LAST);
    return kTypeNames[type_];
  }
  const void* Payload() const { return payload_.data(); }
  uint32_t ClientId() const { return client_id_; }

  bool Read(int socket) {
    if (!ReadHeader(socket)) return false;
    const uint32_t len = payload_.size();
    if (len > 0 && read(socket, payload_.data(), len) != len) return false;
    return true;
  }
  bool Write(int socket) {
    if (!WriteHeader(socket)) return false;
    const uint32_t len = payload_.size();
    if (len > 0 && write(socket, payload_.data(), len) != len) return false;
    return true;
  }

 protected:
  bool WriteHeader(int socket) const {
    if (socket < 0) return false;
    uint8_t tmp[kHeaderSize];
    *(uint32_t*)(tmp + 0) = htonl((uint32_t)type_);
    *(uint32_t*)(tmp + 4) = htonl((uint32_t)client_id_);
    *(uint32_t*)(tmp + 8) = htonl((uint32_t)payload_.size());
    return (write(socket, tmp, kHeaderSize) == kHeaderSize);
  }
  bool ReadHeader(int socket) {
    if (socket < 0) return false;
    uint8_t tmp[kHeaderSize];
    if (read(socket, tmp, kHeaderSize) != kHeaderSize) return false;
    type_ = (MessageType)ntohl(*(uint32_t*)(tmp + 0));
    if (type_ >= CMD_LAST) return false;
    client_id_ = ntohl(*(uint32_t*)(tmp + 4));
    const uint32_t len = (uint32_t)ntohl(*(uint32_t*)(tmp + 8));
    payload_.resize(len);
    return true;
  }

 protected:
  static const size_t kHeaderSize = 12;  // type + id + payload-length

  MessageType type_ = CMD_LAST;
  uint32_t client_id_ = 0;
  data_t payload_;
};
