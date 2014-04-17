var sme = new Module.Parent(42);
sme.mulVal(2);
Module.print('*')
Module.print(sme.getVal());

Module.print('c1');

var c1 = new Module.Child1();
Module.print(c1.getVal());
c1.mulVal(2);
Module.print(c1.getVal());
Module.print(c1.getValSqr());
Module.print(c1.getValSqr(3));
Module.print(c1.getValTimes()); // default argument should be 1
Module.print(c1.getValTimes(2));

Module.print('c1 v2');

c1 = new Module.Child1(8); // now with a parameter, we should handle the overloading automatically and properly and use constructor #2
Module.print(c1.getVal());
c1.mulVal(2);
Module.print(c1.getVal());
Module.print(c1.getValSqr());
Module.print(c1.getValSqr(3));

Module.print('c2')

var c2 = new Module.Child2();
Module.print(c2.getVal());
c2.mulVal(2);
Module.print(c2.getVal());
Module.print(c2.getValCube());
var succeeded;
try {
  succeeded = 0;
  Module.print(c2.doSomethingSecret()); // should fail since private
  succeeded = 1;
} catch(e) {}
Module.print(succeeded);
try {
  succeeded = 0;
  Module.print(c2.getValSqr()); // function from the other class
  succeeded = 1;
} catch(e) {}
Module.print(succeeded);
try {
  succeeded = 0;
  c2.getValCube(); // sanity
  succeeded = 1;
} catch(e) {}
Module.print(succeeded);

Module.Child2.prototype.printStatic(); // static calls go through the prototype

// virtual function
c2.virtualFunc();
Module.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();

// extend a class from JS
var c3 = new Module.Child2JS;

c3.virtualFunc = function() {
  Module.print('*js virtualf replacement*');
};
c3.virtualFunc2 = function() {
  Module.print('*js virtualf2 replacement*');
};

c3.virtualFunc();
Module.Child2.prototype.runVirtualFunc(c3);
c3.virtualFunc2();

c2.virtualFunc(); // original should remain the same
Module.Child2.prototype.runVirtualFunc(c2);
c2.virtualFunc2();
Module.print('*ok*');

