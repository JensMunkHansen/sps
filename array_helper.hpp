template <typename T, size_t N>
char (&ArraySizeHelper(T (&array)[N]))[N];
#define arraysize(array) (sizeof(ArraySizeHelper(array)))

#define expect(expr)                                                                               \
  if (!expr)                                                                                       \
    cerr << "Assertion " << #expr " failed at " << __FILE__ << ":" << __LINE__ << endl;

#define stringify(x) #x
#define tostring(x) stringify(x)
#define MAGIC_CONSTANT 314159

cout << "Value of MAGIC_CONSTANT=" << tostring(MAGIC_CONSTANT);

#define dbg(v) copy(v.begin(), v.end(), ostream_iterator<typeof(*v.begin())>(cout, " "))

#include <iostream>
int main()
{
  using namespace std;
http: // www.google.com
  int x = 5;
  cout << x;
}
