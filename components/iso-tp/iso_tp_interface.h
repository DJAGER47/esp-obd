#pragma once

#include <cstddef>
#include <cstdint>

class IIsoTp {
 public:
  struct Message {
    uint32_t tx_id = 0;
    uint32_t rx_id = 0;
    size_t len     = 0;
    uint8_t *data  = nullptr;
  };

  virtual bool send(Message &msg)                        = 0;
  virtual bool receive(Message &msg, size_t size_buffer) = 0;
};
