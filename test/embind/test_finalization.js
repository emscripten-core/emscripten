Module.onRuntimeInitialized = () => {
  const foo1 = new Module["Foo"]("Constructed from JS");
  const foo2 = Module["foo"]();
  const foo3 = Module["pFoo"]();
  setTimeout(gc, 100);
}
