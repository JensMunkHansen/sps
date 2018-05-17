#include <sps/sgc2.hpp>

#if defined(__GNUC__) && !defined(__CYGWIN__)
#include <sps/strace.hpp>
void Init()     __attribute__((constructor(101)));

void Init() {
# if !defined(NDEBUG)
  sps::STrace::Instance().Enable();
# endif
}
#endif

// std::atomic<int>
// .exchange(curVal+1)
// If it is increased, we have a valid buffer


int main() {
  GCTestObject  obj;

  try {
    GCTestObject* pObj;
    uintptr_t index = GCTestObject::Create(&pObj);
    pObj->test();
    //GCTestObject::Destroy(pObj); // invalid read of size ptr_size
    //sps::Destroy<GCTestObject>(index);
    pObj = nullptr;
  } catch(...) {
  }
  return 0;
}
