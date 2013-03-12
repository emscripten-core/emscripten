      #include <stdio.h>
        #include <emscripten.h>

        int main() {
          char *c = "©¬®¶÷ø";
          printf("%s\n",c);
          printf("©¬®¶÷ø\n");
//          printf("%d %d %d %d %d %d %s\n", c[0]&0xff, c[1]&0xff, c[2]&0xff, c[3]&0xff, c[4]&0xff, c[5]&0xff, c);
//          emscripten_run_script("cheez = _malloc(100);"
//                                "Module.writeStringToMemory(\"©¬®¶÷ø\", cheez);"
//                                "Module.print([Pointer_stringify(cheez), Module.getValue(cheez, 'i8')&0xff, Module.getValue(cheez+1, 'i8')&0xff, Module.getValue(cheez+2, 'i8')&0xff, Module.getValue(cheez+3, 'i8')&0xff, Module.getValue(cheez+4, 'i8')&0xff, Module.getValue(cheez+5, 'i8')&0xff,]);");
        }

