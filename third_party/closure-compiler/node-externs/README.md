![node.js Closure Compiler externs](https://raw.github.com/dcodeIO/node.js-closure-compiler-externs/master/NodejsClosureCompilerExterns.png)
================================
A collection of node.js externs for use with [Closure Compiler](https://developers.google.com/closure/compiler/docs/overview)
/ [ClosureCompiler.js](https://github.com/dcodeIO/ClosureCompiler.js).

See: [Advanced Compilation and Externs](https://developers.google.com/closure/compiler/docs/api-tutorial3) for details

#### Naming convention ####

* Externs for core components are all lower case
* Externs for [non-core components](https://github.com/dcodeIO/node.js-closure-compiler-externs/tree/master/contrib) begin with an upper case character

#### Node.js specific annotation ####

If an extern file refers to a module that's usually loaded through `var modulename = require("modulename")`, a comment
is added on top of the file. For example for the fs module:

````javascript
/**
 BEGIN_NODE_INCLUDE
 var fs = require('fs');
 END_NODE_INCLUDE
 */
````

**NOTE:** This comment on its own does nothing. But if you stick to the template, that is including the fs module exactly
the same way naming it also "fs", the compiler will know how to handle the module and its subcomponents. For example:

```javascript
// This is bad:
var EventEmitter = require("events").EventEmitter;

// This is good:
var events = require("events");
var EventEmitter = events.EventEmitter;
```

If a file requires a dependency, it is named in the `@fileoverview` declaration. You should then include the dependency
in your compile step, too.

Testing [![Build Status](https://travis-ci.org/dcodeIO/node.js-closure-compiler-externs.png?branch=master)](https://travis-ci.org/dcodeIO/node.js-closure-compiler-externs)
-------
Externs are automatically syntax-validated through a [ClosureCompiler.js](https://github.com/dcodeIO/ClosureCompiler.js)
test run. This does not imply that the extern is complete or does actually represent the underlying API (but it should).

Usage with ClosureCompiler.js
-----------------------------
[ClosureCompiler.js](https://github.com/dcodeIO/ClosureCompiler.js) depends on an npm distribution of this repository.
As a result, specifiying `--externs=node` automatically includes all node.js specific externs in your compile step. If
you are using non-core modules, you may still need [additional externs](https://github.com/dcodeIO/ClosureCompiler.js#externs-for-advanced_optimizations)
for these.

Downloads
---------
* [ZIP-Archive](https://github.com/dcodeIO/node.js-closure-compiler-externs/archive/master.zip)
* [Tarball](https://github.com/dcodeIO/node.js-closure-compiler-externs/tarball/master)

License
-------
Apache License, Version 2.0 - http://www.apache.org/licenses/LICENSE-2.0.html

This repository is not officially supported by Google, Joyent or individual module authors. If the closure compiler 
license header is used in a file, it is just there so signal that it is ok to include it in official closure channels.
All rights belong to their respective owners.
