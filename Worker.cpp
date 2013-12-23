#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ChannelHandler *handler) : thread() {
    mShutdownflag = false;

	mHandler = handler;

    mWorkerId = -1;

    mSelect.setTimeout(3, 0);

    mClientEmptyCV.assocMutex(&mLock);
}
/////////////////////////////////////////////////
Worker::~Worker() {
    for (unsigned int i = 0; i < mClients.size(); i++) {
        mClients[i]->close();
        delete mClients[i];
        mClients[i] = NULL;
    }
}
/////////////////////////////////////////////////
void Worker::run() {
    
    while (! mShutdownflag) {

        mLock.lock();

        while (mSelect.empty() )
            mClientEmptyCV.wait();

        if (mHandler) {
            std::vector<Socket> ready = mSelect.canRead();

            for (unsigned int i = 0; i < ready.size(); i++) {
                try {
                    if (ready[i].isValid()) {
                        mHandler->onMessageReceived(ready[i]);

                        if (mHandler->shutdownOnExit(ready[i]) )
                            mShutdownflag = true;

                        if (! ready[i].isConnected() ) {
                            mSelect.remove(ready[i]);
                        }
                    }

                    else {
                        ready[i].close();
                        mSelect.remove(ready[i]);
                    }
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
}
/////////////////////////////////////////////////
/*
void Worker::printLocal(const std::string & str) {
    time_t seconds = time(NULL);
    std::cerr << seconds << " [threadserver] " << str << std::endl;
}
/////////////////////////////////////////////////
void Worker::printLocalAndRespond(const std::string & str) {
    printLocal(str);
    respond(str);
}
/////////////////////////////////////////////////
void Worker::respond(const std::string &str) {
    if (! mClient) {
        // TODO: should this be an exception?
        
        printLocal("respond: client is null!\n");
        return;
    }
    
    if (mClient->isValid() ) {
        // TODO: make this tunable
        Select s;

        s.setTimeout(10, 0);
        s.add(mClient->getSocketDescriptor() );

        try {
            if (! s.canWrite().empty() )
                mClient->write(str + "\r\n", MSG_NOSIGNAL);
            else
                printLocal("error: couldn't write response.\n");
        }
        catch (...) {
            printLocal("fatal: couldn't write!\n");
        }
    }
    else {
        printLocal("warning: was not able to send '" + str + "' because socket wasn't ready for writing.");
    }
}
*/
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
    //mClients.push_back(client);

    mSelect.add(client);

    mHandler->onStart(client);

    mClientEmptyCV.signal();
}

bool Worker::shutdownCalled() {
    return (mShutdownflag == true);
}

// vim: ts=4:sw=4:expandtab
