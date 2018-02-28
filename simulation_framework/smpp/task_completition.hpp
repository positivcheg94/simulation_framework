#pragma once

namespace smpp
{
	template<typename Task>
	struct task_completition
	{
		task_completition(
			const double expected_completition_time,
			const size_t worker_index,
			Task&& task
		)
			: expected_completition_time(expected_completition_time), worker_index(worker_index), task(std::move(task))
		{

		}

		task_completition(task_completition&&) = default;
		task_completition& operator=(task_completition&&) = default;


		struct later_first
		{
			constexpr bool operator()(const task_completition& l, const task_completition& r)
			{
				return l.expected_completition_time > r.expected_completition_time;
			}
		};



		// members
		double	expected_completition_time;
		size_t	worker_index;
		Task	task;
	};

	
}
