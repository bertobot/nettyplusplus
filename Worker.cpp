#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler, TimeoutStrategy ts) : thread() {
    mShutdownflag = false;

	mHandler = handler;

    mWorkerId = -1;

    mTimeoutStrategy = ts;

    // TODO: make tunable?
    mSelect.setTimeout(60, 0);

    mClientEmptyCV = new conditionVariable(&mLock);
}
/////////////////////////////////////////////////
Worker::~Worker() {
    stop();

    delete mClientEmptyCV;
    mClientEmptyCV = NULL;
}
/////////////////////////////////////////////////
void Worker::run() {
    
    while (! mShutdownflag) {

        mLock.lock();

        while (mSelect.empty() ) {
            mClientEmptyCV->wait();
            if (mShutdownflag) return;
        }

        if (mHandler) {
            std::vector<Socket> ready = mSelect.canRead();

            if (ready.empty() && mTimeoutStrategy == DISCONNECT) {
                mSelect.clear();
                continue;
            }

            for (unsigned int i = 0; i < ready.size(); i++) {
                try {
                    if (! ready[i].isConnected() ) {
                        mSelect.remove(ready[i]);
                        continue;
                    }

                    mHandler->onMessageReceived(ready[i]);

                    if (mHandler->shutdownOnExit(ready[i]) )
                        mShutdownflag = true;

                }
                catch(NIOException e) {
                    mSelect.remove(ready[i]);

                    ready[i].close();

                    // TODO: log?
                    std::cout << "Safety exception caught in Netty/Worker." << std::endl;
                }
            }
        }

        mLock.unlock();
    }
}
/////////////////////////////////////////////////
void Worker::stop() {
    mShutdownflag = true;
    mClientEmptyCV->signal();
}
/////////////////////////////////////////////////
void Worker::setWorkerId(int id) {
	mWorkerId = id;
}
/////////////////////////////////////////////////
int Worker::getWorkerId() const {
	return mWorkerId;
}
/////////////////////////////////////////////////

void Worker::addClient(Channel &client)
{
    mSelect.add(client);

    mHandler->onStart(client);

    mClientEmptyCV->signal();
}

bool Worker::shutdownCalled() {
    return (mShutdownflag == true);
}

// vim: ts=4:sw=4:expandtab
