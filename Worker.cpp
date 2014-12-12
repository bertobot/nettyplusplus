#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler, BlockingQueue<Socket*> *readyQueue, TimeoutStrategy ts) : thread() {
    mShutdownflag = false;

	mHandler = handler;

    mReadyQueue = readyQueue;

    mWorkerId = -1;

    mTimeoutStrategy = ts;
}
/////////////////////////////////////////////////
Worker::~Worker() {
    stop();

    mReadyQueue = NULL;
}
/////////////////////////////////////////////////
void Worker::run() {
    
    while (! mShutdownflag) {

        Socket *ready = mReadyQueue->pop();

        if (mHandler) {
            try {
                mHandler->onMessageReceived(*ready);

                if (mHandler->shutdownOnExit(*ready) )
                    mShutdownflag = true;
            }
            catch(NIOException e) {

                // TODO: log?
                std::cout << "Safety exception caught in Netty/Worker." << std::endl;
            }
        }

        // NOTE: we don't delete/cleanup ready socket as that is handled by the SelectSocket object in Server.
    }
}
/////////////////////////////////////////////////
void Worker::stop() {
    mShutdownflag = true;
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
bool Worker::shutdownCalled() {
    return (mShutdownflag == true);
}

// vim: ts=4:sw=4:expandtab
