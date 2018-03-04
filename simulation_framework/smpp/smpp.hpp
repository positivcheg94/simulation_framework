#pragma once

#include <vector>
#include <valarray>
#include <list>
#include <functional>
#include <random>
#include <set>

#include <smpp/processor.hpp>
#include <smpp/task.hpp>
#include <smpp/priority_queue.hpp>
#include <smpp/task_completition.hpp>
#include <smpp/task_processor.hpp>

namespace smpp
{
	std::valarray<double> simulate(
		std::vector<Processor> procs, Processor::comparator proc_comp,
		std::vector<SimpleTask>&& tasks, SimpleTask::comparator task_comp,
		const TaskProcessor& tprocessor,
		const SimpleTask::userid_type n_user_hint,
		const bool shuffle = true
	)
	{
		std::vector<SimpleTask> tasks_to_process = std::move(tasks);

		if (shuffle)
		{
			std::random_device rd;
			std::mt19937 g(rd());
			std::shuffle(tasks_to_process.begin(), tasks_to_process.end(), g);
		}
		// sort tasks
		std::sort(tasks_to_process.begin(), tasks_to_process.end(), task_comp);
		// sort processors
		std::sort(procs.begin(), procs.end(), proc_comp);

		auto processed_tasks = tprocessor(procs, tasks_to_process);

		std::set<SimpleTask::userid_type> idx;
		std::valarray<double> times(n_user_hint);

		//size_t previous_idx = std::numeric_limits<size_t>::max();
		for (auto curr = processed_tasks.crbegin(); curr != processed_tasks.crend(); ++curr)
		{
			auto curr_idx = curr->task.userid;
			if (idx.insert(curr_idx).second)
			{
				times[curr_idx] = curr->expected_completition_time;
				if (idx.size() == n_user_hint)
					break;
			}
		}

		return times;
	}
}