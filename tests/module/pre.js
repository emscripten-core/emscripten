// Add this to any program using --pre-js to do a test of Module.print and printErr with multi-line output.
Module.print("This is some text...\nThis is some more...\n\nAnd this is some more again\n\nTrailing bit... ");
Module.print("...Complete Trailer\n\n");
Module.printErr("This is some ERROR text...\nThis is some more...\n\nAnd this is some more again\n\nTrailing bit... ");
Module.printErr("...Complete Trailer\n\n");
