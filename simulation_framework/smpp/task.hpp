#pragma once

#include <functional>

namespace smpp
{
	struct SimpleTask
	{
		typedef std::less<SimpleTask>		small_first;
		typedef std::greater<SimpleTask>	large_first;

		static SimpleTask create(const double complexity, size_t userid)
		{
			return SimpleTask(complexity, userid);
		}

		explicit SimpleTask(const double complexity, const size_t userid)
			: complexity(complexity), userid(userid)
		{

		}

		SimpleTask(const SimpleTask&) = default;
		SimpleTask(SimpleTask&&) = default;
		SimpleTask& operator=(SimpleTask&&) = default;
		SimpleTask& operator=(const SimpleTask&) = default;

		double complexity;
		size_t userid = 0;
	};
}

namespace std
{
	template<>
	struct less<smpp::SimpleTask>
	{
		typedef smpp::SimpleTask val;
		constexpr bool operator()(const val& l, const val& r) const
		{
			return l.complexity < r.complexity;
		}

	};

	template<>
	struct greater<smpp::SimpleTask>
	{
		typedef smpp::SimpleTask val;
		constexpr bool operator()(const val& l, const val& r) const
		{
			return l.complexity > r.complexity;
		}

	};
}