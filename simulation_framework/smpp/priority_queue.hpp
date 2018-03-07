#pragma once

#include <vector>
#include <algorithm>

namespace smpp
{
    template <typename Type, typename Compare = std::less<Type>>
    class priority_queue
    {
    public:
        typedef Type										value_type;
        typedef std::vector<Type>							container_type;
        typedef Compare										value_compare;
        typedef typename container_type::size_type			size_type;
        typedef typename container_type::reference			reference;
        typedef typename container_type::const_reference	const_reference;

        explicit priority_queue(const Compare& compare = Compare())
            : comparator{ compare }
        { }
        void push(value_type element)
        {
            container.push_back(std::move(element));
            std::push_heap(container.begin(), container.end(), comparator);
        }

        template<typename... Args>
        void emplace(Args... args)
        {
            container.push_back(value_type(std::forward<Args>(args)...));
            std::push_heap(container.begin(), container.end(), comparator);
        }

        value_type pop()
        {
            std::pop_heap(container.begin(), container.end(), comparator);
            value_type result = std::move(container.back());
            container.pop_back();
            return std::move(result);
        }

        bool empty() const
        {
            return container.empty();
        }

        const_reference top() const
        {
            return container.front();
        }

    private:
        container_type	container;
        value_compare	comparator;
    };
}