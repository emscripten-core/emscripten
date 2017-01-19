Objective-C support
===================

This directory provides some experimental code for testing Objective-C
support. At the moment, it does not actually run any Objective-C code,
due to no runtime being present.

Blame <ivan@vucica.net> for this text. Just don't sue him. :-)

Compiling process
-----------------

Ideally, Objective-C can become a first-class citizen in the Emscripten
compiling process. Unfortunately, the LLVM bitcode generated for
Objective-C files breaks some assumptions made by Emscripten, such as
symbols having no spaces.

Hence, the easiest build process -- via `hello-direct.sh` -- cannot be
used. (As of May 9th 2013, Emscripten doesn't even recognize `.m` and
`.mm` as valid extensions, for Objective-C and Objective-C++
respectively.)

The alternative build process -- via `hello-via-c.sh` -- uses a small
wrapper called `emobjcc-miniwrapper` (which can ideally become `emobjcc`,
or maybe even fold into `emcc`). It's a shell script that tries to dumbly
pass a file as the first argument, and additional nine arguments, to various
processing stages.

First, it uses the `-rewrite-objc` option to generate a piece of C code
using Clang. This could theoretically be handled by Emscripten even at
this phase. (While the documentation states that this stage should be
generating C++ code and not C code, we treat the output as C code. C code 
produced from experimental Objective-C code in `hello.m` seems to be
handled alright by Clang's C compiler, and, as opposed to being built in
C++ mode, it produces C-style symbols for essential runtime functions.)

Yes, that's right -- Objective-C's "ugly" brackets can be turned into C calls
and Objective-C's class definitions become nothing but structures.

Theoretically it could be handled -- but, not so fast! Instead of handing
this code over directly to Emscripten, we use Clang once more to prefix some
headers and generate an `.ll` file containing the bitcode.

Once this is done, we can finally call `emcc` and try compiling the C code,
outputing it into `hello.html`.

Runtime
-------

Testing was done under OS X, hence the thoughts below relate mostly to the
Apple runtime.

The text is a large simplification and there are far better texts 
documenting Objective-C and the runtime out there. The text may even be 
incorrect. The text is here to provide an overview of issues essential
to implementing a basic language support in Emscripten, without forcing the
reader to comprehend Objective-C in great detail (or perhaps providing an
incentive to study it). It is definitely not a comprehensive (or maybe even
correct) look at the language.

### What is an Objective-C runtime?

To actually run, Objective-C code needs a runtime. This is because every call
such as:

    [X print];

turns into several function calls (any casts omitted; Apple-runtime-based):

    objc_msgSend(objc_getClass("X"), sel_registerSelector("print"));

Hence, Emscripten will need an implementation of an Objective-C runtime.

### How to get a runtime?

We can't, yet. Some elementary problems need to be resolved first.

Once the elementary compiling support stabilizes, there are a few possible
options:

 - linking with a free runtime, such as `libobjc2` in GNUstep or with Apple's
   runtime
 - writing a mini-runtime specific to Emscripten
 - hacking a wrapper around Objective-J language

The last option is probably the most interesting one, while the former two
would allow for greater portability of existing Objective-C code. Existing
code however already highly depends on implementations of other base libraries
such as Foundation (containing strings, arrays, et al) and AppKit (containing
windows, buttons, et al); so to make things usable, a free library such as
GNUstep or Cocotron would also have to be ported.

Let's examine Objective-J.

Objective-J is a language that does for Javascript what Objective-C does for
C. It brings class-based inheritance, it brings the "message-send style"
method calls (with the "ugly" brackets), and is preprocessed into Javascript.
Objective-J even has a runtime with calls very similar to Objective-C
runtime, with functions such as `objj_msgSend()` appearing all over the output
Javascript.

In combination with the Cappuccino framework, we're essentially getting a
browser-native implementation of Foundation and AppKit (only having to replace
`NS` prefix in class names with `CP`).

Another reason for writing at least partially Emscripten-specific
implementation (a wrapper around Objective-J or a mini-runtime) is the fact
that every current runtime needs to have at least some of the functions (such as
the message dispatch methods like `objc_msgSend()`) implemented in platform
specific assembly. This is because `objc_msgSend()`'s signature is
(approximately) as follows:

    id objc_msgSend(id self, SEL _cmd, ...)
    // * "id" is a pointer to a structure representing a valid Objective-C
    //   object
    // * "self" is the same thing as C++'s "this"
    // * "SEL" is a selector -- a 'method name'; with Apple runtime, this is a
    //   C string

An Objective-C method implementation is a method with (typically) the
following signature:

    id CompilerGeneratedMethodName(id self, SEL _cmd, ...)

Note the `...`; the assembly is used to pass the actual method arguments into
the actual method implementation.

While we could replace just message dispatch functions with an
Emscripten-specific way of doing this, it may make more sense to adopt an
Emscripten-specific runtime.

This is, of course, only speculative discussion at this point -- since
some essential changes to Emscripten need to be done first.

### On Objective-C objects

Objective-C objects are essentially C structures whose first element is
`Class isa`, pointing to another valid Objective-C object containing a
description of the C structure and the methods that the class contains.

### Dummy runtime

A small implementation of only three methods is located in `dummy-runtime.js`:

 * `objc_getClass()` should ordinarily take a C string and return a pointer to
   a class structure.
  
    For testing purposes, it just prints out a debug
   statement and returns a Javascript string with the value

 * `sel_registerName()` should ordinarily take a C string, ensure that it is
   unique at runtime (that is, one C string value defines a single selector,
   despite possible multiple pointers being passed to the function) and then
   return a `SEL` typed value defining a selector. With Apple runtime (which
   we target shere), a `SEL` is a `const char*`. While this is not something
   one should depend on in ordinary situations, for Emscripten this is good
   enough -- this fact won't change overnight. 
   
    For testing purposes, this function just prints out a debug statement and
   returns a Javascript string.

 * `objc_msgSend()` takes an Objective-C object (that is, pointer to a
   structure defining an object, starting with `isa`) and a selector, finds
   the function in the vtable, and makes the call. It is worth noting that
   messages can be sent to classes themselves and not just to objects
   (not just to instances of classes); these are methods marked by `+` instead
   of `-`. In C++, these methods would be marked with `static`

    For testing purposes, this function just prints out a debug statement
    and does not return anything.

This runtime is no runtime at all, at the moment, and is just used to get
rid of the Javascript errors and test what's going on with these few methods.


