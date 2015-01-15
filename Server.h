#ifndef __Server_h_
#define __Server_h_

#include "BlockingQueue.h"
#include "ChannelHandler.h"
#include "Exception.h"
#include "Worker.h"

#include <MySocket/ServerSocket.h>
#include <MyThread/thread.h>
#include <vector>
#include <map>

#include <sys/epoll.h>

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

    bool debug;

private:
	ServerSocket *m_server;
    std::vector<Worker*> m_workers;

	int m_port;
	int m_numWorkers;
	int m_backlog;
	ChannelHandler *m_handler;

    BlockingQueue<std::pair<Socket*, std::string> > m_ready_sockets;

    bool m_done;


    // epoll bits
    int m_efd;

    struct epoll_event m_ev;

    int m_maxevents;

};

#endif
// vim: ts=4:sw=4:expandtab
