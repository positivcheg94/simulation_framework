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
		typename TaskProcessor,
		bool shuffle = false>
	auto simulate(std::vector<Processor> procs, std::vector<Task>&& tasks, const TaskProcessor& tprocessor)
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

		return tprocessor(procs, tasks_to_process);
	}
}