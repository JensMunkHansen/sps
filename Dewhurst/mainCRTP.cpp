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
#include "number.h"
#include "crtp_components.h"
#include "writeonly.h"
#include <iostream>
#include <stdexcept>
#include <string>

template<template <typename...> class... CRTPs>
class Thing : public CRTPs<Thing<CRTPs...>>... {
public:
	Thing(int a) : a_(a) {}
	int compare(Thing const &rhs) const {
		if (a_ == rhs.a_)
			return 0;
		if (a_ < rhs.a_)
			return -1;
		return 1;
	}
private:
	int a_;
};

void test_CRTP() {
	using namespace std;
	using namespace DandS;

	cout << "\n===  Testing CRTP  ===" << endl;

	using MyThing = Thing<Ctr, ObjectCounter, Eq, Rel>;
	MyThing a(1);
	cout << "count: " << MyThing::Ctr<MyThing>::get() << endl;
	cout << "another count: " << MyThing::ObjectCounter<MyThing>::get() << endl;
	MyThing b(2);
	if (a != b) {
		MyThing c (3), d (4);
		cout << "count: " << MyThing::Ctr<MyThing>::get() << endl;
		cout << "another count: " << MyThing::ObjectCounter<MyThing>::get() << endl;
	}
	cout << "count: " << MyThing::Ctr<MyThing>::get() << endl;
	cout << "another count: " << MyThing::ObjectCounter<MyThing>::get() << endl;

	cout << "a == b: " << boolalpha << (a == b) << endl;
	cout << "a != b: " << boolalpha << (a != b) << endl;
	cout << "a < b: " << boolalpha << (a < b) << endl;
	cout << "a <= b: " << boolalpha << (a <= b) << endl;
	cout << "a >= b: " << boolalpha << (a >= b) << endl;
	cout << "a > b: " << boolalpha << (a > b) << endl;
}

void test_restricted_numeric() {
	using namespace std;
	using namespace DandS;

	cout << "\n===  Testing restricted numeric  ===" << endl;

	{
		cout << "======  int  ======" << endl;

		using restricted_int = Number<int, Eq_i, Add_i, Shift_i, Stream_i>;
		restricted_int i1(10), i2(3);
		constexpr restricted_int i3(10);
		cout << "i1: " << i1 << endl;
		cout << "i2: " << i2 << endl;
		cout << "i1 == i2: " << (i1 == i2) << endl;
		cout << "i1 == i3: " << (i1 == i3) << endl;
		cout << "i1 + i2: " << (i1 + i2) << endl;
		cout << "i1 + i2 + i3: " << (i1 + i2 + i3) << endl;
		cout << "i1 << i2: " << (i1 << i2) << endl;
		cout << "i1 = i2: " << (i1 = i2) << endl;
		cout << "i1 = 123: " << (i1 = 123) << endl;
	}
	{
		cout << "======  double  ======" << endl;
		using restricted_float = Number<double, Eq_i, Add_i, Stream_i>;
		restricted_float i1(12.3), i2(10.1);
		cout << "i1: " << i1 << endl;
		cout << "i2: " << i2 << endl;
		cout << "i1 == i2: " << (i1 == i2) << endl;
		cout << "i1 + i2: " << (i1 + i2) << endl;
		cout << "i1 = i2: " << (i1 = i2) << endl;
	}
	{
		cout << "======  minimal int  ======" << endl;

		using minimal_int = Number<int, Stream_i>;
		minimal_int i1(10), i2(3);
		constexpr minimal_int i3(10);
		cout << "i1: " << i1 << endl;
		cout << "i2: " << i2 << endl;
		cout << "i3: " << i3 << endl;
	}
}

void test_restricted_register() {
	using namespace std;
	using namespace DandS;

	cout << "\n===  Testing restricted register  ===" << endl;
	{
		// Make a hardware register type.
		using hw_register = unsigned volatile;
		using rw_register = Number<hw_register, Shift_i, Bit_i, Eq_i, Stream_i>;

		rw_register r1(0xFE00), r2(0x0001);
		cout << "r1: " << r1 << endl;
		cout << "r2: " << r2 << endl;
		cout << "r1 == r2: " << (r1 == r2) << endl;
		cout << "r1 | r2: " << (r1 | r2) << endl;
		cout << "r1 & r2 | r1: " << (r1 & r2 | r1) << endl;
		cout << "r1 << r2: " << (r1 << r2) << endl;
		cout << "r1 = r2: " << (r1 = r2) << endl;

		// Make restricted register types.
		using wo_register = write_only<rw_register>;
		using ro_register = read_only<rw_register>;
		using un_register = unused<rw_register>;

		// Make a further restricted register type.
		using status_register = Number<hw_register, Bit_i>;
		using ro_status_register = read_only<status_register>;

		// Make a memory-mapped serial port.
		class UART {
		public:
			UART() : UBRDIV(1200) { check(); }
			// etc...
		private:
			un_register ULCON;         // line control
			un_register UCON;          // control
			ro_status_register USTAT;  // status
			wo_register UTXBUF;        // transmit
			ro_register URXBUF;        // receive
			rw_register UBRDIV;        // baud rate divisor
			static constexpr void check() {
				// Obsessively check everything...
				static_assert(offsetof(UART, ULCON) == 0, "bad offset of ULCON in UART");
				static_assert(offsetof(UART, UCON) == 4, "bad offset of UCON in UART");
				static_assert(offsetof(UART, USTAT) == 8, "bad offset of USTAT in UART");
				static_assert(offsetof(UART, UTXBUF) == 12, "bad offset of UTXBUF in UART");
				static_assert(offsetof(UART, URXBUF) == 16, "bad offset of URXBUF in UART");
				static_assert(offsetof(UART, UBRDIV) == 20, "bad offset of UBRDIV in UART");
				static_assert(sizeof(UART) == 24, "bad size for UART");
			}
		};

		// Map it to an address.
		char pretend_uart_mapping[sizeof(UART)];
		UART *uart0 = new (static_cast<void *>(pretend_uart_mapping)) UART;
		uart0->~UART();
	}
}

int main() {
	try {
		test_CRTP();
		test_restricted_numeric();
		test_restricted_register();
	}
	catch (const std::exception &e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
	catch (...) {
		std::cout << "Unknown exception" << std::endl;
	}

	return 0;
}
