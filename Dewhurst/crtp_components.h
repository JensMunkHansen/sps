//
// Copyright (c) 2017 by Daniel Saks and Stephen C. Dewhurst.
//
// Permission to use, copy, modify, distribute, and sell this
// software for any purpose is hereby granted without fee, provided
// that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice
// appear in supporting documentation.
// The authors make no representation about the suitability of this
// software for any purpose. It is provided "as is" without express
// or implied warranty.
//
#ifndef INCLUDED_CRTP_COMPONENTS
#define INCLUDED_CRTP_COMPONENTS

namespace DandS {
//
// Some templates for CRTP...
//

// The traditional object counter CRTP template
//
template <typename T>
class Ctr {
public:
	Ctr() { ++ctr_; }
	Ctr(Ctr const &) { ++ctr_; }
	~Ctr() { --ctr_; }
	static size_t get() { return ctr_; }
private:
	static size_t ctr_;
};

template <typename T>
size_t Ctr<T>::ctr_ = 0;

// The same, but with a separate template argument for counter type
//
template <typename T, typename I = size_t>
class ObjectCounter {
public:
	ObjectCounter() { ++ctr_; }
	ObjectCounter(ObjectCounter const &) { ++ctr_; }
	~ObjectCounter() { --ctr_; }
	static I get() { return ctr_; }
private:
	static I ctr_;
};

template <typename T, typename I>
I ObjectCounter<T, I>::ctr_ = 0;

// Equality comparison for Barton/Nackman trick
template <typename T>
struct Eq {
	friend bool operator ==(T const &a, T const &b) { return a.compare(b) == 0; }
	friend bool operator !=(T const &a, T const &b) { return a.compare(b) != 0; }
};

// Relational operators for same
template <typename T>
struct Rel {
	friend bool operator <(T const &a, T const &b) { return a.compare(b) < 0; }
	friend bool operator <=(T const &a, T const &b) { return a.compare(b) <= 0; }
	friend bool operator >(T const &a, T const &b) { return a.compare(b) > 0; }
	friend bool operator >=(T const &a, T const &b) { return a.compare(b) >= 0; }
};

}

#endif
