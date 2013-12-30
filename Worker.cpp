#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler) : thread() {
    mShutdownflag = false;

	mHandler = handler;

    mWorkerId = -1;

    // TODO: make tunable?
    mSelect.setTimeout(1, 0);

    mClientEmptyCV.assocMutex(&mLock);
}
/////////////////////////////////////////////////
Worker::~Worker() {
    stop();
}
/////////////////////////////////////////////////
void Worker::run() {
    
    while (! mShutdownflag) {

        mLock.lock();

        while (mSelect.empty() ) {
            mClientEmptyCV.wait();
            if (mShutdownflag) return;
        }

        if (mHandler) {
            std::vector<Socket> ready = mSelect.canRead();

            for (unsigned int i = 0; i < ready.size(); i++) {
                try {
                    mHandler->onMessageReceived(ready[i]);

                    if (mHandler->shutdownOnExit(ready[i]) )
                        mShutdownflag = true;

                    if (! ready[i].isConnected() )
                        mSelect.remove(ready[i]);
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
    mClientEmptyCV.signal();
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

void Worker::addClient(Socket &client)
{
    mSelect.add(client);

    mHandler->onStart(client);

    mClientEmptyCV.signal();
}

bool Worker::shutdownCalled() {
    return (mShutdownflag == true);
}

// vim: ts=4:sw=4:expandtab
