#ifndef __Server_h_
#define __Server_h_

#include "ChannelHandler.h"
#include "Exception.h"
#include "SelectSocket.h"
#include "Worker.h"

#include <MySocket/ServerSocket.h>
#include <MyThread/thread.h>
#include <vector>

/**

Server(int port, int workers, ChannelHandler *handler);

**/

class Server : public thread {

public:
	Server(int port, int workers, ChannelHandler *handler = NULL, TimeoutStrategy ts = KEEP);
	virtual ~Server();

	void run();

    void stop();


	ChannelHandler * getChannelHandler();

    void setChannelHandler(ChannelHandler *handler);

private:
	ServerSocket *m_server;
    std::vector<Worker*> m_workers;

	int m_port;
	int m_numWorkers;
	int m_backlog;
	ChannelHandler *m_handler;

    bool m_done;

};

#endif
// vim: ts=4:sw=4:expandtab
