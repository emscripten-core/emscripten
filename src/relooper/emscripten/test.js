// js -m -n -e "load('relooper.js')" test.js

function test() {
  print("-- If shape --\n");

  //Relooper.setDebug(1);

  {
    Relooper.init();

    var b_a = Relooper.addBlock("// block A\n");
    var b_b = Relooper.addBlock("// block B\n");
    var b_c = Relooper.addBlock("// block C\n");

    Relooper.addBranch(b_a, b_b, "check == 10", "atob();");
    Relooper.addBranch(b_a, b_c, 0, "atoc();");

    Relooper.addBranch(b_b, b_c, 0, "btoc();");

    var output = Relooper.render(b_a);
    print(output);
  }

  {
    Relooper.init();

    var b_a = Relooper.addBlock("// block A\n");
    var b_b = Relooper.addBlock("// block B\n");
    var b_c = Relooper.addBlock("// block C\n");

    Relooper.addBranch(b_a, b_b, "check == fee()");
    Relooper.addBranch(b_a, b_c, 0, 0);

    Relooper.addBranch(b_c, b_b);

    var output = Relooper.render(b_a);
    print(output);
  }
}

test();

// TODO: wrap the relooper itself

