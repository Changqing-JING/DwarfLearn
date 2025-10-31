template <typename T>
class C {
public:
  unsigned char data[sizeof(T)];
};

int main() {
  C<int> c_int{};
  C<double> c_double{};
  return c_int.data[0] + c_double.data[0];
}