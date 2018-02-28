#pragma once

#include <functional>

#include <smpp/priority_queue.hpp>
#include <smpp/task_completition.hpp>
#include <valarray>


namespace smpp
{
	struct TaskProcessor
	{
		template<typename ProcessorContainer, typename TaskContainer>
		auto operator()(const ProcessorContainer& procs, TaskContainer& tasks) const
		{
			typedef typename TaskContainer::value_type task_type;

			priority_queue<task_completition<task_type>, typename task_completition<task_type>::later_first> p_queue;
			std::list<task_completition<task_type>> processed_tasks;

			auto task_iterator = tasks.begin();
			for (size_t i = 0; i < procs.size() && task_iterator != tasks.end(); ++i)
			{
				auto time_to_process = procs[i].time_to_complete(task_iterator->complexity);
				p_queue.emplace(time_to_process, i, std::move(*task_iterator));
				++task_iterator;
			}

			while (!p_queue.empty())
			{
				auto tk = p_queue.pop();
				if (task_iterator != tasks.end())
				{
					auto time_to_process = procs[tk.worker_index].time_to_complete(task_iterator->complexity);
					p_queue.emplace(tk.expected_completition_time + time_to_process, tk.worker_index, std::move(*task_iterator));
					++task_iterator;
				}
				processed_tasks.push_back(std::move(tk));
			}

			return processed_tasks;
		}
		
	};

	struct TaskProcessorWithTicker
	{
		explicit TaskProcessorWithTicker(const double tick_period)
			: tick_period(tick_period)
		{
		}

		template<typename ProcessorContainer, typename TaskContainer>
		auto operator()(const ProcessorContainer& procs, TaskContainer& tasks) const
		{
			typedef typename TaskContainer::value_type task_type;

			priority_queue<task_completition<task_type>, typename task_completition<task_type>::later_first> p_queue;
			std::list<task_completition<task_type>> processed_tasks;

			std::vector<size_t> free_procs(procs.size());

			double simulation_internal_time = 0.0;
			auto task_iterator = tasks.begin();
			for (size_t i = 0; i < procs.size() && task_iterator != tasks.end(); ++i)
			{
				auto time_to_process = procs[i].time_to_complete(task_iterator->complexity);
				p_queue.emplace(time_to_process, i, std::move(*task_iterator));
				++task_iterator;
			}

			while (!p_queue.empty())
			{
				// clear vector or free idle processors after tick
				free_procs.clear();
				// pop first task as processed
				auto first_tk = p_queue.pop();
				// add poped task worker index to idle processors indexes
				free_procs.push_back(first_tk.worker_index);
				// calculate next simulation time
				double next_simulation_time = std::max(simulation_internal_time + tick_period, first_tk.expected_completition_time);
				// add move popped task to processed
				processed_tasks.push_back(std::move(first_tk));
				while (!p_queue.empty())
				{
					if (p_queue.top().expected_completition_time > next_simulation_time)
						break;
					auto tk = p_queue.pop();
					free_procs.push_back(tk.worker_index);
					processed_tasks.push_back(std::move(tk));
				}
				simulation_internal_time = next_simulation_time;

				for (auto idle_processor_idx : free_procs)
				{
					if (task_iterator != tasks.end())
					{
						auto time_to_process = procs[idle_processor_idx].time_to_complete(task_iterator->complexity);
						p_queue.emplace(simulation_internal_time + time_to_process, idle_processor_idx, std::move(*task_iterator));
						++task_iterator;
					}
				}
				
			}

			return processed_tasks;
		}

		double tick_period;
	};
}
