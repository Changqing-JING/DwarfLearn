class C1 {
public:
  int foo() {
    int f11 = 1;
    return f11;
  }

  static int goo() {
    int g11 = 2;
    return g11;
  }
};

namespace N1 {
class C2 {
public:
  void yoo() {
  }
};

void zoo() {
}
} // namespace N1

int main() {
  C1 c1;
  int f12 = c1.foo();
  int g12 = C1::goo();

  N1::C2 c2;
  c2.yoo();
  N1::zoo();

  return f12 + g12;
}