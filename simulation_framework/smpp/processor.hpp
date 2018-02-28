#pragma once

#include <functional>

namespace smpp
{
	struct Processor
	{
		typedef std::less<Processor>	slow_first;
		typedef std::greater<Processor>	fast_first;

		explicit Processor(const double mips)
			: mips(mips)
		{

		}

		Processor(const Processor&) = default;
		Processor(Processor&&) = default;
		Processor& operator=(Processor&&) = default;
		Processor& operator=(const Processor&) = default;
		~Processor() = default;

		double time_to_complete(const double complexity) const
		{
			return complexity / mips;
		}

		double mips;
	};
}

namespace std
{
	template<>
	struct less<smpp::Processor>
	{
		typedef smpp::Processor val;
		constexpr bool operator()(const val& l, const val& r) const
		{
			return l.mips < r.mips;
		}

	};

	template<>
	struct greater<smpp::Processor>
	{
		typedef smpp::Processor val;
		constexpr bool operator()(const val& l, const val& r) const
		{
			return l.mips > r.mips;
		}

	};
}