#pragma once

#define SPS_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return reinterpret_cast<Class##Private *>(d_ptr); } \
    inline const Class##Private* d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr); } \
    friend class Class##Private;

#define SPS_D(Class) Class##Private * const d = d_func()

// consider renaming d_ptr to self

// arguments by value to interface can be forwarded to impl using std::move

// avoid forwarding all arguments by overloading operator->() and use pointer semantics for forwarding..
