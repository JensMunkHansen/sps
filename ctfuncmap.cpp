/**
 * @file   ctfuncmap.cpp
 * @author Jens Munk Hansen <jmh@debian9laptop.parknet.dk>
 * @date   Wed Feb 19 18:42:20 2020
 *
 * @brief
 *
 * Copyright 2020
 *
 */

#include <iostream>
#include <map>
#include <string>

// Same prototype, but different names

using Map = std::map<std::string, int (*)()>;

//-------------------------------------//

template <int (*)()>
struct Get
{
  static constexpr std::string name()
  {
    std::string tmp = __PRETTY_FUNCTION__;
    auto s = tmp.find("= ");
    auto e = tmp.find("; ");
    return std::string(tmp.substr(s + 2, e - s - 2));
  }
};

template <int (*func)()>
void insert2map(Map& fctmap)
{
  fctmap[Get<func>::name()] = func;
}

int first()
{
  return 0;
}

int second()
{
  return 1;
}

template <char... N>
struct interned
{
  using type = interned;
  static char const value[];
};

template <char... N>
char const interned<N...>::value[]{ N... };

template <int N>
constexpr char ch(char const (&s)[N], int i)
{
  return i < N ? s[i] : '\0';
}

#define intern_t(s) interned<ch(s,0), ch(s,1), ch(s,2), ch(s,3), ch(s,4), ch(s,5), ch(s,6), ch(s,7), ch(s,8), ch(s, 9), ch(s, 10), ch(s, 11), ch(s,12), ch(s,13), ch(s,14), ch(s,15, ch(s,16), ch(s,17)>::type;

class A
{
public:
  A() {}
  ~A() {}

  int fun(float f)
  {
    int retval = static_cast<int>(f);
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return retval;
  }
  int bla() { return 0; }
};

int main(int arc, char* argv[])
{
  A a;

  int k = a.fun(1.0f);

  Map fctmap;

  insert2map<first>(fctmap);
  insert2map<second>(fctmap);

  for (auto&& i : fctmap)
    std::cout << i.first << " -> " << i.second() << std::endl;

  // intern_t("db")::value
  return 0;
}
