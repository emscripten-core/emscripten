(async function main(){
// If MODULARIZE is used, `TheModule` is a promise.
if (TheModule instanceof Promise) {
  TheModule = await TheModule;
}

// Part 1

var sme = new TheModule.Parent(42);
sme.mulVal(2);
console.log('*')
console.log(sme.getVal());
sme.parentFunc(90);
console.log(typeof sme.getAsConst());
console.log(typeof sme.voidStar(sme));
console.log(sme.get_immutableAttr());
console.log(sme.immutableAttr);

try {
  sme.immutableAttr = 1;
} catch(e) {}
console.log(sme.immutableAttr); // Should be unchanged
console.log(sme.attr);
console.log(sme.get_attr());
sme.attr = 9;
console.log(sme.attr);
sme.set_attr(10);
console.log(sme.attr);
console.log(sme.get_attr());

console.log(typeof sme.getBoolean());
console.log(sme.getBoolean());

console.log('c1');

var c1 = new TheModule.Child1();
console.log(c1.getVal());
c1.mulVal(2);
console.log(c1.getVal());
console.log(c1.getValSqr());
console.log(c1.getValSqr(3));
console.log(c1.getValTimes()); // default argument should be 1
console.log(c1.getValTimes(2));
console.log(sme.getBoolean());
c1.parentFunc(90);

console.log('c1 v2');

c1 = new TheModule.Child1(8); // now with a parameter, we should handle the overloading automatically and properly and use constructor #2
console.log(c1.getVal());
c1.mulVal(2);
console.log(c1.getVal());
console.log(c1.getValSqr());
console.log(c1.getValSqr(3));
console.log(sme.getBoolean());

console.log('c2')

var c2 = new TheModule.Child2();
console.log(c2.getVal());
c2.mulVal(2);
console.log(c2.getVal());
console.log(c2.getValCube());
var succeeded;
try {
  succeeded = 0;
  console.log(c2.doSomethingSecret()); // should fail since private
  succeeded = 1;
} catch(e) {}
console.log(succeeded);
try {
  succeeded = 0;
  console.log(c2.getValSqr()); // function from the other class
  succeeded = 1;
} catch(e) {}
console.log(succeeded);
try {
  succeeded = 0;
  c2.getValCube(); // sanity
  succeeded = 1;
} catch(e) {}
console.log(succeeded);

TheModule.Child2.prototype.printStatic(42); // static calls go through the prototype

// virtual function
c2.virtualFunc();
TheModule.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();

// extend a class from JS
var c3 = new TheModule.Child2JS;

c3.virtualFunc = function() {
  console.log('*js virtualf replacement*');
};
c3.virtualFunc2 = function() {
  console.log('*js virtualf2 replacement*');
};
c3.virtualFunc3 = function(x) {
  console.log('*js virtualf3 replacement ' + x + '*');
};

c3.virtualFunc();
TheModule.Child2.prototype.runVirtualFunc(c3);
c3.virtualFunc2();
c3.virtualFunc3(123); // this one is not replaced!
try {
  c3.virtualFunc4(123);
} catch(e) {
  console.log('caught: ' + e);
}

// Test virtual method dispatch from c++
TheModule.Child2.prototype.runVirtualFunc3(c3, 43);

c2.virtualFunc(); // original should remain the same
TheModule.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();
console.log('*ok*');

// Part 2

var suser = new TheModule.StringUser("hello", 43);
suser.Print(41, "world");
suser.PrintFloat(12.3456);
console.log(suser.returnAString());

var bv = new TheModule.RefUser(10);
var bv2 = new TheModule.RefUser(11);
console.log(bv2.getValue(bv));

console.log(typeof bv2.getMe());
console.log(bv2.getMe().getValue(bv));
console.log(bv2.getMe().getValue(bv2));

console.log(typeof bv2.getCopy());
console.log(bv2.getCopy().getValue(bv));
console.log(bv2.getCopy().getValue(bv2));

bv2.getAnother().PrintFloat(21.12);

console.log(new TheModule.Inner().get());
console.log('getAsArray: ' + new TheModule.Inner().getAsArray(12));
new TheModule.Inner().mul(2);
new TheModule.Inner().incInPlace(new TheModule.Inner());

console.log(TheModule.enum_value1);
console.log(TheModule.enum_value2);

// Enums from classes are accessed via the class.
var enumClassInstance = new TheModule.EnumClass();
console.log([enumClassInstance.GetEnum(), TheModule.EnumClass.e_val].join(','));

// Enums from namespaces are accessed via the top-level module, as with classes defined
// in namespaces, see `Inner` above.
console.log(TheModule.e_namespace_val);

var typeTester = new TheModule.TypeTestClass();

console.log('return char ' + (typeTester.ReturnCharMethod() & 255));
typeTester.AcceptCharMethod((2<<6)-1);
typeTester.AcceptCharMethod(-1);

console.log('return unsigned char ' + (typeTester.ReturnUnsignedCharMethod() & 255));
typeTester.AcceptUnsignedCharMethod((2<<7)-1);

console.log('return unsigned short ' + (typeTester.ReturnUnsignedShortMethod() & 65535));
typeTester.AcceptUnsignedShortMethod((2<<15)-1);

console.log('return unsigned long ' + (typeTester.ReturnUnsignedLongMethod() | 0));
typeTester.AcceptUnsignedLongMethod((2<<31)-1);
var voidPointerUser = new TheModule.VoidPointerUser();

voidPointerUser.SetVoidPointer(3);
console.log('void * ' + voidPointerUser.GetVoidPointer());

// Array tests

var arrayClass = new TheModule.ArrayClass();
console.log('int_array[0] == ' + arrayClass.get_int_array(0));
console.log('int_array[7] == ' + arrayClass.get_int_array(7));
arrayClass.set_int_array(0, 42);
arrayClass.set_int_array(7, 43);
console.log('int_array[0] == ' + arrayClass.get_int_array(0));
console.log('int_array[7] == ' + arrayClass.get_int_array(7));

try {
  arrayClass.set_int_array(-1, 42);
} catch (e) {
  console.log('idx -1: ' + e);
}

try {
  arrayClass.set_int_array(8, 42);
} catch (e) {
  console.log('idx 8: ' + e);
}

console.log('struct_array[0].attr1 == ' + arrayClass.get_struct_array(0).get_attr1());
console.log('struct_array[0].attr2 == ' + arrayClass.get_struct_array(0).get_attr2());
console.log('struct_array[7].attr1 == ' + arrayClass.get_struct_array(7).get_attr1());
console.log('struct_array[7].attr2 == ' + arrayClass.get_struct_array(7).get_attr2());

// Verify that bounds checking is *not* enabled when not asked for.
// This actually causes an illegal memory access, but as it's only a read, and the return
// value is not used, it shouldn't cause any problems in practice.
arrayClass.get_struct_array(8);

var struct = new TheModule.StructInArray(13, 17);
arrayClass.set_struct_array(0, struct);
struct = new TheModule.StructInArray(14, 18);
arrayClass.set_struct_array(7, struct);

console.log('struct_array[0].attr1 == ' + arrayClass.get_struct_array(0).get_attr1());
console.log('struct_array[0].attr2 == ' + arrayClass.get_struct_array(0).get_attr2());
console.log('struct_array[7].attr1 == ' + arrayClass.get_struct_array(7).get_attr1());
console.log('struct_array[7].attr2 == ' + arrayClass.get_struct_array(7).get_attr2());

struct = new TheModule.StructInArray(100, 101);
arrayClass.set_struct_ptr_array(0, struct);
console.log('struct_ptr_array[0]->attr1 == ' + arrayClass.get_struct_ptr_array(0).get_attr1());
console.log('struct_ptr_array[0]->attr2 == ' + arrayClass.get_struct_ptr_array(0).get_attr2());

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

// Returned pointers (issue 14745)

var factory = new TheModule.ObjectFactory();
var objectProvider = factory.getProvider();
var smallObject = objectProvider.getObject();

// This will print 123 if we managed to access the object, which means that integers
// were correctly typecast to ObjectProvider pointer and SmallObject pointer.
console.log(smallObject.getID(123));

TheModule.destroy(factory)

// end of issue 14745

// octet[] to char* (issue 14827)

const arrayTestObj = new TheModule.ArrayArgumentTest();
const bufferAddr = TheModule._malloc(35);
TheModule.stringToUTF8('I should match the member variable', bufferAddr, 35);

const arrayTestResult = arrayTestObj.byteArrayTest(bufferAddr);
const arrayDomStringResult = arrayTestObj.domStringTest('I should match the member variable');
console.log(arrayTestResult);
console.log(arrayDomStringResult);

TheModule.destroy(arrayTestObj)
TheModule._free(bufferAddr);
	
// end of issue 14827

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
    console.log('ERROR: An array was not copied to HEAP32 after memory reallocation');
  }
}

console.log('\ndone.')
})();
