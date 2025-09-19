
struct ST {
  int x;
  int y;
};
int goo() {
  ST st;
  st.x = 10;
  int a = st.x;
  int b = 2;
  int c = a + b;
  return c;
}
