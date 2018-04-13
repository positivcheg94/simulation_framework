#pragma once
#include<list>
#include<vector>
#include<tuple>


namespace smpp
{
    namespace mmsim
    {
        inline size_t calculate_number_of_tasks(const size_t problem_size, const size_t slice_size)
        {
            const auto n_full = problem_size / slice_size;
            const auto partial = (problem_size % slice_size) != 0;
            if (partial)
                return n_full * (n_full + 2) + 1;
            return n_full * n_full;
        }

        inline size_t calculate_number_of_tasks(const size_t problem_size, const std::list<size_t>& slice_sizes)
        {
            size_t accumulated_size = 0;
            for (auto& slice_size : slice_sizes)
                accumulated_size += calculate_number_of_tasks(problem_size, slice_size);
            return accumulated_size;
        }

        inline auto calculate_complexity(const size_t m, const size_t n, const size_t k, double multiplication_to_addition = 1.0)
        {
            return std::make_pair(m * k*(n*multiplication_to_addition + n - 1), m*n + n*k + m*k);
        }

        inline auto calculate_complexities(const size_t problem_size, const size_t slice_size)
        {
            typedef decltype(calculate_complexity(size_t(), size_t(), size_t(), double())) compl_t;
            const auto n_full = problem_size / slice_size;
            const auto partial = problem_size - n_full * slice_size;

            auto p1 = calculate_complexity(slice_size, problem_size, slice_size);
            if (partial == 0)
            {
                return std::make_tuple(n_full, p1, compl_t(0.0, 0), compl_t(0.0, 0));
            }
            auto p2 = calculate_complexity(partial, problem_size, slice_size);
            auto p3 = calculate_complexity(partial, problem_size, partial);

            return std::make_tuple(n_full, p1, p2, p3);
        }

        /*
         * Task should have static create method (double complexity, size_t n_numbers, size_t userid)
         */
        template<typename Task>
        std::vector<Task> create_tasks(const size_t problem_size, const std::list<size_t>& slice_sizes)
        {
            typedef decltype(calculate_complexity(size_t(), size_t(), size_t(), double())) compl_t;

            const auto n_tasks = calculate_number_of_tasks(problem_size, slice_sizes);
            std::vector<Task> tasks;
            tasks.reserve(n_tasks);
            typename Task::userid_type user_id = 0;
            for (auto& slice_size : slice_sizes)
            {
                size_t n;
                compl_t p1, p2, p3;
                std::tie(n, p1, p2, p3) = calculate_complexities(problem_size, slice_size);
                tasks.insert(tasks.end(), n*n, Task::create(p1.first, p1.second, user_id));
                if (p2.first != 0.0)
                {
                    tasks.insert(tasks.end(), 2 * n, Task::create(p2.first, p2.second, user_id));
                    tasks.insert(tasks.end(), 1, Task::create(p3.first, p3.second, user_id));
                }
                ++user_id;
            }

            return tasks;
        }
    }
}