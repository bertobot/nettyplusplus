#include "Server.h"

Server::Server(int port, int workers, ChannelHandler *handler) {
	m_handler = handler;
	m_port = port;

	m_server = NULL;
	m_backlog = 128;
	m_numWorkers = workers;

    // init server
	m_server = new ServerSocket(m_port);

	if (! m_server->isValid() ) {
        std::stringstream ss;
        ss << "Error creating socket: " << errno;
        throw Exception(ss.str());
	}

	if (! m_server->isBound() ) {
        std::stringstream ss;
        ss << "Binding error: " << errno;
        throw Exception(ss.str());
	}

	const int set = 1;
	m_server->setOption(SOL_SOCKET, SO_REUSEADDR, set);
	m_server->listen(m_backlog);

}

Server::~Server() {
/*
	close();

	m_handler = NULL;
	m_server = NULL;
*/
}

void Server::run() {
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

	for (int i = 0; i < m_numWorkers; i++) {
		Worker *c = m_workers[i];
		if (c)
			c->stop();
	}

	delete m_server;
	m_server = NULL;

	printf("goodbye.\n");
}

ChannelHandler * Server::getChannelHandler() {
	return m_handler;
}

// vim: ts=4:sw=4:expandtab
