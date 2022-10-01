#include <iostream>
#include <type_traits>
#include <utility>

namespace detail {

template<class T, T... inds, class F>
constexpr void loop(std::integer_sequence<T, inds...>, F&& f) {
  (f(std::integral_constant<T, inds>{}), ...);// C++17 fold expression
}

}// detail

template<class T, T count, class F>
constexpr void loop(F&& f) {
  detail::loop(std::make_integer_sequence<T, count>{}, std::forward<F>(f));
}

int main() {
  loop<int, 5>([] (auto i) {
    constexpr int it_is_even_constexpr = i;
    std::cout << it_is_even_constexpr << std::endl;
  });
  return 0;
}
