/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

// Flag values used when creating postMessage notifications and when freeing
// proxying queues. New postMessages are created for new work unless the
// relevant task queue is in state NOTIFICATION_PENDING and proxying queues can
// only be freed when all of their task queues are in NOTIFICATION_NONE state.
typedef enum notification_state {
  NOTIFICATION_NONE = 0,
  NOTIFICATION_RECEIVED = 1,
  NOTIFICATION_PENDING = 2,
} notification_state;
