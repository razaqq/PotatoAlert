// Copyright 2021 <github.com/razaqq>

#include "ThreadPool.hpp"

#include <cassert>
#include <future>
#include <mutex>
#include <queue>
#include <thread>


using PotatoAlert::Core::ThreadPool;

ThreadPool::ThreadPool(size_t threadCount) : m_inFlight(0)
{
	std::unique_lock<std::mutex> lock(m_queueMutex);

	m_threads.reserve(threadCount);

	for (size_t i = 0; i <= threadCount; i++)
		StartWorker(i, lock);
}

ThreadPool::~ThreadPool()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_isStopping = true;
	m_conditionConsumers.notify_all();
	m_conditionProducers.notify_all();
	m_conditionConsumers.wait(lock, [this]() { return this->m_threads.empty(); });
	assert(m_inFlight == 0);
}

void ThreadPool::WaitUntilEmpty()
{
	std::unique_lock<std::mutex> lock(m_queueMutex);
	m_conditionProducers.wait(lock, [this]() { return m_tasks.empty(); });
}

void ThreadPool::WaitUntilNothingInFlight()
{
	std::unique_lock<std::mutex> lock(m_inFlightMutex);
	m_inFlightCondition.wait(lock, [this]() { return m_inFlight == 0; });
}

void ThreadPool::StartWorker(size_t id, const std::unique_lock<std::mutex>& lock)
{
	assert(lock.owns_lock() && lock.mutex() == &this->m_queueMutex);
	assert(id <= this->m_threads.size());

	auto worker_func = [this, id]()
	{
		while (true)
		{
			std::function<void()> task;
			bool notify;

			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_conditionConsumers.wait(lock, [this, id]()
				{
					return m_isStopping || !m_tasks.empty();
				});

				// deal with downsizing of thread pool or shutdown
				if (m_isStopping && m_tasks.empty())
				{
					// detach this worker, effectively marking it stopped
					m_threads[id].detach();
					// downsize the workers vector as much as possible
					while (!m_threads.empty() && !m_threads.back().joinable())
						m_threads.pop_back();
					// if this is was last worker, notify the destructor
					if (m_threads.empty())
						m_conditionConsumers.notify_all();
					return;
				}

				if (!m_tasks.empty())
				{
					task = std::move(m_tasks.front());
					m_tasks.pop();
					notify = m_tasks.empty();
				}
				else
				{
					continue;
				}
			}

			if (notify)
			{
				std::unique_lock<std::mutex> lock(m_queueMutex);
				m_conditionProducers.notify_all();
			}

			task();
		}
	};

	if (id < m_threads.size())
	{
		std::thread& worker = m_threads[id];
		// start only if not already running
		if (!worker.joinable())
		{
			worker = std::thread(worker_func);
		}
	}
	else
	{
		m_threads.emplace_back(std::thread(worker_func));
	}
}
