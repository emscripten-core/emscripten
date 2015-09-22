#include <stdio.h>
#include <string.h>

int main() {
  char test[80], blah[80];
  const char *sep = "\\/:;=-";
  char *word, *phrase, *brkt, *brkb;

  strcpy(test, "This;is.a:test:of=the/string\\tokenizer-function.");

  for (word = strtok_r(test, sep, &brkt); word;
       word = strtok_r(NULL, sep, &brkt)) {
    strcpy(blah, "blah:blat:blab:blag");
    for (phrase = strtok_r(blah, sep, &brkb); phrase;
         phrase = strtok_r(NULL, sep, &brkb)) {
      printf("at %s:%s\n", word, phrase);
    }
  }
  return 0;
}
