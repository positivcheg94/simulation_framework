#pragma once

#include <vector>
#include <list>
#include <functional>
#include <random>

#include <smpp/priority_queue.hpp>
#include <smpp/task_completition.hpp>

namespace smpp
{
	template<class Task, class task_comparator, class mips_comparator, bool shuffle>
	auto simulate(std::vector<double> mips, std::vector<Task>&& tasks)
	{
		std::vector<Task> tasks_to_process = std::move(tasks);

		if constexpr(shuffle)
		{
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(tasks_to_process.begin(), tasks_to_process.end(), g);
		}
		

		// sort tasks
		std::sort(tasks_to_process.begin(), tasks_to_process.end(), task_comparator());
		// sort processors
		std::sort(mips.begin(), mips.end(), mips_comparator());

		priority_queue<task_completition<Task>, typename task_completition<Task>::later_first> p_queue;
		std::list<task_completition<Task>> processed_tasks;

		auto task_iterator = tasks_to_process.begin();
		for (size_t i = 0; i < mips.size() && task_iterator != tasks_to_process.end(); ++i)
		{
			auto time_to_process = task_iterator->complexity / mips[i];
			p_queue.emplace(time_to_process, i, std::move(*task_iterator));
			++task_iterator;
		}

		while (!p_queue.empty())
		{
			auto tk = p_queue.pop();
			if (task_iterator != tasks_to_process.end())
			{
				auto time_to_process = task_iterator->complexity / mips[tk.worker_index];
				p_queue.emplace(tk.expected_completition_time + time_to_process, tk.worker_index, std::move(*task_iterator));
				++task_iterator;
			}
			processed_tasks.push_back(std::move(tk));
		}

		return processed_tasks;
	}
}