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
			const auto partial = problem_size % slice_size == 0;
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

		inline double calculate_complexity(const size_t m, const size_t n, const size_t k, double multiplication_to_addition = 1.0)
		{
			return m * k*(n*multiplication_to_addition + std::log(n));
		}

		inline auto calculate_complexities(const size_t problem_size, const size_t slice_size)
		{
			const auto n_full = problem_size / slice_size;
			const auto partial = problem_size - n_full * slice_size;

			auto p1 = calculate_complexity(slice_size, slice_size, slice_size);
			if (partial == 0)
			{
				return std::make_tuple(n_full, p1, 0.0, 0.0);
			}
			auto p2 = calculate_complexity(partial, slice_size, slice_size);
			auto p3 = calculate_complexity(partial, slice_size, partial);

			return std::make_tuple(n_full, p1, p2, p3);
		}

		/*
		 *
		 * Task should have static create method (double complexity, size_t userid)
		 */
		template<typename Task>
		std::vector<Task> create_tasks(const size_t problem_size, const std::list<size_t>& slice_sizes)
		{
			const auto n_tasks = calculate_number_of_tasks(problem_size, slice_sizes);
			std::vector<Task> tasks;
			tasks.reserve(n_tasks);
			size_t user_id = 1;
			for (auto& slice_size : slice_sizes)
			{
				size_t n;
				double p1, p2, p3;
				std::tie(n, p1, p2, p3) = calculate_complexities(problem_size, slice_size);
				tasks.insert(tasks.end(), n*n, Task::create(p1, user_id));
				if (p2 != 0.0)
				{
					tasks.insert(tasks.end(), 2 * n, Task::create(p2, user_id));
					tasks.insert(tasks.end(), 1, Task::create(p3, user_id));
				}
				++user_id;
			}

			return tasks;
		}
	}
}