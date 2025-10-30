
double gd;
int gi;

void foo(int f1) {
  if (f1) {
    int fa = 1;
    gi = fa;
  } else {
    int fb = 2;
    gi = fb;
  }

  if (f1) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
    int fc = 3;
#pragma GCC diagnostic pop
  }
}

int main() {
  int x = 10;
  {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
    double x = 5;
#pragma GCC diagnostic pop
    gd = x;
  }
  gi = x;
  return 0;
}