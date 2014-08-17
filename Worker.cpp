#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler, TimeoutStrategy ts) : thread() {
    mShutdownflag = false;

	mHandler = handler;

    mWorkerId = -1;

    mTimeoutStrategy = ts;

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

        std::vector<Socket> ready;

        if (mHandler) {
            try {
                ready = mSelect.canRead();

                if (ready.empty() ) std::cout << "[Worker::run] canRead empty." << std::endl;

                if (ready.empty() && mTimeoutStrategy == DISCONNECT) {
                    std::cout << "nope!" << std::endl;
                    mSelect.clear();
                    mLock.unlock();
                    continue;
                }
            }

            catch(NIOException e) {
                std::cout << "[Worker::run] canRead failed" << std::endl;
                
                mLock.unlock();

                continue;
            }

            for (unsigned int i = 0; i < ready.size(); i++) {
                try {
                    mHandler->onMessageReceived(ready[i]);

                    // clean up if we closed in the method above
                    if (! ready[i].isConnected() )
                        mSelect.remove(ready[i]);

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
