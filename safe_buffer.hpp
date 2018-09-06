#ifndef _SAFE_BUFFER_HPP_
#define _SAFE_BUFFER_HPP_

#include <streambuf>
#include <iosfwd>
#include <cstdlib>
#include <cctype>
#include <ostream>
#include <functional>
#include <cassert>

class safe_buffer : public std::streambuf
{
public:
  explicit safe_buffer(std::ostream &sink, char* buff, std::size_t buff_sz = 256) :
    cap_next_(true),
    sink_(sink),
    buffer_(buff)
  {
    sink_.clear();
    char *base = buffer_;
    setp(base, base + buff_sz - 1);
  }

protected:
  bool flush()
  {
    std::ptrdiff_t n = pptr() - pbase();
    pbump(-n); // Move put pointer to start, current location is pptr()

    return sink_.write(pbase(), n);
  }

private:
  int_type overflow(int_type ch) {
    if (sink_ && ch != traits_type::eof()) {
      assert(std::less_equal<char *>()(pptr(), epptr()));
      *pptr() = char(ch);
      pbump(1);
      if (flush())
	return ch;
    }

    return traits_type::eof();
  }

  int sync() {
    return flush() ? 0 : -1;
  }

  // copy ctor and assignment not implemented;
  // copying not allowed
  safe_buffer(const safe_buffer &);
  safe_buffer &operator= (const safe_buffer &);

private:
  bool cap_next_;
  std::ostream &sink_;
  char* buffer_;
};

#endif /* _SAFE_BUFFER_HPP_ */
