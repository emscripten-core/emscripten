function init_cd_test(name) {
  console.log("-------------- Starting test " + name + " --------------");
  Module['cyberdwarf'].initialize_debugger();
}

function TestException(message) {
   this.message = message;
   this.name = "TestException";
}

function test_assert(desc, value) {
  if (!value) {
    throw new TestException("Test case " + desc + " failed!");
  }
}
