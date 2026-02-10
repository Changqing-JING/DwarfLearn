#include <tuple>

int main() {
  std::tuple<int, double, char> myTuple{42, 3.14, 'a'};
  return std::get<0>(myTuple) + static_cast<int>(std::get<1>(myTuple)) + static_cast<int>(std::get<2>(myTuple));
}