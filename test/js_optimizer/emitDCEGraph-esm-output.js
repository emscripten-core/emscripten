[
 {
  "name": "emcc$defun$fd_close_impl",
  "reaches": [
   "emcc$defun$helper"
  ]
 },
 {
  "name": "emcc$defun$fd_write_impl",
  "reaches": []
 },
 {
  "name": "emcc$defun$helper",
  "reaches": []
 },
 {
  "name": "emcc$defun$unused_import_impl",
  "reaches": []
 },
 {
  "name": "emcc$export$__indirect_function_table",
  "export": "__indirect_function_table",
  "reaches": []
 },
 {
  "name": "emcc$export$_main",
  "export": "main",
  "reaches": [],
  "root": true
 },
 {
  "name": "emcc$export$_unused_export",
  "export": "unused_export",
  "reaches": []
 },
 {
  "name": "emcc$export$_used_export",
  "export": "used_export",
  "reaches": [],
  "root": true
 },
 {
  "name": "emcc$export$memory",
  "export": "memory",
  "reaches": [],
  "root": true
 },
 {
  "name": "emcc$import$fd_close_impl",
  "import": [
   "env",
   "fd_close"
  ],
  "reaches": [
   "emcc$defun$fd_close_impl"
  ]
 },
 {
  "name": "emcc$import$fd_write_impl",
  "import": [
   "env",
   "fd_write"
  ],
  "reaches": [
   "emcc$defun$fd_write_impl"
  ]
 },
 {
  "name": "emcc$import$unused_import_impl",
  "import": [
   "env",
   "unused_import"
  ],
  "reaches": [
   "emcc$defun$unused_import_impl"
  ]
 }
]
