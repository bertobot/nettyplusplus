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

    m_done = false;

    for (unsigned int i = 0; i < m_numWorkers; i++)
        m_workers.push_back(new Worker(handler) );
}

Server::~Server() {
	stop();

    // finally, join all threads
    for (int j = 0; j < m_numWorkers; j++) {
        Worker *c = m_workers[j];
        if (c) {
            c->join();
            delete c;
            c = NULL;
        }
    }

	m_handler = NULL;
	m_server = NULL;
}

void Server::run() {

    int workerNum = 0;

    SelectSocket serverSelect;
    
    serverSelect.setTimeout(60, 0);

    serverSelect.add(m_server->getSocketDescriptor() );
    
    while (! m_done) {

        if (serverSelect.canRead().size() > 0) {
			Socket client = m_server->accept();
            
            m_workers[workerNum]->addClient(client);
            m_workers[workerNum]->start();

            if (++workerNum >= m_numWorkers) workerNum = 0;
        }
    }
}

void Server::stop() {
	m_server->close();

    m_done = true;

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
