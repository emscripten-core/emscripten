[
 {
  "name": "emcc$defun$UTF8ArrayToString",
  "reaches": []
 },
 {
  "name": "emcc$defun$UTF8ToString",
  "reaches": [
   "emcc$defun$UTF8ArrayToString"
  ]
 },
 {
  "name": "emcc$defun$_emscripten_console_log",
  "reaches": [
   "emcc$defun$UTF8ToString"
  ]
 },
 {
  "name": "emcc$defun$initRuntime",
  "reaches": [],
  "root": true
 },
 {
  "name": "emcc$defun$ready",
  "reaches": [
   "emcc$defun$run"
  ],
  "root": true
 },
 {
  "name": "emcc$defun$run",
  "reaches": [
   "emcc$export$_main"
  ]
 },
 {
  "name": "emcc$export$_main",
  "export": "_main",
  "reaches": []
 },
 {
  "name": "emcc$export$_unused",
  "export": "c",
  "reaches": []
 },
 {
  "name": "emcc$import$_emscripten_console_log",
  "import": [
   "env",
   "a"
  ],
  "reaches": [
   "emcc$defun$_emscripten_console_log"
  ]
 }
]
