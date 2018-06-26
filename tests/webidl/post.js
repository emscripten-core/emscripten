
// Part 1

var sme = new TheModule.Parent(42);
sme.mulVal(2);
out('*')
out(sme.getVal());
sme.parentFunc(90);
out(typeof sme.getAsConst());
out(typeof sme.voidStar(sme));
out(sme.get_immutableAttr());
out(typeof sme.getBoolean());
out(sme.getBoolean());

out('c1');

var c1 = new TheModule.Child1();
out(c1.getVal());
c1.mulVal(2);
out(c1.getVal());
out(c1.getValSqr());
out(c1.getValSqr(3));
out(c1.getValTimes()); // default argument should be 1
out(c1.getValTimes(2));
out(sme.getBoolean());
c1.parentFunc(90);

out('c1 v2');

c1 = new TheModule.Child1(8); // now with a parameter, we should handle the overloading automatically and properly and use constructor #2
out(c1.getVal());
c1.mulVal(2);
out(c1.getVal());
out(c1.getValSqr());
out(c1.getValSqr(3));
out(sme.getBoolean());

out('c2')

var c2 = new TheModule.Child2();
out(c2.getVal());
c2.mulVal(2);
out(c2.getVal());
out(c2.getValCube());
var succeeded;
try {
  succeeded = 0;
  out(c2.doSomethingSecret()); // should fail since private
  succeeded = 1;
} catch(e) {}
out(succeeded);
try {
  succeeded = 0;
  out(c2.getValSqr()); // function from the other class
  succeeded = 1;
} catch(e) {}
out(succeeded);
try {
  succeeded = 0;
  c2.getValCube(); // sanity
  succeeded = 1;
} catch(e) {}
out(succeeded);

TheModule.Child2.prototype.printStatic(); // static calls go through the prototype

// virtual function
c2.virtualFunc();
TheModule.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();

// extend a class from JS
var c3 = new TheModule.Child2JS;

c3.virtualFunc = function() {
  out('*js virtualf replacement*');
};
c3.virtualFunc2 = function() {
  out('*js virtualf2 replacement*');
};
c3.virtualFunc3 = function(x) {
  out('*js virtualf3 replacement ' + x + '*');
};

c3.virtualFunc();
TheModule.Child2.prototype.runVirtualFunc(c3);
c3.virtualFunc2();
c3.virtualFunc3(123); // this one is not replaced!
try {
  c3.virtualFunc4(123);
} catch(e) {
  out('caught: ' + e);
}

// Test virtual method dispatch from c++
TheModule.Child2.prototype.runVirtualFunc3(c3, 43);

c2.virtualFunc(); // original should remain the same
TheModule.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();
out('*ok*');

// Part 2

var suser = new TheModule.StringUser("hello", 43);
suser.Print(41, "world");
suser.PrintFloat(12.3456);
out(suser.returnAString());

var bv = new TheModule.RefUser(10);
var bv2 = new TheModule.RefUser(11);
out(bv2.getValue(bv));

out(typeof bv2.getMe());
out(bv2.getMe().getValue(bv));
out(bv2.getMe().getValue(bv2));

out(typeof bv2.getCopy());
out(bv2.getCopy().getValue(bv));
out(bv2.getCopy().getValue(bv2));

bv2.getAnother().PrintFloat(21.12);

out(new TheModule.Inner().get());
out('getAsArray: ' + new TheModule.Inner().getAsArray(12));
new TheModule.Inner().mul(2);

out(TheModule.enum_value1);
out(TheModule.enum_value2);

// Enums from classes are accessed via the class.
enumClassInstance = new TheModule.EnumClass();
out([enumClassInstance.GetEnum(), TheModule.EnumClass.e_val].join(','));

// Enums from namespaces are accessed via the top-level module, as with classes defined
// in namespaces, see `Inner` above.
out(TheModule.e_namespace_val);

typeTester = new TheModule.TypeTestClass();

out('return char ' + typeTester.ReturnCharMethod());
typeTester.AcceptCharMethod((2<<6)-1);
// Prints -1 because the c++ code accepts unsigned char.
typeTester.AcceptCharMethod((2<<7)-1);

// Prints -1 because all integers are signed in javascript.
out('return unsigned char ' + typeTester.ReturnUnsignedCharMethod());
typeTester.AcceptUnsignedCharMethod((2<<7)-1);

// Prints -1 because all integers are signed in javascript.
out('return unsigned short ' + typeTester.ReturnUnsignedShortMethod());
typeTester.AcceptUnsignedShortMethod((2<<15)-1);

// Prints -1 because all integers are signed in javascript.
out('return unsigned long ' + typeTester.ReturnUnsignedLongMethod());
typeTester.AcceptUnsignedLongMethod((2<<31)-1);
var voidPointerUser = new TheModule.VoidPointerUser();

voidPointerUser.SetVoidPointer(3);
out('void * ' + voidPointerUser.GetVoidPointer());

// Array tests

var arrayClass = new TheModule.ArrayClass();
out('int_array[0] == ' + arrayClass.get_int_array(0));
out('int_array[7] == ' + arrayClass.get_int_array(7));
arrayClass.set_int_array(0, 42);
arrayClass.set_int_array(7, 43);
out('int_array[0] == ' + arrayClass.get_int_array(0));
out('int_array[7] == ' + arrayClass.get_int_array(7));

try {
  arrayClass.set_int_array(-1, struct);
} catch (e) {
  out('idx -1: ' + e);
}

try {
  arrayClass.set_int_array(8, struct);
} catch (e) {
  out('idx 8: ' + e);
}

out('struct_array[0].attr1 == ' + arrayClass.get_struct_array(0).get_attr1());
out('struct_array[0].attr2 == ' + arrayClass.get_struct_array(0).get_attr2());
out('struct_array[7].attr1 == ' + arrayClass.get_struct_array(7).get_attr1());
out('struct_array[7].attr2 == ' + arrayClass.get_struct_array(7).get_attr2());

// Verify that bounds checking is *not* enabled when not asked for.
// This actually causes an illegal memory access, but as it's only a read, and the return
// value is not used, it shouldn't cause any problems in practice.
arrayClass.get_struct_array(8);

var struct = new TheModule.StructInArray(13, 17);
arrayClass.set_struct_array(0, struct);
struct = new TheModule.StructInArray(14, 18);
arrayClass.set_struct_array(7, struct);

out('struct_array[0].attr1 == ' + arrayClass.get_struct_array(0).get_attr1());
out('struct_array[0].attr2 == ' + arrayClass.get_struct_array(0).get_attr2());
out('struct_array[7].attr1 == ' + arrayClass.get_struct_array(7).get_attr1());
out('struct_array[7].attr2 == ' + arrayClass.get_struct_array(7).get_attr2());

struct = new TheModule.StructInArray(100, 101);
arrayClass.set_struct_ptr_array(0, struct);
out('struct_ptr_array[0]->attr1 == ' + arrayClass.get_struct_ptr_array(0).get_attr1());
out('struct_ptr_array[0]->attr2 == ' + arrayClass.get_struct_ptr_array(0).get_attr2());

// receiving arrays

var receiver = new TheModule.ReceiveArrays();
receiver.giveMeArrays([0.5, 0.25, 0.01, -20.42], [1, 4, 9, 10], 4);

// Test IDL_CHECKS=ALL

try {
  p = new TheModule.Parent(NaN); // Expects an integer
} catch (e) {}

try {
  p = new TheModule.Parent(42);
  p.voidStar(1234) // Expects a wrapped pointer
} catch (e) {}

try {
  s = new TheModule.StringUser('abc', 1);
  s.Print(123, null); // Expects a string or a wrapped pointer
} catch (e) {}

// Check for overflowing the stack

var before = Date.now();

for (var i = 0; i < 1000000; i++) {
  var temp = new TheModule.StringUser('abc', 1);
  TheModule.destroy(temp);
  if (Date.now() - before >= 1000) break;
}

if (isMemoryGrowthAllowed) {
  // Check for HEAP reallocation when using large arrays
  var numArrayEntries = 100000;
  var intArray = new Array(numArrayEntries);
  for (var i = 0; i < numArrayEntries; i++) {
    intArray[i] = i;
  }

  var startHeapLength = TheModule['HEAP8'].length;
  var offset;
  var storeArray = new TheModule.StoreArray();
  storeArray.setArray(intArray);
  // Add more data until the heap is reallocated
  while (TheModule['HEAP8'].length === startHeapLength) {
    intArray = intArray.concat(intArray);
    storeArray.setArray(intArray);
  }
  
  // Make sure the array was copied to the newly allocated HEAP
  var numCopiedEntries = 0;
  for (var i = 0; i < intArray.length; i++) {
    if (storeArray.getArrayValue(i) !== intArray[i]) {
      break;
    }
    numCopiedEntries += 1;
  }

  if (intArray.length !== numCopiedEntries) {
    out('ERROR: An array was not copied to HEAP32 after memory reallocation');
  }
}

//

out('\ndone.')

