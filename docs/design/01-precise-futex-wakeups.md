# Design Doc: Precise Futex Wakeups in Emscripten

- **Status**: Draft
- **Bug**: https://github.com/emscripten-core/emscripten/issues/26633

## Context
Currently, `emscripten_futex_wait` (in
`system/lib/pthread/emscripten_futex_wait.c`) relies on a periodic wakeup loop
for pthreads and the main runtime thread. This is done for two primary reasons:

1. **Thread Cancellation**: To check if the calling thread has been cancelled while it is blocked.
2. **Main Runtime Thread Events**: To allow the main runtime thread (even when not the main browser thread) to process its mailbox/event queue.

The current implementation uses a 1ms wakeup interval for the main runtime
thread and a 100ms interval for cancellable pthreads. This leads to unnecessary
CPU wakeups and increased latency for events.

## Goals
- Remove the periodic wakeup loop from `emscripten_futex_wait`.
- Implement precise, event-driven wakeups for cancellation and mailbox events.
- Maintain the existing `emscripten_futex_wait` API signature.
- Focus implementation on threads that support `atomic.wait` (pthreads and workers).

## Non-Goals
- **Main Browser Thread**: Changes to the busy-wait loop in `futex_wait_main_browser_thread` are out of scope.
- **Direct Atomics Usage**: Threads that call `atomic.wait` directly (bypassing `emscripten_futex_wait`) will remain un-interruptible.
- **Wasm Workers**: Wasm Worker do not have a `pthread` structure, they are not covered by this design.

## Proposed Design

The core idea is to allow "side-channel" wakeups (cancellation, mailbox events)
to interrupt the `atomic.wait` call by having the waker call `atomic.wake` on the
same address the waiter is currently blocked on.

As part of this design we will need to explicitly state that
`emscripten_futex_wait` now supports spurious wakeups.  i.e. it may return `0`
(success) even if the underlying futex was not explicitly woken by the
application.

### 1. `struct pthread` Extensions
We will add the following fields to `struct pthread` (in
`system/lib/libc/musl/src/internal/pthread_impl.h`). All operations on these
fields must use `memory_order_seq_cst` to ensure the handshake is robust.

```c
// The address the thread is currently waiting on in emscripten_futex_wait.
// NULL if the thread is not currently in a futex wait.
_Atomic(void*) waiting_on_address;

// A counter that is incremented every time the thread wakes up from a futex wait.
// Used by wakers to ensure the target thread has actually acknowledged the wake.
_Atomic(uint32_t) wait_counter;

// A bitmask of reasons why the thread was woken for a side-channel event.
_Atomic(uint32_t) wait_reasons;

#define WAIT_REASON_CANCEL  (1 << 0)
#define WAIT_REASON_MAILBOX (1 << 1)
```

### 2. Waiter Logic (`emscripten_futex_wait`)
The waiter will follow this logic (using `SEQ_CST` for all atomic accesses):

1.  **Pre-check**: Check `wait_reasons`. If non-zero, handle the reasons (e.g., process mailbox or handle cancellation).
2.  **Publish**: Set `waiting_on_address = addr`.
3.  **Counter Snapshot**: Read `current_counter = wait_counter`.
4.  **Double-check**: This is critical to avoid the race where a reason was added just before `waiting_on_address` was set. If `wait_reasons` is now non-zero, clear `waiting_on_address` and go to step 1.
5.  **Wait**: Call `ret = __builtin_wasm_memory_atomic_wait32(addr, val, timeout)`.
6.  **Unpublish**:
    -   Set `waiting_on_address = NULL`.
    -   Atomically increment `wait_counter`.
7.  **Post-check**: Check `wait_reasons`. If non-zero, handle the reasons.
8.  **Return**: Return the result of the wait to the caller.
    -   If `ret == ATOMICS_WAIT_OK`, return `0`.
    -   If `ret == ATOMICS_WAIT_TIMED_OUT`, return `-ETIMEDOUT`.
    -   If `ret == ATOMICS_WAIT_NOT_EQUAL`, return `-EWOULDBLOCK`.

Note: We do **not** loop internally if `ret == ATOMICS_WAIT_OK`. Even if we suspect the wake was caused by a side-channel event, we must return to the user to avoid "swallowing" a simultaneous real application wake that might not have changed the memory value.

### 3. Waker Logic
When a thread needs to wake another thread for a side-channel event (e.g., in `pthread_cancel` or `em_task_queue_enqueue`):

1.  Atomically OR the appropriate bit into the target thread's `wait_reasons` (`SEQ_CST`).
2.  Read `target_addr = target->waiting_on_address` (`SEQ_CST`).
3.  If `target_addr` is not NULL:
    -   Read `start_c = target->wait_counter` (`SEQ_CST`).
    -   Enter a loop:
        -   Call `emscripten_futex_wake(target_addr, 1)`.
        -   Exit loop if `target->wait_counter != start_c` OR `target->waiting_on_address != target_addr`.
        -   **Yield**: Call `sched_yield()` (or a small sleep) to allow the target thread to proceed if it is currently being scheduled.

### 4. Handling the Race Condition
The "Lost Wakeup" race is handled by the combination of:
-   The waiter double-checking `wait_reasons` after publishing its `waiting_on_address`.
-   The waker looping `atomic.wake` until the waiter increments its `wait_counter`.

Even if the waker's first `atomic.wake` occurs after the waiter's double-check
but *before* the waiter actually enters the `atomic.wait` instruction, the waker
will continue to loop and call `atomic.wake` again. The subsequent call(s) will
successfully wake the waiter once it is actually sleeping.

Multiple wakers can safely call this logic simultaneously; they will all exit
the loop as soon as the waiter acknowledges the wake by incrementing the
counter.

### 5. Overlapping and Spurious Wakeups
The design must handle cases where "real" wakeups (triggered by the application) and "side-channel" wakeups (cancellation/mailbox) occur simultaneously.

1.  **Spurious Wakeups for Other Threads**: If multiple threads are waiting on the same address (e.g., a shared mutex), a side-channel `atomic_wake(addr, 1)` targeted at Thread A might be delivered by the kernel to Thread B.
    -   **Thread B's response**: It will wake up, increment its `wait_counter`, see that its `wait_reasons` are empty, and return `0` to its caller.
    -   **Thread C (the waker)**: It will see that Thread A's `wait_counter` has *not* changed and `waiting_on_address` is still `addr`. It will therefore continue its loop and call `atomic_wake` again until Thread A is finally woken.
    -   **Result**: Thread B experiences a "spurious" wakeup. This is acceptable and expected behavior for futex-based synchronization.
2.  **Handling Side-Channel Success**: If Thread A is woken by the side-channel, it handles the event and returns `0`. The user's code will typically see that its own synchronization condition is not yet met and immediately call `emscripten_futex_wait` again. This effectively "resumes" the wait from the user's perspective while having allowed the side-channel event to be processed.
3.  **No Lost "Real" Wakeups**: By returning to the caller whenever `atomic.wait` returns `OK`, we ensure that we never miss or swallow a real application-level `atomic.wake`.

### 6. Counter Wrap-around
The `wait_counter` is a `uint32_t` and will wrap around to zero after $2^{32}$ wakeups. This is safe because:
1.  **Impossibility of Racing**: For the waker to "miss" a wake-up due to wrap-around, the waiter would have to wake up and re-enter a sleep state exactly $2^{32}$ times in the very brief window between the waker's `atomic_wake` and its subsequent check of `wait_counter`. Even at extreme wakeup frequencies (e.g., 1 million per second), this would take over an hour.
2.  **Address Change Check**: The waker loop also checks `target->waiting_on_address != target_addr`. If the waiter wakes up and either stops waiting or starts waiting on a *different* address, the waker will exit the loop regardless of the counter value.

### 6. Benefits
-   **Lower Power Consumption**: Threads can sleep indefinitely (or for the full duration of a user-requested timeout) without periodic wakeups.
-   **Lower Latency**: Mailbox events and cancellation requests are processed immediately rather than waiting for the next 1ms or 100ms tick.
-   **Simpler Loop**: The complex logic for calculating remaining timeout slices in `emscripten_futex_wait` is removed.

## Alternatives Considered
-   **Signal-based wakeups**: Not currently feasible in Wasm as signals are not
    implemented in a way that can interrupt `atomic.wait`.
-   **A single global "wake-up" address per thread**: This would require the
    waiter to wait on *two* addresses simultaneously (the user's futex and its
    own wakeup address), which `atomic.wait` does not support. The proposed
    design works around this by having the waker use the *user's* futex address.

## Security/Safety Considerations
-   The `waiting_on_address` must be managed carefully to ensure wakers don't
    call `atomic.wake` on stale addresses. The `wait_counter` and clearing the
    address upon wake mitigate this.
-   The waker loop should have a reasonable fallback (like a yield) to prevent a
    busy-wait deadlock if the waiter is somehow prevented from waking up (though
    `atomic.wait` is generally guaranteed to wake if `atomic.wake` is called).
