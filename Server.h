#ifndef __Server_h_
#define __Server_h_

#include "ChannelHandler.h"
#include "Worker.h"
#include "Exception.h"

#include <MySocket/ServerSocket.h>
#include <MySocket/SelectSocket.h>
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

    void stop();


	ChannelHandler * getChannelHandler();

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
