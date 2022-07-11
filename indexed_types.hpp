/**
 * @file   indexed_types.h
 * @author Jens Munk Hansen <jens.munk.hansen@gmail.com>
 * @date   Thu June 02 18:14:44 2009
 *
 * @brief  Generic indexed types. It is not thread safe, so in a
 *         threaded environment it is adviced to extend this class
 *         with a custom deleter, which is thread safe
 *
 *
 */

/*
 *  This file is part of SOFUS.
 *
 *  SOFUS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SOFUS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SOFUS.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// C headers
#include <sps/cstdio>
#include <sps/cstdlib>
#include <cstdint>
#include <cstring>

// C++ headers
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <sps/typeinfo>

namespace sps {

/**
 * Indexed types by means of custom auto-pointer and garbage
 * collector. The allocation takes place outside this manager to
 * allow the possibility for using a shared memory segment.
 *
 * Usage example:
 *
 * class A {
 * public:
 *   A() {
 *     data = shared_ptr<int>(new int(2));
 *   }                                   }
 * #ifdef _INDEXED_TYPES_CLONEABLE
 *   A* clone() const {
 *     A* out = new A(*this);
 *     out->data = shared_ptr<int>(new int(*this->data.get()));
 *     return out;
 *   }
 * #endif
 * private:
 *   shared_ptr<int> data;
 * };
 *
 * A* pA = new A();
 *
 * // Remove ownership
 * indexed_type_index i = create_handle<A>(pA); // pA is now NULL
 *
 * // Get object
 * pA = &(get_object<A>(i));
 *
 * // Use object through pointer
 * *pA->data = 5;
 *
 * // Clone object - note A may contain shared_ptr's
 * indexed_type_index j = clone_object<A>(i);
 *
 * // Destroy objects
 * destroy_object<A>(i);
 * destroy_object<A>(j);
 */

/// Cross-platform index
typedef uint64_t indexed_type_index;

/// Forward declaration of indexed_auto_ptr_collector
template<typename T> class indexed_auto_ptr_collector;

/**
 * Indexed auto pointer, which take ownership and delete the object
 * when it is destroyed. Be sure not to delete any aliases to the
 * object argument used in the ctor.
 *
 */
template <typename T>
class indexed_auto_ptr {
 public:
  /**
   * Ctor, note the ownership cannot be taken instantly
   *
   * @param ptr
   */
#ifdef SPS_TYPE_ID_NAME
  explicit indexed_auto_ptr(T*& ptr) : type(typeid(T).name()), t(ptr) {
#else
  explicit indexed_auto_ptr(T*& ptr) : type(&typeid(T)), t(ptr) {
#endif
    signature = this;
    indexed_auto_ptr_collector<T>::register_handle(this);
    ptr = 0;
  }

  /**
   * Dtor
   *
   */
  ~indexed_auto_ptr() {
    delete t;        // destroy object
    signature = NULL;  // destroy signature
  }

  /**
   * Convert index to an indexed_auto_ptr<T>.
   *
   * @param index
   *
   * @return
   *
   * @throw std::runtime_error
   */
  static indexed_auto_ptr* from_index(const indexed_type_index index );

  /**
   * Convert indexed_auto_ptr<T> to an index.
   *
   *
   * @return
   */
  indexed_type_index to_index();

  /**
   * Get the actual object contained by handle
   *
   *
   * @return
   */
  T& get_object() const {
    return *t;
  }

 private:
  /// used as a unique object signature
  indexed_auto_ptr* signature;
  /// type checkig information
  base_id_t type;
  /// object pointer
  T *t;

  /// Collector
  friend class indexed_auto_ptr_collector<T>;
};

namespace types {
/**
   \defgroup IndexedAutoPtr helper functions
   @{
*/

/**
 * Create integer handle for object
 *
 * @param t The object
 *
 * @return
 */
template <typename T> indexed_type_index create_handle(T* t) {
  indexed_auto_ptr<T>* handle = new indexed_auto_ptr<T>(t);
  return handle->to_index();
}

/**
 * Obtain object represented by handle.
 *
 * @param index
 *
 * @throw std::runtime_error
 *
 * @return
 */
template <typename T> T& get_object(const indexed_type_index index) {
  indexed_auto_ptr<T>* handle = indexed_auto_ptr<T>::from_index(index);
  return handle->get_object();
}

/**
 * Destroy object. If deleting object, rather than leaving it to
 * garbage collection, you must delete it via the handle; do not
 * delete T* directly.
 *
 * @param index
 *
 * @throw std::runtime_error
 */
template <typename T> void destroy_object(const indexed_type_index index) {
  indexed_auto_ptr<T>* handle = indexed_auto_ptr<T>::from_index(index);
  // If no exception is thrown using from_index at index, it is okay to
  // reset the pointer
  indexed_auto_ptr_collector<T>::objvector[index] = nullptr;
  delete handle;
}

#ifdef _INDEXED_TYPES_CLONEABLE
/**
 * Clone object using index - not possible since T may contain
 * shared_ptr and C++ does not support reflection (without using
 * macros and templates). The solution is to require T to implement:
 *
 * T* T::clone() const;
 *
 * @param index
 *
 * @return
 *
 * @throw std::runtime_error
 */
template <typename T>
indexed_type_index clone_object(const indexed_type_index index) {
  indexed_auto_ptr<T>* curHandle = indexed_auto_ptr<T>::from_index(index);
  T* clone = curHandle->get_object().clone();
  indexed_auto_ptr<T>* newHandle = new indexed_auto_ptr<T>(clone);
  return newHandle->to_index();
}
#endif
//@}
}  // namespace types

/**
 * Name:
 *   indexed_auto_ptr_collector
 *
 * Description:
 * - Indexed collector singleton (one collector object for each type
 T). Ensures that registered handles are deleted when the library
 is released (they may also be deleted before that without
 problems).
 *
 */
template <typename T>
class indexed_auto_ptr_collector {
 public:
  static std::vector<indexed_auto_ptr<T>*> objvector;

  /**
   * Destructor
   *
   */
  ~indexed_auto_ptr_collector() {
    size_t nObjectsCleared = 0;

    typename std::vector<indexed_auto_ptr<T>*>::iterator i;
    typename std::vector<indexed_auto_ptr<T>*>::iterator end = objvector.end();
    for (i = objvector.begin(); i != end ; ++i) {
      if (*i) {
        // check for valid signature
        if ((*i)->signature == *i) {
          delete *i;
          nObjectsCleared++;
        }
      }
    }
#ifdef DEBUG
    if (nObjectsCleared) {
      fprintf(stdout, "Garbage collector: ");
      fprintf(stdout, "Cleared %zu %s item(s)\n",
              nObjectsCleared, this->objname);
    }
#endif
  }

  /**
   * Register handle
   *
   * @param obj
   */
  static void register_handle(indexed_auto_ptr<T>* obj) {
    // Singleton - one for each type
    static indexed_auto_ptr_collector singleton(obj);

    typename std::vector<indexed_auto_ptr<T>*>::iterator i;
    typename std::vector<indexed_auto_ptr<T>*>::iterator end = objvector.end();

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
  explicit indexed_auto_ptr_collector(indexed_auto_ptr<T>* obj) {
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
  indexed_auto_ptr_collector(const indexed_auto_ptr_collector&);
};

/// Explicit instantiation
template <typename T>
std::vector<indexed_auto_ptr<T>*> indexed_auto_ptr_collector<T>::objvector;

template <typename T>
indexed_auto_ptr<T>*
indexed_auto_ptr<T>::from_index(const indexed_type_index index) {
  // Find object in static vector
  indexed_auto_ptr* obj = NULL;

  if (index < indexed_auto_ptr_collector<T>::objvector.size()) {
    if (indexed_auto_ptr_collector<T>::objvector[index] != nullptr) {
      obj = indexed_auto_ptr_collector<T>::objvector[index];
    }
  }

  if (!obj) {
    // Check to see we don"t have an invalid pointer
    fprintf(stderr,
            "Parameter is NULL. It does not represent an "      \
            "indexed_auto_ptr object.\n");
    throw std::runtime_error("indexed_auto_ptr<T>::from_index");
  }

  if ( !( ((uintptr_t) obj) < UINTPTR_MAX) ) {
    fprintf(stderr, "Pointer value of of range");
    throw std::runtime_error("indexed_auto_ptr<T>::from_index");
  }
  // Check memory has correct signature
  if (obj->signature != obj) {
    fprintf(stderr,
            "Parameter does not represent an indexed_auto_ptr object.\n");
    throw std::runtime_error("indexed_auto_ptr<T>::from_index");
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
            "Given indexed_auto_ptr does not represent the correct type.\n");
    throw std::runtime_error("indexed_auto_ptr<T>::from_index");
#ifdef SPS_TYPE_ID_NAME
  }
#else
  }
#endif
  return obj;
}

template <typename T>
indexed_type_index indexed_auto_ptr<T>::to_index() {
  // Find pointer in vector
  auto it = find(indexed_auto_ptr_collector<T>::objvector.begin(),
                 indexed_auto_ptr_collector<T>::objvector.end(), this);
  if (it != indexed_auto_ptr_collector<T>::objvector.end()) {
    return indexed_type_index(
             std::distance(
               indexed_auto_ptr_collector<T>::objvector.begin(),
               it));
  } else {
    fprintf(stderr, "Invalid index.\n");
    throw std::runtime_error("indexed_auto_ptr<T>::to_index");
  }
}
}  // namespace sps
