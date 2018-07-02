#include <sps/sps_export.h>
#include <sps/if_threadpool.hpp>
#include <sps/threadpool.hpp>

// TODO(JMH): Explicitly instantiate SubmitJob

namespace sps {
class ThreadPoolImpl : public IThreadPool, public ThreadPool {
 public:
  explicit ThreadPoolImpl(const std::size_t nThreads) : ThreadPool(nThreads) {
  }

  ThreadPoolImpl()
      : ThreadPoolImpl{std::max<unsigned int>(std::thread::hardware_concurrency(), 2u) - 1u} {}

  int Initialize() override {
    // No need to initialize
    return 0;
  }

  int SubmitJob(SimpleCallback cb, void* pUserData) override {
    this->submit([](SimpleCallback _cb, void* _pUserData)->void {
        (*_cb)(_pUserData);}, cb, pUserData);
    return 0;
  }

  ~ThreadPoolImpl() override {
    // No additional cleanup is needed
  }
};

int ThreadPoolCreate(IThreadPool** ppObj) {
  *ppObj = (IThreadPool*) new ThreadPoolImpl(2);
  return 0;
}
int ThreadPoolDestroy(IThreadPool* pObj) {
  if (pObj) {
    delete pObj;
  }
  return 0;
}

}  // namespace sps
