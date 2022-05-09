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

#ifndef INCLUDED_STL_ITERATOR_TRAITS
#define INCLUDED_STL_ITERATOR_TRAITS

#include "pred_expr.h"
#include <iterator>
#include <type_traits>

// Making the stl iterator categories look like the facilities in <type_traits>.
// Also convenience functions to connect up to predicate expressions.

namespace DandS {
	inline namespace STL_ITERATOR_TRAITS_IMPL {
		template <bool c>
		using is_true = std::conditional_t<c, std::true_type, std::false_type>;
	}

	template <typename It>
	using Category
		= typename std::iterator_traits<It>::iterator_category;

	template <typename It>
	using is_exactly_rand
		= std::is_same<Category<It>, std::random_access_iterator_tag>;
	constexpr auto isExactlyRand = pred<is_exactly_rand>();

	template <typename It>
	using is_exactly_bi
		= std::is_same<Category<It>, std::bidirectional_iterator_tag>;
	constexpr auto isExactlyBi = pred<is_exactly_bi>();

	template <typename It>
	using is_exactly_for
		= std::is_same<Category<It>, std::forward_iterator_tag>;
	constexpr auto isExactlyFor = pred<is_exactly_for>();

	template <typename It>
	using is_exactly_in
		= std::is_same<Category<It>, std::input_iterator_tag>;
	constexpr auto isExactlyIn = pred<is_exactly_in>();

	template <typename It>
	using is_exactly_out
		= std::is_same<Category<It>, std::output_iterator_tag>;
	constexpr auto isExactlyOut = pred<is_exactly_out>();

	template <typename It>
	using is_rand = is_true<is_exactly_rand<It>::value>;
	constexpr auto isRand = pred<is_rand>();
	using IsRand = decltype(isRand);

	template <typename It>
	using is_bi = is_true<is_rand<It>::value || is_exactly_bi<It>::value>;
	constexpr auto isBi = pred<is_bi>();
	using IsBi = decltype(isBi);

	template <typename It>
	using is_for = is_true<is_exactly_for<It>::value || is_bi<It>::value>;
	constexpr auto isFor = pred<is_for>();
	using IsFor = decltype(isFor);

	template <typename It>
	using is_in = is_true<is_exactly_in<It>::value || is_for<It>::value>;
	constexpr auto isIn = pred<is_in>();
	using IsIn = decltype(isIn);

	template <typename It>
	using is_out = is_true<is_exactly_out<It>::value || is_for<It>::value>;
	constexpr auto isOut = pred<is_out>();
	using IsOut = decltype(isOut);
}

#endif
