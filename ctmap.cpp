#include <cstdio>

template<char... N> struct interned {
  using type = interned;
  static char const value[];
};

template<char... N> char const interned<N...>::value[]{N...};

template<int N>
constexpr char ch(char const(&s)[N], int i) {
  return i < N ? s[i] : '\0';
}

#define intern_t(s) interned<ch(s,0),ch(s,1),ch(s,2),ch(s,3),ch(s,4),ch(s,5),ch(s,6),ch(s,7)>::type

struct singleton {

  template<typename T>
  singleton(T) {
    if( T::value == intern_t("db")::value )
      printf("Initializing DB singleton: %s\n", T::value);
    else
      printf("Initializing singleton: %s\n", T::value);
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

int main() {

  get_singleton< intern_t("db") >::value.do_action();

}
