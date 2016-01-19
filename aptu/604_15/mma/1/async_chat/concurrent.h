#pragma once

#include <queue>
#include <set>
#include <mutex>
#include <boost\thread.hpp>

namespace async_chat
{

//multiple producer, single consumer
template<typename T>
struct concurrent_queue
{
	void push(T const& item)
	{
		std::unique_lock<std::mutex> lock(m_);
		q_.push(item);
	}

	std::queue<T> move_to_queue()
	{
		std::unique_lock<std::mutex> lock(m_);
		std::queue<T> r = move(q_);
		return r;
	}

private:
	mutable std::mutex m_;
	std::queue<T> q_;
};


template<typename K>
struct concurrent_set
{
	using unique_lock = boost::unique_lock<boost::shared_mutex>;
	using shared_lock = boost::shared_lock<boost::shared_mutex>;

	void put(K const& key)
	{
		unique_lock lock(m_);
		set_.insert(key);
	}

	void erase(K const& key)
	{
		unique_lock lock(m_);
		set_.erase(key);
	}

	bool exist(K const& key) const
	{
		shared_lock lock(m_);
		return set_.count(key) != 0;
	}

	template<typename Functor>
	void foreach_unordered(Functor f) const
	{
		shared_lock lock(m_);
		//std::vector<K> items(set_.begin(), set_.end());
		//std::random_shuffle(items.begin(), items.end());
		//std::_For_each(items.begin(), items.end(), f);
		std::_For_each(set_.begin(), set_.end(), f);
	}

private:
	std::set<K> set_;
	mutable boost::shared_mutex m_;
};

} //async_chat