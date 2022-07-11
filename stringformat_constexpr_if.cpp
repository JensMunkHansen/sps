#include <string>
#include <iostream>
#include <memory>

#include <sps/cenv.h>

#ifndef CXX17
# error
#endif

#ifdef _MSC_VER
# pragma warning(disable : 4996)
using mysnprintf = _snprintf;
#else
//using mysnprintf = std::snprintf;
#endif

/**
 * Convert all std::strings to const char* using constexpr if (C++17)
 */
template<typename T>
auto convert(T&& t) {
  if constexpr (std::is_same<std::remove_cv_t<std::remove_reference_t<T>>, std::string>::value) {
    return std::forward<T>(t).c_str();
  } else {
    return std::forward<T>(t);
  }
}

/**
 * printf like formatting for C++ with std::string
 * Original source: https://stackoverflow.com/a/26221725/11722
 */
template<typename ... Args>
std::string stringFormatInternal(const std::string& format, Args&& ... args) {
  size_t size = snprintf(nullptr, 0, format.c_str(), std::forward<Args>(args) ...) + 1;
  if( size <= 0 ) {
    throw std::runtime_error( "Error during formatting." );
  }
  std::unique_ptr<char[]> buf(new char[size]);
  snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1);
}

template<typename ... Args>
std::string stringFormat(std::string fmt, Args&& ... args) {
  return stringFormatInternal(fmt, convert(std::forward<Args>(args))...);
}

#include <memory>
#include <string>
#include <stdexcept>

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args ) {
  size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
  if( size <= 0 ) {
    throw std::runtime_error( "Error during formatting." );
  }
  std::unique_ptr<char[]> buf( new char[ size ] );
  snprintf( buf.get(), size, format.c_str(), args ... );
  return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

int main() {

  std::string s = "a";
  const std::string cs = "b";
  const char* cc = "c";

  int i = 1;
  const int i2 = 2;

  using namespace std::literals;
  std::cout << stringFormat("%s %s %s %s %s %d %d %d \n", s, cs, cc, "d", "e"s, i, i2, 3);

  std::cout << string_format("%s %s %s %s %s %d %d %d \n", s, cs, cc, "d", "e"s, i, i2, 3);
  return 0;
}


#if 0

#include <iostream>
#include <format>

int main() {
  std::cout << std::format("Hello {}!\n", "world");
}

#include <stdarg.h>  // For va_start, etc.

std::string string_format(const std::string fmt, ...) {
  int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
  std::string str;
  va_list ap;
  while (1) {     // Maximum two passes on a POSIX system...
    str.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) {  // Everything worked
      str.resize(n);
      return str;
    }
    if (n > -1)  // Needed size returned
      size = n + 1;   // For null char
    else
      size *= 2;      // Guess at a larger size (OS specific)
  }
  return str;
}

A safer and more efficient (I tested it, and it is faster) approach:

#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr

std::string string_format(const std::string fmt_str, ...) {
  int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
  std::unique_ptr<char[]> formatted;
  va_list ap;
  while(1) {
    formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
    strcpy(&formatted[0], fmt_str.c_str());
    va_start(ap, fmt_str);
    final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
    va_end(ap);
    if (final_n < 0 || final_n >= n)
      n += abs(final_n - n + 1);
    else
      break;
  }
  return std::string(formatted.get());
}
#endif
