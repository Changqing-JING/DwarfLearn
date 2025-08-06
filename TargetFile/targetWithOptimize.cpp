void foo(int);

int goo(int x, int y) {
  foo(y);
  return y + x;
}

int main() {
}