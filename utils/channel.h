#ifndef CHANNEL_H
#define CHANNEL_H

#include <cassert>
#include <queue>

#include <mutex>
#include <condition_variable>
#include <atomic>

template<class T>
class Channel
{
public:
	enum FullPushBehavior { BLOCK, DROP_NEWEST, DROP_OLDEST, NEVER_FULL };
	Channel(void);
	Channel(FullPushBehavior fullPushBehavior, size_t maxSize = 1);
	Channel(const Channel &other) = delete;

	//closing the front of the channel will cause all pushes into the channel to return false, permenently
	//use it when a consumer wants to inform producers that nothing else will be consumed from the channel
	bool isFrontClosed(void) const;
	void closeFront(void);

	//closing the back of the channel will cause all pops from the channel to return false, permenently
	//use it when a producer wants to inform consumers that nothing else will be produced into the channel
	bool isBackClosed(void) const;
	void closeBack(void);

	//put an item in the channel. returns false if the front of the channel was closed.
	bool push(const T& item);

	//get an item from the channel and place it in 'result'. if the channel is empty, block until something is added or until the back is closed
	//returns false when because the channel is empty and the back is closed
	bool pop(T& result);

private:
	const FullPushBehavior fullPushBehavior;
	const size_t maxSize;
	std::atomic<bool> _isFrontClosed;
	std::atomic<bool> _isBackClosed;

	std::queue<T> queue;
	std::mutex queueMutex;
	std::condition_variable fullWait;
	std::condition_variable emptyWait;
};

class ChannelClosedException : public std::exception
{

};

template<class T>
Channel<T>::Channel(void)
	:fullPushBehavior(NEVER_FULL), maxSize(0), _isFrontClosed(false), _isBackClosed(false)
{

}

template<class T>
Channel<T>::Channel(FullPushBehavior fullPushBehavior, size_t maxSize)
	:fullPushBehavior(fullPushBehavior), maxSize(maxSize), _isFrontClosed(false), _isBackClosed(false)
{
	if (fullPushBehavior != NEVER_FULL)
		assert(maxSize > 0);
}

template<class T>
bool Channel<T>::isFrontClosed(void) const
{
	return _isFrontClosed.load(std::memory_order_acquire);
}

template<class T>
void Channel<T>::closeFront(void)
{
	std::unique_lock<std::mutex> locker(queueMutex);

	_isFrontClosed.store(true, std::memory_order_release);

	//if anyone is waiting for the "full" condition variable, wake them up so that they know they should stop pushing data
	fullWait.notify_all();
}

template<class T>
bool Channel<T>::isBackClosed(void) const
{
	return _isBackClosed.load(std::memory_order_acquire);
}

template<class T>
void Channel<T>::closeBack(void)
{
	std::unique_lock<std::mutex> locker(queueMutex);

	_isBackClosed.store(true, std::memory_order_release);

	//if anyone is waiting for the "empty" condition variable, wake them up so that they know they're not going to get any more data
	emptyWait.notify_all();
}

template<class T>
bool Channel<T>::push(const T& item)
{
	if (isBackClosed())
	{
		throw ChannelClosedException();
	}


	std::unique_lock<std::mutex> locker(queueMutex);

	if (isFrontClosed())
	{
		return false;
	}

	auto canAddItem = [this]() { return queue.size() < maxSize; };

	if (fullPushBehavior == NEVER_FULL)
	{
		queue.push(item);

		//wake up anyone who might be waiting
		emptyWait.notify_one();
	}
	else if (fullPushBehavior == BLOCK)
	{
		//block until the queue isn't full anymore
		if (!canAddItem())
		{
			fullWait.wait(locker, [this]() { return isFrontClosed() || queue.size() < maxSize; });
		}

		if (isFrontClosed())
		{
			return false;
		}
		queue.push(item);

		//wake up anyone who might be waiting
		emptyWait.notify_one();
	}
	else if (fullPushBehavior == DROP_NEWEST)
	{
		//if the queue is full, we're just going to drop the given item
		if (canAddItem())
		{
			queue.push(item);

			//wake up anyone who might be waiting
			emptyWait.notify_one();
		}
	}
	else if (fullPushBehavior == DROP_OLDEST)
	{
		//if the queue is full, pop off the front of the channel
		if (!canAddItem())
		{
			queue.pop();
		}
		queue.push(item);

		//wake up anyone who might be waiting
		emptyWait.notify_one();
	}

	return false;
}

template<class T>
bool Channel<T>::pop(T& result)
{
	std::unique_lock<std::mutex> locker(queueMutex);

	//if the queue is closed and empty, return false to iindicate that the caller will not recieve a response
	if (isBackClosed() && queue.empty())
		return false;

	//if the queue is empty, block!
	if (queue.empty())
	{
		emptyWait.wait(locker, [this]() { return isBackClosed() || !queue.empty(); });

		//if the queue is still empty, it means it's closed and will never get another item,so return false
		if (queue.empty())
			return false;
	}

	result = queue.front();
	queue.pop();

	//notify any pushers who may be waiting on a full queue that it is no longer empty
	fullWait.notify_one();
	return true;
}

#endif // CHANNEL_H
