// Copyright 2021 <github.com/razaqq>
#pragma once

#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>


namespace PotatoAlert::Core {

class ThreadPool
{
public:
	ThreadPool(size_t threadCount = std::max(2u, std::thread::hardware_concurrency()));

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;
	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;

	template<typename Func, typename... Args>
	auto Enqueue(Func&& func, Args&&... args) -> std::future<std::invoke_result_t<Func&&, Args&&...>>
	{
		using Ret = std::invoke_result_t<Func&&, Args&&...>;

		auto task = std::make_shared<std::packaged_task<Ret()>>(
			std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

		std::future<Ret> res = task->get_future();

		std::unique_lock<std::mutex> lock(m_queueMutex);

		m_tasks.emplace([task](){ (*task)(); });
		std::atomic_fetch_add_explicit(&m_inFlight, 1, std::memory_order_relaxed);
		m_conditionConsumers.notify_one();

		return res;
	}

	void WaitUntilEmpty();
	void WaitUntilNothingInFlight();
	~ThreadPool();

private:
	void StartWorker(size_t id, const std::unique_lock<std::mutex>& lock);

	bool m_isStopping = false;
	
	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_tasks;

	std::mutex m_queueMutex;
	std::condition_variable m_conditionProducers;
	std::condition_variable m_conditionConsumers;

	std::mutex m_inFlightMutex;
	std::condition_variable m_inFlightCondition;
	std::atomic<size_t> m_inFlight;
};

}  // namespace PotatoAlert::Core
