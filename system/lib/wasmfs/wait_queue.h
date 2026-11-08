// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Wait queues let a thread block in poll() until the readiness of one of the
// files it watches may have changed.
//
//  - A PollWaiter is owned by one blocking poll() call (on its stack).
//  - A WaitQueue is owned by a pollable object (e.g. one side of a pipe) and
//    holds the waiters currently interested in it.
//  - A WaitRegistration puts a waiter in a queue for as long as it lives.
//
// The object that owns a queue calls notifyAll() whenever its poll() result may
// have changed. That marks every registered waiter as triggered and wakes it.

#pragma once

#include "wasmfs_internal.h"
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <condition_variable>
#include <emscripten/threading.h>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

namespace wasmfs {

class WaitQueue;
class WaitRegistration;

class PollWaiter {
public:
  using Deadline = std::optional<std::chrono::steady_clock::time_point>;

  PollWaiter() = default;
  PollWaiter(const PollWaiter&) = delete;
  PollWaiter& operator=(const PollWaiter&) = delete;

  // Sleep until a queue this waiter is registered in is notified, or until
  // `deadline` passes (std::nullopt waits forever). Returns immediately if a
  // notification arrived since the previous wait(). Clears the notification
  // before returning. Returns true if notified, false on timeout.
  bool wait(Deadline deadline) {
    if (emscripten_is_main_browser_thread()) {
      return waitAsync(deadline);
    }

    std::unique_lock<std::mutex> lock(mutex);
    auto isTriggered = [&]() { return triggered; };
    bool notified = true;
    if (deadline) {
      notified = cond.wait_until(lock, *deadline, isTriggered);
    } else {
      cond.wait(lock, isTriggered);
    }
    triggered = false;
    return notified;
  }

private:
  std::mutex mutex;
  std::condition_variable cond;
  bool triggered = false;
  bool async = false;

  bool waitAsync(Deadline deadline) {
    {
      std::lock_guard<std::mutex> lock(mutex);
      if (triggered) {
        triggered = false;
        return true;
      }
      async = true;
    }

    int ms = -1;
    if (deadline) {
      auto remaining = std::chrono::ceil<std::chrono::milliseconds>(
        *deadline - std::chrono::steady_clock::now());
      ms = static_cast<int>(std::max<long long>(remaining.count(), 0));
    }
    int res = _wasmfs_poll_wait(this, ms);

    std::lock_guard<std::mutex> lock(mutex);
    bool notified = res || triggered;
    triggered = false;
    return notified;
  }

  friend class WaitQueue;
};

class WaitQueue {
public:
  WaitQueue() = default;
  WaitQueue(const WaitQueue&) = delete;
  WaitQueue& operator=(const WaitQueue&) = delete;

  ~WaitQueue() {
    assert(waiters.empty() && "WaitQueue destroyed while waiters registered");
  }

  // Wake every registered waiter. Call after any change that may affect the
  // poll() result of the object owning this queue.
  void notifyAll() {
    // Keep the queue locked while touching waiters: a waiter cannot be
    // unregistered (and so cannot be destroyed) until we are done with it.
    std::lock_guard<std::mutex> lock(mutex);
    for (auto* waiter : waiters) {
      bool isAsync = false;
      {
        std::lock_guard<std::mutex> waiterLock(waiter->mutex);
        waiter->triggered = true;
        isAsync = waiter->async;
      }

      if (isAsync) {
        _wasmfs_poll_notify(waiter);
      } else {
        waiter->cond.notify_one();
      }
    }
  }

private:
  std::mutex mutex;
  // A waiter may appear more than once (e.g. the same fd passed twice to
  // poll()); each registration adds and removes exactly one entry.
  std::vector<PollWaiter*> waiters;

  void add(PollWaiter* waiter) {
    std::lock_guard<std::mutex> lock(mutex);
    waiters.push_back(waiter);
  }

  void remove(PollWaiter* waiter) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = std::find(waiters.begin(), waiters.end(), waiter);
    assert(it != waiters.end());
    waiters.erase(it);
  }

  friend class WaitRegistration;
};

class WaitRegistration {
public:
  WaitRegistration(WaitQueue& queue, PollWaiter& waiter)
    : queue(&queue), waiter(&waiter) {
    queue.add(&waiter);
  }

  ~WaitRegistration() { reset(); }

  WaitRegistration(const WaitRegistration&) = delete;
  WaitRegistration& operator=(const WaitRegistration&) = delete;

  WaitRegistration(WaitRegistration&& other) noexcept
    : queue(std::exchange(other.queue, nullptr)), waiter(other.waiter) {}

  WaitRegistration& operator=(WaitRegistration&& other) noexcept {
    if (this != &other) {
      reset();
      queue = std::exchange(other.queue, nullptr);
      waiter = other.waiter;
    }
    return *this;
  }

private:
  WaitQueue* queue;
  PollWaiter* waiter;

  void reset() {
    if (queue) {
      queue->remove(waiter);
      queue = nullptr;
    }
  }
};

} // namespace wasmfs
