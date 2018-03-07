#pragma once

namespace smpp
{
    template<typename T>
    T& get_ref(T& t)
    {
        return t;
    }

    template<typename T>
    T& get_ref(T* t)
    {
        return *t;
    }

    template<typename Task>
    struct task_completition
    {
        task_completition(
            const double time_start,
            const double time_end,
            const size_t worker_index,
            Task&& task
        )
            : time_start(time_start), time_end(time_end), worker_index(worker_index), task(std::move(task))
        {

        }

        task_completition(task_completition&&) = default;
        task_completition& operator=(task_completition&&) = default;


        friend std::ostream& operator<< (std::ostream& stream, const task_completition& tc)
        {
            stream << tc.time_start << ',' << tc.time_end << ',' << tc.worker_index << ',' << get_ref(tc.task);
            return stream;
        }


        struct later_first
        {
            constexpr bool operator()(const task_completition& l, const task_completition& r)
            {
                return l.time_end > r.time_end;
            }
        };



        // members
        double	time_start;
        double	time_end;
        size_t	worker_index;
        Task	task;
    };

    
}
