#include <sps/typeinfo>

#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>

#include <cstring>

#include <sps/crtp_helper.hpp>

// TODO(JMH): Experiment with reference counted thread-safe objects

namespace sps {

template<typename T> class GarbageCollector;

/// Cross-platform index
typedef uint64_t indexed_type_index;

// Thread-safe sharing using atomics
template <typename T>
class Shared : public CRTP<T, Shared> {
 protected:
  Shared() {
    std::cout << "Shared" << std::endl;
  }
 public:
  ~Shared() {
    std::cout << "~Shared" << std::endl;
  }
};

template <typename T>
class GarbageCollected : public CRTP<T, GarbageCollected> {
 private:
  base_id_t type;                ///< Type information
  GarbageCollected* signature;   ///< Unique signature

 protected:
  GarbageCollected() :
#ifdef SPS_TYPE_ID_NAME
    type(typeid(T).name())
#else
    type(&typeid(T))
#endif
  {
    this->signature = this;
    GarbageCollector<T>::RegisterHandle(this);
    std::cout << "GarbageCollected" << std::endl;
  }

 public:
  ~GarbageCollected() {
    std::cout << "~GarbageCollected" << std::endl;
    // Zero-out signature
    signature = NULL;

    indexed_type_index index;
    this->ToIndex(&index);
    GarbageCollector<T>::objvector[index] = nullptr;
  }

  static indexed_type_index Create(T** obj) {
    *obj = new T();

    // TODO(JMH): Move the indexing to another parent
    indexed_type_index index;
    (*obj)->ToIndex(&index);
    return index;
  }

  static void Destroy(T* obj) {
    delete obj;
  }

  static GarbageCollected* FromIndex(const indexed_type_index index);
  //throw(std::runtime_error);

  int ToIndex(indexed_type_index* index);

  void test() {
    const T& t = this->underlying();
    SPS_UNREFERENCED_PARAMETER(t);
  }
  /// Collector
  friend class GarbageCollector<T>;
};

template <typename T> void Destroy(const indexed_type_index index)
//throw(std::runtime_error)
{
  GarbageCollected<T>* handle = GarbageCollected<T>::FromIndex(index);
  if (handle) {
    GarbageCollector<T>::objvector[index] = nullptr;
    delete handle->thisp();
    handle = nullptr;
  }
}

template <typename T>
class GarbageCollector {
 public:
  static std::vector<GarbageCollected<T>*> objvector;

  /**
   * Destructor
   *
   */
  ~GarbageCollector() {
    size_t nObjectsCleared = 0;

    typename std::vector<GarbageCollected<T>*>::iterator i;
    typename std::vector<GarbageCollected<T>*>::iterator end = objvector.end();
    for (i = objvector.begin(); i != end ; ++i) {
      if (*i) {
        // check for valid signature (invalid read of size 8 when deleted)
        if ((*i)->signature == *i) {
          delete *i;
          nObjectsCleared++;
        }
      }
    }
#ifdef SPS_DEBUG
    if (nObjectsCleared) {
      fprintf(stdout, "Garbage collector: ");
      fprintf(stdout, "Cleared %zu %s item(s)\n",
              nObjectsCleared, this->objname);
    }
#else
    (void)nObjectsCleared;
#endif
  }

  /**
   * Register handle
   *
   * @param obj
   */
  static void RegisterHandle(GarbageCollected<T>* obj) {
    // Singleton - one for each type
    static GarbageCollector singleton(obj);

    typename std::vector<GarbageCollected<T>*>::iterator i;
    typename std::vector<GarbageCollected<T>*>::iterator end = objvector.end();

    i = std::find(objvector.begin(), objvector.end(), nullptr);
    if (i != end) {
      (*i) = obj;
    } else {
      singleton.objvector.push_back(obj);
    }
  }

 private:
  /// Name
  char objname[256];

  /**
   * Private ctor (prevent construction)
   *
   * @param obj
   */
  explicit GarbageCollector(GarbageCollected<T>* obj) {
    char buffer[128];
    // buffer is always terminated
#ifdef SPS_TYPE_ID_NAME
    snprintf(buffer, sizeof(buffer), "%zu", strlen(obj->type));
#else
    snprintf(buffer, sizeof(buffer), "%zu", strlen(obj->type.name()));
#endif

#if defined(__GNUC__)
    // GNU use the length followed by the name
    strncpy(objname, obj->type + strlen(buffer), sizeof(objname));
#elif defined(_MSC_VER)
    // Microsoft have 6 extra characters in names
    strncpy(objname, obj->type + 6, sizeof(objname));
#endif
    // strncpy does not terminate if source is larger than destination
    objname[sizeof(objname)-1] = '\0';
  }

  /**
   * Private copy-ctor (prevent construction)
   *
   */
  GarbageCollector(const GarbageCollector&);
};

/// Explicit instantiation
template <typename T>
std::vector<GarbageCollected<T>*> GarbageCollector<T>::objvector;

template <typename T>
GarbageCollected<T>*
GarbageCollected<T>::FromIndex(const sps::indexed_type_index index)
//throw(std::runtime_error)
{
  // Find object in static vector
  GarbageCollected* obj = NULL;

  if (index < GarbageCollector<T>::objvector.size()) {
    if (GarbageCollector<T>::objvector[index] != nullptr) {
      obj = GarbageCollector<T>::objvector[index];
    }
  }

  if (!obj) {
    // Check to see we don"t have an invalid pointer
    fprintf(stderr,
            "Parameter is NULL. It does not represent an "      \
            "GarbageCollected object.\n");
    throw std::runtime_error("GarbageCollected<T>::FromIndex");
  }

  if ( !(reinterpret_cast<uintptr_t>(obj) < UINTPTR_MAX) ) {
    fprintf(stderr, "Pointer value of of range");
    throw std::runtime_error("GarbageCollected<T>::FromIndex");
  }
  // Check memory has correct signature
  if (obj->signature != obj) {
    fprintf(stderr,
            "Parameter does not represent an GarbageCollected object.\n");
    throw std::runtime_error("GarbageCollected<T>::FromIndex");
  }

#ifdef SPS_TYPE_ID_NAME
  if (strcmp(obj->type, typeid(T).name()) != 0) {
    fprintf(stderr, "Given: <%s>, Required: <%s>.\n", obj->type,
            typeid(T).name());
#else
  if (*(obj->type) != typeid(T)) {
    fprintf(stderr, "Given: <%s>, Required: <%s>.\n", obj->type->name(),
            typeid(T).name());
#endif
    fprintf(stderr,
            "Given GarbageCollected does not represent the correct type.\n");
    throw std::runtime_error("GarbageCollected<T>::FromIndex");
#ifdef SPS_TYPE_ID_NAME
  }
#else
  }
#endif
  return obj;
}

template <typename T>
int GarbageCollected<T>::ToIndex(indexed_type_index* index) {
  // Find pointer in vector
  auto it = find(GarbageCollector<T>::objvector.begin(),
                 GarbageCollector<T>::objvector.end(), this);
  if (it != GarbageCollector<T>::objvector.end()) {
    *index =
      indexed_type_index(
        std::distance(
          GarbageCollector<T>::objvector.begin(),
          it));
    return 0;
  } else {
    fprintf(stderr, "Invalid index.\n");
    return -1;
  }
}


}  // namespace sps


//  TODO(JEM): introduce indexed after garbage collected
//  If an object is typed (using typename) gc is using it
class GCTestObject :
  public sps::GarbageCollected<GCTestObject>,
  public sps::Shared<GCTestObject> {
 public:
  GCTestObject() {
    std::cout << "GCTestObject" << std::endl;
    // GarbageCollected called before shared
  }
  ~GCTestObject() {
  }
};
