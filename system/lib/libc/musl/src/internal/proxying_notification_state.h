/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Flag values used when creating postMessage notifications and when freeing
// proxying queues. New postMessages are created for new work unless the
// relevant task queue is in state PENDING_NOTIFICATION and proxying queues can
// only be freed when all of their task queues are in NO_NOTIFICATION state.
typedef enum notification_state {
  NO_NOTIFICATION = 0,
  RECEIVED_NOTIFICATION = 1,
  PENDING_NOTIFICATION = 2,
} notification_state;
