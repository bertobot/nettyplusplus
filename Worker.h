#ifndef __Worker_h_
#define __Worker_h_

#include "ChannelHandler.h"
#include "Exception.h"
#include <MyThread/thread.h>
#include <MySocket/SelectSocket.h>
#include <MySocket/ServerSocket.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <errno.h>
#include <string.h>

class Worker : public thread {
public:
	Worker(ChannelHandler *handler);

    virtual ~Worker();

    void stop();
    
    int close();

    void run();

	/*
    void printLocal(const std::string&);
    void respond(const std::string&);
    void printLocalAndRespond(const std::string&);
	*/

	void setWorkerId(int id);
	int getWorkerId() const;

    void addClient(Socket &client);

private:
    bool mShutdownflag;

    int mWorkerId;
    
    ChannelHandler *mHandler;

    std::vector<Socket *> mClients;

    SelectSocket mSelect;
};

#endif
