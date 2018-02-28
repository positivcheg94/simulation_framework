#pragma once

#include <vector>
#include <list>
#include <functional>
#include <random>

#include <smpp/priority_queue.hpp>
#include <smpp/task_completition.hpp>

namespace smpp
{
	template<class Processor, class Task, class TaskComparator, class ProcessorComparator,
		
		bool shuffle = false>
	auto simulate(std::vector<Processor> procs, std::vector<Task>&& tasks)
	{
		std::vector<Task> tasks_to_process = std::move(tasks);

		if constexpr(shuffle)
		{
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(tasks_to_process.begin(), tasks_to_process.end(), g);
		}

		// sort tasks
		std::sort(tasks_to_process.begin(), tasks_to_process.end(), TaskComparator());
		// sort processors
		std::sort(procs.begin(), procs.end(), ProcessorComparator());

		priority_queue<task_completition<Task>, typename task_completition<Task>::later_first> p_queue;
		std::list<task_completition<Task>> processed_tasks;

		auto task_iterator = tasks_to_process.begin();
		for (size_t i = 0; i < procs.size() && task_iterator != tasks_to_process.end(); ++i)
		{
			auto time_to_process = procs[i].time_to_complete(task_iterator->complexity);
			p_queue.emplace(time_to_process, i, std::move(*task_iterator));
			++task_iterator;
		}

		while (!p_queue.empty())
		{
			auto tk = p_queue.pop();
			if (task_iterator != tasks_to_process.end())
			{
				auto time_to_process = procs[tk.worker_index].time_to_complete(task_iterator->complexity);
				p_queue.emplace(tk.expected_completition_time + time_to_process, tk.worker_index, std::move(*task_iterator));
				++task_iterator;
			}
			processed_tasks.push_back(std::move(tk));
		}

		return processed_tasks;
	}
}