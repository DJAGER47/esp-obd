#pragma once
#include "driver/twai.h"

template <typename T, size_t Size>
class RingBuffer {
 public:
  RingBuffer() :
      head(0),
      tail(0) {}

  bool push(const T& item) {
    size_t next_head = (head + 1) % Size;
    if (next_head == tail)
      return false;  // буфер полон

    buffer[head] = item;
    head         = next_head;
    return true;
  }

  bool pop(T* item) {
    if (tail == head)
      return false;  // буфер пуст

    *item = buffer[tail];
    tail  = (tail + 1) % Size;
    return true;
  }

  bool is_empty() const {
    return head == tail;
  }
  bool is_full() const {
    return (head + 1) % Size == tail;
  }

 private:
  T buffer[Size];
  volatile size_t head;
  volatile size_t tail;
};