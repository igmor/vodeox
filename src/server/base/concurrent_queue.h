#ifndef __COUNCURRENT_QUEUE_H
#define __COUNCURRENT_QUEUE_H

#include <vector>
#include <queue>

#include "base/scoped_lock.h"

namespace vodeox {

/*
 * concurrent queue implementation
*/
template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data>			m_queue;
    mutable vodeox::mutex		m_mutex;
    vodeox::condition_variable	m_condition_variable;
	bool						m_shutdown;
public:
	
    void push(Data const& data)
    {
        scoped_lock lock(m_mutex);
        m_queue.push(data);
        lock.unlock();
        m_condition_variable.notify();
    }

    bool empty() const
    {
        scoped_lock lock(m_mutex);
        return m_queue.empty();
    }

	void wait_and_pop(std::vector<Data>& values)
    {
        scoped_lock lock(m_mutex);
        while(m_queue.empty())
        {
            m_condition_variable.wait(m_mutex);
        }
        
		while(!m_queue.empty())
		{
	        values.push_back(m_queue.front());
		    m_queue.pop();
		}
    }

	void pop(std::vector<Data>& values)
    {
        scoped_lock lock(m_mutex);
		while(!m_queue.empty())
		{
	        values.push_back(m_queue.front());
		    m_queue.pop();
		}
    }

	void shutdown()
	{
		//unblock waiting thread
        m_condition_variable.notify();
	}
};

} //namespace
#endif
