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
  b();
  
  // We expect that b gets replaced by a below
  var f = [b];
  e();

  return;
}

})(Module.asmGlobalArg, Module.asmLibraryArg, buffer);

// {"b":"a"}
