/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <em_task_queue.h>
#include <pthread.h>

// Try to increment the refcount of the mailbox, ensuring it will stay open
// until the refcount is decremented again. Returns 1 on success or 0 if the
// mailbox is already closed.
int emscripten_thread_mailbox_ref(pthread_t thread);

// Decrement the mailbox's refcount.
void emscripten_thread_mailbox_unref(pthread_t thread);

// Send a message to the given `thread`. This should only be called after
// incrementing the mailbox refcount to ensure it stays open. The receiving
// thread will receive the message the next time it returns to its event loop,
// or if the target thread shuts down before then, the message's shutdown
// handler will be called instead.
void emscripten_thread_mailbox_send(pthread_t thread, task t);

// Initialize the mailbox on a pthread struct. Called during `pthread_create`.
void _emscripten_thread_mailbox_init(pthread_t thread);

void _emscripten_thread_mailbox_await(pthread_t thread);

// Close the mailbox and cancel any pending messages.
void _emscripten_thread_mailbox_shutdown(pthread_t thread);

// Send a postMessage notification telling the target thread to check its
// mailbox when it returns to its event loop. Pass in the current thread id
// minimize calls back into Wasm.
void _emscripten_notify_mailbox_postmessage(pthread_t target_thread,
                                            pthread_t curr_thread);
