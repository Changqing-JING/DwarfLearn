#include <vector>
#include "A.h"
class A {

public:
#ifdef __CPTC__
  __noinline
#endif
      void
      foo() {
    std::vector<int> v1;
    while (i < 1000) {
      v1.push_back(i);
      i++;
    }

    while (i < 2000) {

      i *= 2;
    }

    while (i > 1) {

      i--;
    }

    while (i != 5) {

      i /= 22;
    }
  }

  int get() {
    return i;
  }

private:
  int i = 0;
};

int goo() {
  A a;
  a.foo();
  return a.get();
}
