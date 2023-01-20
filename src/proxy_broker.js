/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

'use strict';

// Map pthread IDs to message ports we use to communicate with those pthreads.
const threadPorts = new Map();

// Map recipient pthread IDs for whom we don't yet have a message port to
// messages we've received for them.
const bufferedMessages = new Map();

function handleMessage(msg) {
  const thread = msg.data.targetThread;
  const port = threadPorts.get(thread);
  if (port !== undefined) {
    port.postMessage(msg.data, msg.data.transferList);
  } else {
    // Hold on to the message until we receive a port for the recipient.
    let buffered = bufferedMessages.get(thread);
    if (buffered === undefined) {
      buffered = [];
      bufferedMessages.set(thread, buffered);
    }
    buffered.push(msg);
  }
}

self.onmessage = (msg) => {
  const cmd = msg.data.cmd;
  const thread = msg.data.thread;
  if (cmd === 'create') {
    const port = msg.data.port;
    threadPorts.set(thread, port);
    port.onmessage = handleMessage;
    // Forward any messages we have already received for this thread.
    const buffered = bufferedMessages.get(thread);
    if (buffered !== undefined) {
      buffered.forEach(handleMessage);
      bufferedMessages.delete(thread);
    }
    return;
  }
  if (cmd === 'destroy') {
    bufferedMessages.delete(thread);
  }
};
