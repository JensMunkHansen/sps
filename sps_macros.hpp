#pragma once

#include <sps/sps_compiler.h>

template <typename T>
inline T* sGetPtrHelper(T* ptr)
{
  return ptr;
}
template <typename Ptr>
inline auto sGetPtrHelper(Ptr& ptr) -> decltype(ptr.operator->())
{
  return ptr.operator->();
}

// The body must be a statement:
#define SPS_CAST_IGNORE_ALIGN(body)                                                                \
  SPS_WARNING_PUSH SPS_WARNING_DISABLE_GCC("-Wcast-align") body SPS_WARNING_POP

#define SPS_DECLARE_PRIVATE(Class)                                                                 \
  inline Class##Private* d_func()                                                                  \
  {                                                                                                \
    SPS_CAST_IGNORE_ALIGN(return reinterpret_cast<Class##Private*>(sGetPtrHelper(d_ptr));)         \
  }                                                                                                \
  inline const Class##Private* d_func() const                                                      \
  {                                                                                                \
    SPS_CAST_IGNORE_ALIGN(return reinterpret_cast<const Class##Private*>(sGetPtrHelper(d_ptr));)   \
  }                                                                                                \
  friend class Class##Private;

#define SPS_DECLARE_PRIVATE_D(Dptr, Class)                                                         \
  inline Class##Private* d_func()                                                                  \
  {                                                                                                \
    SPS_CAST_IGNORE_ALIGN(return reinterpret_cast<Class##Private*>(sGetPtrHelper(Dptr));)          \
  }                                                                                                \
  inline const Class##Private* d_func() const                                                      \
  {                                                                                                \
    SPS_CAST_IGNORE_ALIGN(return reinterpret_cast<const Class##Private*>(sGetPtrHelper(Dptr));)    \
  }                                                                                                \
  friend class Class##Private;

#define SPS_DECLARE_PUBLIC(Class)                                                                  \
  inline Class* q_func()                                                                           \
  {                                                                                                \
    return static_cast<Class*>(q_ptr);                                                             \
  }                                                                                                \
  inline const Class* q_func() const                                                               \
  {                                                                                                \
    return static_cast<const Class*>(q_ptr);                                                       \
  }                                                                                                \
  friend class Class;

#define SPS_D(Class) Class##Private* const d = d_func()
#define SPS_Q(Class) Class* const q = q_func()

/*
  Some classes do not permit copies to be made of an object. These
  classes contains a private copy constructor and assignment
  operator to disable copying (the compiler gives an error message).
*/
#define SPS_DISABLE_COPY(Class)                                                                    \
  Class(const Class&) = delete;                                                                    \
  Class& operator=(const Class&) = delete;

#define SPS_DISABLE_MOVE(Class)                                                                    \
  Class(Class&&) = delete;                                                                         \
  Class& operator=(Class&&) = delete;

#define SPS_DISABLE_COPY_MOVE(Class)                                                               \
  Q_DISABLE_COPY(Class)                                                                            \
  Q_DISABLE_MOVE(Class)

// consider renaming d_ptr to self

// arguments by value to interface can be forwarded to impl using std::move

// avoid forwarding all arguments by overloading operator->() and use pointer semantics for
// forwarding..

// pimpl good for non-polymorphic classes. ABI stable when adding non-member function, adding
// non-virtual
