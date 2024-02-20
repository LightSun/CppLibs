#ifndef SAVEQUEUE_H
#define SAVEQUEUE_H

#include <assert.h>
#include <atomic>
#include <stdlib.h>
#include <stdio.h>

namespace h7 {
template<typename T>
class SaveQueue
{
public:
    //2 ^ n
  SaveQueue(size_t buffer_size)
    : buffer_(new cell_t [buffer_size])
    , buffer_mask_(buffer_size - 1)
  {
    assert((buffer_size >= 2) &&
      ((buffer_size & (buffer_size - 1)) == 0));
    for (size_t i = 0; i != buffer_size; i += 1)
      buffer_[i].sequence_.store(i, std::memory_order_relaxed);
    enqueue_pos_.store(0, std::memory_order_relaxed);
    dequeue_pos_.store(0, std::memory_order_relaxed);
  }
  SaveQueue(): buffer_(nullptr){}

  ~SaveQueue()
  {
    delete [] buffer_;
  }

  void setBufferSize(size_t buffer_size){
      assert(buffer_ == nullptr);
      buffer_ = new cell_t [buffer_size];
      buffer_mask_ = buffer_size - 1;
      //
      assert((buffer_size >= 2) &&
        ((buffer_size & (buffer_size - 1)) == 0));
      for (size_t i = 0; i != buffer_size; i += 1)
        buffer_[i].sequence_.store(i, std::memory_order_relaxed);
      enqueue_pos_.store(0, std::memory_order_relaxed);
      dequeue_pos_.store(0, std::memory_order_relaxed);
  }

  size_t size(){
     size_t push = enqueue_pos_.load(std::memory_order_acquire);
     size_t pop = dequeue_pos_.load(std::memory_order_acquire);
     printf("push count = %d, pop count = %d\n.", push, pop);
     return push - pop;
  }

  size_t bufferSize(){
      return buffer_mask_ + 1;
  }

  bool enqueue(T const& data)
  {
    //printf("--- enqueue start ---\n");
    cell_t* cell;
    size_t pos = enqueue_pos_.load(std::memory_order_relaxed);
    for (;;)
    {
      cell = &buffer_[pos & buffer_mask_];
      size_t seq =
        cell->sequence_.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)pos;
      if (dif == 0)
      {
        if (enqueue_pos_.compare_exchange_weak
            (pos, pos + 1, std::memory_order_relaxed))
          break;
      }
      else if (dif < 0)
        return false;
      else
        pos = enqueue_pos_.load(std::memory_order_relaxed);
    }
    cell->data_ = data;
    cell->sequence_.store(pos + 1, std::memory_order_release);
    //printf("--- enqueue end ---\n");
    return true;
  }

  bool dequeue(T& data)
  {
    //printf("--- dequeue start ---\n");
    cell_t* cell;
    size_t pos = dequeue_pos_.load(std::memory_order_relaxed);
    for (;;)
    {
      cell = &buffer_[pos & buffer_mask_];
      size_t seq =
        cell->sequence_.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
      if (dif == 0)
      {
        if (dequeue_pos_.compare_exchange_weak
            (pos, pos + 1, std::memory_order_relaxed))
          break;
      }
      else if (dif < 0)
        return false;
      else
        pos = dequeue_pos_.load(std::memory_order_relaxed);
    }
    data = cell->data_;
    cell->sequence_.store(pos + buffer_mask_ + 1, std::memory_order_release);
    //printf("--- dequeue end ---\n");
    return true;
  }

private:
  struct cell_t
  {
    std::atomic<size_t>   sequence_;
    T                     data_;
  };

  static size_t const     cacheline_size = 64;
  typedef char            cacheline_pad_t [cacheline_size];

  cacheline_pad_t         pad0_;
  cell_t* const           buffer_;
  size_t const            buffer_mask_;
  cacheline_pad_t         pad1_;
  std::atomic<size_t>     enqueue_pos_;
  cacheline_pad_t         pad2_;
  std::atomic<size_t>     dequeue_pos_;
  cacheline_pad_t         pad3_;

  SaveQueue(SaveQueue const&);
  void operator = (SaveQueue const&);
};
}
#endif // SAVEQUEUE_H
