#ifndef __Worker_h_
#define __Worker_h_

#include "ChannelHandler.h"
#include "Exception.h"
#include <MyThread/thread.h>
#include <MySocket/Select.h>
#include <MySocket/ServerSocket.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <errno.h>
#include <string.h>

class Worker : public thread {
protected:
    bool shutdownflag;

	ServerSocket *server;
	Socket *client;
	int workerId;
	ChannelHandler *m_handler;

public:
	Worker(ServerSocket *server, ChannelHandler *handler);

    void stop();
    int close();

    void run();

    void printLocal(const std::string&);
    void respond(const std::string&);
    void printLocalAndRespond(const std::string&);

	void setWorkerId(int id);
	int getWorkerId() const;
    
    virtual ~Worker();
};

#endif
