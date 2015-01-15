#ifndef __Worker_h_
#define __Worker_h_

#include "BlockingQueue.h"
#include "ChannelHandler.h"
#include "Exception.h"

#include <MySocket/NIOException.h>
#include <MyThread/thread.h>
#include <MyThread/conditionVariable.h>

#include <iostream>
#include <sstream>
#include <vector>
#include <errno.h>
#include <string.h>

enum TimeoutStrategy {
    KEEP,
    DISCONNECT
};

class Worker : public thread {
public:
	Worker(ChannelHandler *handler, BlockingQueue<std::pair<Socket*, std::string> > *readyQueue, TimeoutStrategy ts=KEEP);

    virtual ~Worker();

    void stop();
    
    void run();

    void setWorkerId(int id);

	int getWorkerId() const;

    void addClient(Channel &client);

    bool shutdownCalled();

private:
    bool mShutdownflag;

    int mWorkerId;
    
    ChannelHandler *mHandler;

    BlockingQueue<std::pair<Socket*, std::string> > *mReadyQueue;

    TimeoutStrategy mTimeoutStrategy;
};

#endif

// vim: ts=4:sw=4:expandtab
