#pragma once

#include <functional>

#include <smpp/processor.hpp>
#include <smpp/task.hpp>
#include <smpp/priority_queue.hpp>
#include <smpp/task_completition.hpp>



namespace smpp
{
	struct TaskProcessor
	{
		typedef SimpleTask task;
		typedef priority_queue<task_completition<task*>, typename task_completition<task*>::later_first> task_queue;
		

		virtual std::vector<task_completition<task*>> operator()(const std::vector<Processor>& procs, std::vector<task>& tasks) const = 0;
	};

	struct TaskProcessorWithTransfer : TaskProcessor
	{
		typedef TaskProcessor::task			task;
		typedef TaskProcessor::task_queue	task_queue;

		TaskProcessorWithTransfer(
			double bandwidth		= 1.25e+7, // ~100 mbit/s
			double connection_setup = 0.00001  // time to setup connection

		)
			: bandwidth(bandwidth), connection_setup(connection_setup)
		{
		}

		double transfer_time(size_t bytes_to_transfer) const
		{
			return bytes_to_transfer / bandwidth + connection_setup;
		}

		std::vector<task_completition<task*>> operator()(const std::vector<Processor>& procs, std::vector<task>& tasks) const override
		{
			task_queue p_queue;
			std::vector<task_completition<task*>> processed_tasks;
			processed_tasks.reserve(tasks.size());

			auto task_iterator = tasks.begin();
			for (size_t i = 0; i < procs.size() && task_iterator != tasks.end(); ++i)
			{
				const auto time_to_process = procs[i].time_to_complete(task_iterator->complexity);
				p_queue.emplace(time_to_process, i, &(*task_iterator));
				++task_iterator;
			}

			while (!p_queue.empty())
			{
				auto tk = p_queue.pop();
				if (task_iterator != tasks.end())
				{
					const auto time_to_process = 
						procs[tk.worker_index].time_to_complete(task_iterator->complexity)	//main processing time
						+
						transfer_time(task_iterator->bytes_to_transfer)						// time for data transfer
						;
					p_queue.emplace(tk.expected_completition_time + time_to_process, tk.worker_index, &(*task_iterator));
					++task_iterator;
				}
				processed_tasks.push_back(std::move(tk));
			}

			return processed_tasks;
		}

		double bandwidth;
		double connection_setup;
	};
}
