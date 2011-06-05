#ifndef QNT_CORE_UTIL_QUEUE_HPP
#define QNT_CORE_UTIL_QUEUE_HPP

#include <deque>
#include <boost/thread.hpp>

#include "atomic.hpp"

template <typename T>
class lockedQueue {
public:
  void push(const T& t) {
    boost::mutex::scoped_lock lock(m_queue_mutex);
    bool was_empty = m_queue.empty();
    m_queue.push_back(t);
    lock.unlock();
    if(was_empty)
      m_queue_cond.notify_one();
  }

  T wait_pop() {
    boost::mutex::scoped_lock lock(m_queue_mutex);
    while(m_queue.empty())
      m_queue_cond.wait(lock);
    T val = m_queue.front();
    m_queue.pop_front();
    return val;
  }

  bool try_pop(T& val) {
    boost::mutex::scoped_lock lock(m_queue_mutex);
    if(m_queue.empty())
      return false;
    val = m_queue.front();
    m_queue.pop_front();
    return true;
  }
private:
  std::deque<T> m_queue;
  boost::mutex m_queue_mutex;
  boost::condition_variable m_queue_cond;
};

template <typename T>
class locklessQueue {
  class QueueItem {
  public:
    QueueItem() : next(0) {}
    T value;
    QueueItem *next;
  };
  
  QueueItem *head;

public:
  locklessQueue() : head(new QueueItem) {}

  void push(const T& t) {
    QueueItem *new_head = new QueueItem;
    new_head->value = t;
    do {
      new_head->next = head->next;
    } while(!compareAndSwapPointer(head->next, new_head->next, new_head));
  }

  T wait_pop() {
    QueueItem *item;
    T result;
    for(;;) {
      do {
        item = head->next;
        if(item == 0)
          break;
      } while(!compareAndSwapPointer(head->next, item, item->next));
      if(item) {
        result = item->value;
        delete item;
        return result;
      }
    }
  }

  bool try_pop(T& t) {
    QueueItem *item;
    do {
      item = head->next;
      if(item == 0)
        return false;
    } while(!compareAndSwapPointer(head->next, item, item->next));
    t = item->value;
    delete item;
    return true;
  }
};

#ifdef USE_LOCKED_QUEUE
template <typename T>
class queue : public lockedQueue<T> {};
#else
template <typename T>
class queue : public locklessQueue<T> {};
#endif


#endif // QNT_CORE_UTIL_QUEUE_HPP
