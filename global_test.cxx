#include <atomic>
#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>

// On window, we need a different way to ensure dtors are called when unloading a DLL
#ifdef _MSC_VER
#define SPS_ATTR_DESTRUCTOR
// TODO: On microsoft one needs to handle load/unload of DLL's
#else
#define SPS_ATTR_DESTRUCTOR __attribute__((destructor(101)))
#endif

class SingletonRegistry
{
public:
  // Register a destructor function
  static void RegisterDestructor(const std::function<void()>& func)
  {
    std::lock_guard<std::mutex> lock(GetMutex());
    GetDestructors().push_back(func);
  }

  // Destroy all registered singletons
  static void DestroyAllInstances()
  {
    std::lock_guard<std::mutex> lock(GetMutex());
    for (auto& destructor : GetDestructors())
    {
      destructor();
    }
    GetDestructors().clear();
  }

private:
  static std::list<std::function<void()>>& GetDestructors()
  {
    static std::list<std::function<void()>> destructor_list;
    return destructor_list;
  }

  static std::mutex& GetMutex()
  {
    static std::mutex destructor_mutex;
    return destructor_mutex;
  }
};

template <class T>
class Singleton
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
      std::lock_guard<std::recursive_mutex> guard(g_mutex);
      // Relaxed order is okay due to lock
      pInstance = g_instance.load(std::memory_order_relaxed);
      if (!pInstance)
      {
        // Ensure Singleton<T>::InstanceDestroy() is instantiated
        Singleton<T>::InstanceDestroy();

        pInstance = new T;
        g_instance.store(pInstance, std::memory_order_release);

        // Add the destructor function to the global registry
        SingletonRegistry::RegisterDestructor([]() { InstanceDestroy(); });
      }
    }
    return pInstance;
  }

  static int InstanceDestroy() SPS_ATTR_DESTRUCTOR
  {
    T* pInstance = g_instance.exchange(nullptr, std::memory_order_acq_rel);
    if (pInstance)
    {
      delete pInstance;
      g_instance.store(nullptr, std::memory_order_release); // Ensure pointer is nullified
    }
    return 0;
  }

protected:
  Singleton() = default;
  ~Singleton() = default;

  // Disallow copying and assignment
  Singleton(Singleton const&) = delete;
  Singleton& operator=(Singleton const&) = delete;

private:
  static std::atomic<T*> g_instance;
  static std::recursive_mutex g_mutex;
};

// Definitions of static members
template <class T>
std::atomic<T*> Singleton<T>::g_instance{ nullptr };

template <class T>
std::recursive_mutex Singleton<T>::g_mutex;

// Any singleton is now a Phoenix singleton
class Test : public Singleton<Test>
{
public:
private:
  float m_float;
  float* m_heapFloat;
  Test()
    : m_heapFloat(nullptr)
  {
    m_float = 2.0f;
    m_heapFloat = new float;
  }
  ~Test()
  {
    std::cout << "Destroyed\n";
    delete m_heapFloat;
    m_heapFloat = nullptr;
  }
  Test& operator=(const Test& rhs) = delete;

  friend class Singleton<Test>;
};

template <class T>
class TTest : public Singleton<TTest<T>>
{
public:
  T m_value;
  TTest() { m_value = T(1.0); }

private:
  friend class Singleton<TTest<T>>;
};

int main()
{
  Test* test = Test::InstanceGet();
  TTest<float>* ttest = TTest<float>::InstanceGet();

  SingletonRegistry::DestroyAllInstances();
  std::cout << "Before exit\n";
  return 0;
}
