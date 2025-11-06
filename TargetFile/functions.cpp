class C1 {
public:
  int foo() {
    int f11 = 1;
    return f11 + a1;
  }

  static int goo(C1 &gp1, C1 *gp2) {
    int g11 = 2;
    return g11 + gp1.a1 + gp2->a1;
  }
  int a1{};
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
  C1 g1{};
  C1 g2{};
  int g12 = C1::goo(g1, &g2);

  N1::C2 c2;
  c2.yoo();
  N1::zoo();

  return f12 + g12;
}