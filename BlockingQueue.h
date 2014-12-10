#ifndef __BlockingQueue_h_
#define __BlockingQueue_h_

#include <MyThread/thread.h>
#include <MyThread/conditionVariable.h>

#include <deque>

template <class T>
class BlockingQueue {

public:
    BlockingQueue() {
		mSize = 0;
        mNotEmpty = new conditionVariable(&mLock);
    }

    virtual ~BlockingQueue() {
        if (mNotEmpty) {
            delete mNotEmpty;
            mNotEmpty = NULL;
        }
    }

    void push(const T &value) {

        mPushLock.lock();

		++mSize;

        mQueue.push_back(value);

        mNotEmpty->signal();

        mPushLock.unlock();
    }

	void headPush(const T &value) {
		mLock.lock();

		++mSize;

		mQueue.push_front(value);

		mNotEmpty->signal();

		mLock.unlock();
	}

    T pop() {
        mLock.lock();

        while (mQueue.empty() )
            mNotEmpty->wait();

        T result = mQueue.front();

        mQueue.pop_front();

		--mSize;

        mLock.unlock();

        return result;
    }

	long int size() const { return mSize; }

private:
    std::deque<T> mQueue;
    mutex mLock, mPushLock;
    conditionVariable *mNotEmpty;
	long int mSize;


};

#endif
// vim: set ts=4:sw=4:expandtab
