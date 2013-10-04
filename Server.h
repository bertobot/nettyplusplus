#ifndef __Server_h_
#define __Server_h_

#include "ChannelHandler.h"
#include "Worker.h"
#include <MySocket/ServerSocket.h>
#include <MyThread/thread.h>
#include <vector>

/**

Server(int port, int workers, ChannelHandler *handler);

**/

class Server : public thread {

public:
	Server(int port, int workers, ChannelHandler *handler = NULL);
	virtual ~Server();

	void run();

	void close();

	ChannelHandler * getChannelHandler();

private:
	ServerSocket *m_server;
    std::vector<Worker*> m_workers;

	int m_port;
	int m_numWorkers;
	int m_backlog;
	ChannelHandler *m_handler;

};

#endif

