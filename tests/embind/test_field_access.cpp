#include <stdio.h>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

using namespace emscripten;

struct vec2
{
  float x, y;
};

struct obj
{
  vec2 vector;
  float f;
};

EMSCRIPTEN_BINDINGS(property) {
  class_<vec2>("vec2")
    .constructor<>()
    .property("x", &vec2::x)
    .property("y", &vec2::y);
  class_<obj>("obj")
    .constructor<>()
    .ref_property("vector", &obj::vector)
    .property("f", &obj::f);
}

int main()
{
  EM_ASM({
	 var o = new Module.obj();
	 // Test persistence assigning fields of the field.
	 o.vector.x = 5;
	 console.log('x = ' + o.vector.x);
	 // Test assignment into the field.
	 var v = new Module.vec2();
	 v.x = 24; v.y = 75;
	 o.vector = v;
	 console.log('field is ' + o.vector.x + ' / ' + o.vector.y);
	 // Test that o.vector and v are still separate.
	 v.x = 3; v.y = 8;
	 console.log('vector is ' + v.x + ' / ' + v.y);
	 console.log('field is ' + o.vector.x + ' / ' + o.vector.y);
	 // Test that assignment works the other way.
	 v = o.vector;
	 o.vector.x = 18; o.vector.y = 32;
	 console.log('vector is ' + v.x + ' / ' + v.y);
	 console.log('field is ' + o.vector.x + ' / ' + o.vector.y);
	 o.delete();
	 v.delete();
    });
  return 0;
}
