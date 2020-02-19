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

int main() {
    constexpr auto pf = __PRETTY_FUNCTION__; // gcc ok; clang ok; (1)
    static_assert(std::string_view(__PRETTY_FUNCTION__) == std::string_view("int main()")); // gcc ok; clang ok; (2)
#ifdef __GNUG__
    X<sizeof(__PRETTY_FUNCTION__)-1> x;
#else
    X<strlen(__PRETTY_FUNCTION__)> x; // gcc not ok; clang ok; (3)
#endif
}
