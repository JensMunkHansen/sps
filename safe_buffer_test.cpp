// Copyright 2007 Edd Dawson.
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file BOOST_LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

#include <sps/str_utils.hpp>
#include <sps/safe_buffer.hpp>

#include <stdint.h>
#include <malloc.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include <vector>

void* (*__old_malloc_hook)(size_t, const void*);

void *__wrap_malloc(size_t c, const void*) {
  printf("malloc called with argument %zu\n", c);

  __malloc_hook = __old_malloc_hook;
  void* p = malloc(c);
  __malloc_hook = __wrap_malloc;
  return p;
}

int main() {

  __old_malloc_hook = __malloc_hook;
  __malloc_hook = __wrap_malloc;

  #define SZ 1024
  char buffer[SZ];

  float f = 4.56;
  snprintf(buffer, 15,"%f",f); // no malloc

  //  std::vector<int> v(145);

  safe_buffer buff(std::cout, &buffer[0], SZ);

  std::ostream out(&buff);
  char arr[1024];
  char* parr = arr;
  size_t nLeft = 1024;

  char* c1 = sps::set_color_s(&parr, nLeft, sps::colors::red, sps::colors::none);
  char* c2 = strcat(c1,sps::set_face_s(&parr,nLeft,sps::faces::uline));
  out << c2 << "This is red and underlined" << sps::set_color_s(&parr,nLeft,sps::colors::none) << std::endl;

  out << "0x" << std::setfill('0') << std::hex << std::setw(2*sizeof(uintptr_t)) << (uintptr_t)&f << std::endl;



  printf("bytes left: %lu\n", nLeft);
  /*
  using namespace sps;
  out << set_color(colors::red, colors::cyan) << set_face(faces::uline) << "This is red" << set_color(colors::none) << std::endl;
  */

  // Here it is flushed
  out << std::flush;

  // Causes flushing
  for (size_t i = 0; i < 256 ; i++) {
    out << " ";
  }

  __malloc_hook = __old_malloc_hook;


  return 0;
}
