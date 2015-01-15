#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler, BlockingQueue<std::pair<Socket*, std::string> > *readyQueue, TimeoutStrategy ts) : thread() {
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

        std::pair<Socket*, std::string> p(mReadyQueue->pop());

        if (mHandler) {
            try {
                mHandler->onMessageReceived(*p.first, p.second);

                if (mHandler->shutdownOnExit(*p.first) )
                    mShutdownflag = true;
            }
            catch(NIOException e) {

                // TODO: log?
                std::cout << "Safety exception caught in Netty/Worker." << std::endl;
            }
        }
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
