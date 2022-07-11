#include <type_traits>
#include <string_view>

constexpr std::size_t strlen(char const* s) {
  std::size_t n = 0;
  while (*s++ != '\0')
    ++n;
  return n;
}

template <std::size_t>
struct X {};

static char const PF[] = "int main()";

int main() {
  constexpr auto pf = std::string_view(__PRETTY_FUNCTION__); // gcc ok; clang ok; (1)
  static_assert(pf == std::string_view("int main()")); // gcc ok; clang ok; (2)
  X<strlen(__PRETTY_FUNCTION__)> x; // gcc not ok; clang ok; (3)

  static_assert(pf[0] == 'i'); // not ok; (4)
  X<pf.size()> x1; // ok; (5)
  X<strlen(pf.data())> x2; // not ok; (6)

  static_assert(__builtin_constant_p(pf.data()) == 0); // ok (7)
  static_assert(__builtin_strlen(pf.data()) == 10); // ok (8)
  static_assert(__builtin_memcmp(pf.data(), "int main()", __builtin_strlen(pf.data())) == 0); // ok (9)
  static_assert(std::char_traits<char>::compare(pf.data(), "int main()", std::char_traits<char>::length(pf.data())) == 0); // ok (10)

  static_assert(std::char_traits<char>::length(PF) == 10); // not ok (11)
  static_assert(__builtin_strlen(PF) == 10); // not ok (12)
}
