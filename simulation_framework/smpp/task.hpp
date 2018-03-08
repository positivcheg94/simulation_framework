#pragma once

#include <functional>

namespace smpp
{
    struct SimpleTask
    {
        typedef uint8_t userid_type;

        typedef std::function<bool(const SimpleTask&, const SimpleTask&)> comparator;

        static SimpleTask create(const double complexity, const size_t n_numbers, const userid_type userid)
        {
            return SimpleTask(complexity, 8*sizeof(double)*n_numbers, userid);
        }

        explicit SimpleTask(const double complexity, const size_t bits_to_transfer, const userid_type userid)
            : complexity(complexity), bits_to_transfer(bits_to_transfer), userid(userid)
        {

        }

        static comparator small_first()
        {
            return [](const SimpleTask& l, const SimpleTask& r) -> bool { return  l.complexity < r.complexity; };
        }

        static comparator large_first()
        {
            return [](const SimpleTask& l, const SimpleTask& r) -> bool { return  l.complexity > r.complexity; };
        }

        SimpleTask(const SimpleTask&) = default;
        SimpleTask(SimpleTask&&) = default;
        SimpleTask& operator=(SimpleTask&&) = default;
        SimpleTask& operator=(const SimpleTask&) = default;

        friend std::ostream& operator<< (std::ostream& stream, const SimpleTask& task)
        {
            stream << task.complexity << ',' << task.bits_to_transfer << ',' << int(task.userid);
            return stream;
        }

        double		complexity;
        size_t		bits_to_transfer	= 0;
        userid_type	userid				= 0;
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