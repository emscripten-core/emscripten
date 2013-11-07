
int state = 0;

extern "C" {

void one() {
  state++;
}

int two() {
  return state;
}

}

