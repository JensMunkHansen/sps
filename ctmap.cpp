#include <cstdio>
#include <iostream>

template<char... N> struct interned {
  using type = interned;
#if 0
  interned(int k = 0) {
    type::p = k;
  }
#endif
  static int const p;
  static char const value[];
};

template<char... N>
char const interned<N...>::value[] {N...};

template<char... N>
const int interned<N...>::p = 2;

template<int N>
constexpr char ch(char const(&s)[N], int i) {
  return i < N ? s[i] : '\0';
}

//#define intern_t(s) interned<ch(s,0),ch(s,1),ch(s,2),ch(s,3),ch(s,4),ch(s,5),ch(s,6),ch(s,7)>::type


#define intern_t(s) interned<ch(s,0), ch(s,1), ch(s,2), ch(s,3), ch(s,4), ch(s,5), ch(s,6), ch(s,7), ch(s,8), ch(s, 9), ch(s, 10), ch(s, 11), ch(s,12), ch(s,13), ch(s,14), ch(s,15), ch(s,16), ch(s,17)>::type


#define intern_p(s) interned<ch(s,0), ch(s,1), ch(s,2), ch(s,3), ch(s,4), ch(s,5), ch(s,6), ch(s,7), ch(s,8), ch(s, 9), ch(s, 10), ch(s, 11), ch(s,12), ch(s,13), ch(s,14), ch(s,15), ch(s,16), ch(s,17)>::p


struct singleton {

  template<typename T>
  singleton(T) {
    printf("%s\n", intern_t("hello")::value);
    //intern_p(__PRETTY_FUNCTION__) = 2;

    //    static interned<'a'> bum(2);

    if( T::value == intern_t("db")::value ) {
      printf("%s\n", __PRETTY_FUNCTION__);
      printf("Initializing DB singleton: %s\n", T::value);
    } else {
      printf("Initializing singleton: %s\n", T::value);
    }
  }

  void do_action() {
    printf("singleton::do_action()\n");
  }
};

template<typename K, typename V> struct map {
  static V value;
};

template<typename K, typename V> V map<K,V>::value{K{}};

template<typename K> using get_singleton = map<K,singleton>;

void testMe(const char c) {
  printf("%c\n", c);
}


int main() {

  get_singleton< intern_t("db") >::value.do_action();

  // Ugly - segfault
  // *const_cast<int*>(&(intern_t("hello")::p)) = 1;

  printf("Should be 2: %d\n", interned<'a'>::p);
  testMe('a');

}
