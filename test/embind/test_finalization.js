Module.onRuntimeInitialized = () => {
  console.log("start");
  const foo1 = new Module["Foo"]("Constructed from JS");
  const foo2 = Module["foo"]();
  const foo3 = Module["pFoo"]();
  console.log("done");
  setTimeout(() => { console.log("timeout"); gc() }, 100);
}
