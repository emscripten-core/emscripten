/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <emscripten/em_asm.h>

#include <pthread.h>

EM_JS(void, run_test, (), {
   function sendMessageToMainThread(cmd, payload) {
     postMessage({
       cmd: 'custom',
       customCmd: cmd,
       payload: payload,
       worker: _pthread_self()
     });
   }

   Module.onCustomMessage = function (message) {
     switch (message.customCmd) {
       case 'reply_first': {
         sendMessageToMainThread('second_msg', message.payload + '[second]');
         break;
       }
       case 'reply_second': {
         sendMessageToMainThread('last_msg', message.payload + '[last]');
         break;
       }
       default: {
         throw('unknown custom message');
       }
     }
   };
   sendMessageToMainThread('first_msg', '[first]');
})

void* Run(void* data)
{
    (void)data;
    run_test();
    emscripten_exit_with_live_runtime();
    return NULL;
}

int main()
{
    pthread_t th;
    pthread_create(&th, NULL, Run, NULL);
    emscripten_exit_with_live_runtime();
    return 0;
}
