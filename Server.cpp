#include "Server.h"

Server::Server(int port, int workers, ChannelHandler *handler) {
	m_handler = handler;
	m_port = port;

	m_server = NULL;
	m_backlog = 128;
	m_numWorkers = workers;
}

Server::~Server() {
/*
	close();

	m_handler = NULL;
	m_server = NULL;
*/
}

void Server::run() {
	m_server = new ServerSocket(m_port);

	if (! m_server->isValid() ) {
		printf("problem creating socket: %d\n", errno);
		//return 1;
		return;
	}

	if (! m_server->isBound() ) {
		printf("binding error: %s\n", strerror(errno) );
		//exit(5);
		return;
	}

	const int set = 1;
	m_server->setOption(SOL_SOCKET, SO_REUSEADDR, set);
	m_server->listen(m_backlog);

    for (int i = 0; i < m_numWorkers; i++) {
        Worker *newWorker = new Worker(m_server, m_handler);

        newWorker->setWorkerId(i);

        newWorker->start();
        printf("started thread %d.\n", i);

        m_workers.push_back(newWorker);
    }

	// finally, join all threads
	for (int j = 0; j < m_numWorkers; j++) {
		Worker *c = m_workers[j];
		if (c) {
			c->join();
			delete c;
			c = NULL;
		}
	}
}

void Server::close() {
	m_server->close();
	delete m_server;
	m_server = NULL;

	printf("goodbye.\n");
}

ChannelHandler * Server::getChannelHandler() {
	return m_handler;
}

