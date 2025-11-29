#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>
#include <mutex>

template <int A>
class aligned;
template <>
class __attribute__((aligned(16))) aligned<16>
{
  const int k = 4;
};

template <class T>
class Identifiable
{
private:
  static std::uint32_t UIDCreate()
  {
    static std::atomic<std::uint32_t> g_uid = { 0 };
    std::uint32_t retval = std::atomic_fetch_add(&g_uid, 1U);
    if (retval + 1 == 0)
    {
      exit(-1);
    }
    return retval;
  }

protected:
  Identifiable() { m_Id = UIDCreate(); }

public:
  std::uint32_t IDGet() const { return m_Id; }

private:
  std::uint32_t m_Id;
};

template <class T>
class Default
{
public:
  /**
   * InstanceGet
   *
   * Acquire the (non-static) singleton instance
   *
   * @return Pointer to singleton instance
   */
  static T* InstanceGet()
  {
    T* pInstance = g_instance.load(std::memory_order_acquire);
    if (!pInstance)
    {
      std::lock_guard<std::mutex> guard(g_mutex);
      pInstance = g_instance.load(std::memory_order_relaxed);
      if (!pInstance)
      {
        pInstance = new T;
        g_instance.store(pInstance, std::memory_order_release);
      }
    }
    return pInstance;
  }

  static int InstanceDestroy() __attribute__((destructor(101)));

  ~Default() { std::cout << "~Default()" << std::endl; }

protected:
  Default() { std::cout << "Default()" << std::endl; }

private:
  static std::atomic<T*> g_instance; // Default instance
  static std::mutex g_mutex;         // Mutex for accessing default instance
};

template <class T>
int Default<T>::InstanceDestroy()
{
  int retval = -1;
  std::lock_guard<std::mutex> guard(g_mutex);
  if (g_instance)
  {
    std::cout << "Default destroyed" << std::endl;
    delete g_instance;
    g_instance = nullptr;
    retval = 0;
  }
  return retval;
}

template <class T>
std::atomic<T*> Default<T>::g_instance{ nullptr };

template <class T>
std::mutex Default<T>::g_mutex;

template <class T>
class Singleton : public Default<T>
{
protected:
  Singleton() {}

  Singleton(Singleton const&) = delete;
  Singleton& operator=(Singleton const&) = delete;

protected:
  // Friends of friends does not work
};

class A
  : public aligned<16>
  , public Default<A>
  , public Identifiable<A>
{
public:
  A() { std::cout << "A()" << std::endl; }
  ~A() { std::cout << "~A()" << std::endl; }
};

template int Default<A>::InstanceDestroy();

class B : public Singleton<B>
{
private:
  B() { std::cout << "B()" << std::endl; }
  ~B() { std::cout << "~B()" << std::endl; }

private:
  int data;
  friend class Default<B>;
};

template int Default<B>::InstanceDestroy();

int main()
{
  A a;
  printf("0x%x\n", &a);
  printf("%d\n", (uintptr_t)&a % 16);

  std::cout << a.IDGet() << std::endl;
  A* pAGlobal = A::InstanceGet();
  std::cout << pAGlobal->IDGet() << std::endl;

  //  std::cout << pAGlobal->IDGet() << std::endl;

  B* pBGlobal = B::InstanceGet();

  std::cout << int(*reinterpret_cast<int*>(&a)) << std::endl;

  std::cout << "Hello" << std::endl;
  return 0;
}
