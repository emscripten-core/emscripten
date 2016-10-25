var asm = (function(global, env, buffer) {
"use asm";
function a()
{
return 0;
}

function b()
{
return 0;
}

function c()
{
  a();
  return;
}

function d()
{
  // Because a is used both as a variable and a function, we will
  // not use a as a candidate for replacement, nor will we replace
  // calls to b with a.
  var a = 0;
  b();
  return;
}

})(Module.asmGlobalArg, Module.asmLibraryArg, buffer);

// {}
