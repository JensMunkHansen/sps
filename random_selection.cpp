// -*- compile-command: "clang++ -ggdb -o random_selection -std=c++0x -stdlib=libc++ random_selection.cpp" -*-
//Reference implementation for doing random number selection from a container.
//Kept for posterity and because I made a surprising number of subtle mistakes on my first attempt.
#include <random>
#include <iterator>

template <typename RandomGenerator = std::default_random_engine>
struct random_selector
{
	//On most platforms, you probably want to use std::random_device("/dev/urandom")()
	random_selector(RandomGenerator g = RandomGenerator(std::random_device()()))
		: gen(g) {}

	template <typename Iter>
	Iter select(Iter start, Iter end) {
		std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
		std::advance(start, dis(gen));
		return start;
	}

	//convenience function
	template <typename Iter>
	Iter operator()(Iter start, Iter end) {
		return select(start, end);
	}

	//convenience function that works on anything with a sensible begin() and end(), and returns with a ref to the value type
	template <typename Container>
	auto operator()(const Container& c) -> decltype(*begin(c))& {
		return *select(begin(c), end(c));
	}

private:
	RandomGenerator gen;
};

//example use case with uniformity test
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <string>

typedef std::vector<std::string> vec_t;
typedef std::unordered_map<typename vec_t::value_type, unsigned> histogram_t;

void run_testcase(vec_t& source_container, const unsigned samples = 1000u) {
	histogram_t histogram;
	random_selector<> selector{};
	for (auto i = samples; i > 0; --i) {
		++(histogram[selector(source_container)]);
	}

        std::cout << "Selection histogram from " << samples << " samples:\n";
	//if only C++11 had a nicer hack overloading ostream& operator<<(ostream&,std::pair<>)
	for (auto i : histogram) { std::cout << i.first << ':' << ' ' << i.second << '\n'; }
}

//just some silly preamble
int main(int argc, const char* argv[]) {
	vec_t source_container;
	if (argc > 1) {
		source_container.assign(argv+1, argv+argc);
	} else {
		source_container = {"2", "43", "443"};
	}

	run_testcase(source_container);
	return 0;
}
