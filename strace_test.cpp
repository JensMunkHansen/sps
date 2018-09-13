#include <sps/strace.hpp>

int testMe(int a, float* q) {
  //  Debug::DeathHandler dh;
  int* p = NULL;
  *p = 0;
  return 0;
}

using namespace sps;

int main() {
  STrace::Instance().Enable();

  STrace::Instance().OptionSet(STrace::STRACE_GEN_CORE_DUMP, 1);
  STrace::Instance().OptionSet(STrace::STRACE_COMMON_PATH, 1);
  STrace::Instance().OptionSet(STrace::STRACE_REL_PATH, 0);

#if 0
  FILE* fp = ::fopen("/home/jmh/test.txt", "w+");
  int fd = fileno(fp);
  STrace::Instance().OptionSet(STrace::STRACE_FILEDESCRIPTOR, fd);
#endif

  int k = 2433;

  testMe(k, NULL);

  return 0;
}
