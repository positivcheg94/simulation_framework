#pragma once

#include <functional>

namespace smpp
{
    struct Processor
    {
        typedef std::function<bool(const Processor&, const Processor&)> comparator;

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

        static comparator slow_first()
        {
            return [](const Processor& l, const Processor& r) -> bool { return  l.mips < r.mips; };
        }

        static comparator fast_first()
        {
            return [](const Processor& l, const Processor& r) -> bool { return  l.mips > r.mips; };
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