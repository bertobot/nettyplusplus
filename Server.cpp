#include "Server.h"

Server::Server(int port, int workers, ChannelHandler *handler, TimeoutStrategy ts) {
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
    //m_server->setLinger(true, 0);
	m_server->listen(m_backlog);

    m_done = false;

    for (int i = 0; i < m_numWorkers; i++) {
        Worker *w = new Worker(handler, ts);
        w->start();
        m_workers.push_back(w);
    }
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

    SelectSocket serverSelect(60, 0);
    
    serverSelect.add(m_server->getSocketDescriptor() );
    
    while (! m_done) {

        try {
            if (serverSelect.canRead().size() > 0) {
                Socket client = m_server->accept();

                // set the client socket linger to 0.
                //client.setLinger(true, 0);
                
                // debug
                printf("Server::run add client: %d\n", client.getSocketDescriptor() );

                if (! client.isValid() ) {
                    printf("Server::run error accepting socket: %s\n", strerror(errno));
                    continue;
                }

                m_workers[workerNum]->addClient(client);
   
                // debug
                printf("Server::run added client: %d\n", client.getSocketDescriptor() );

                if (++workerNum >= m_numWorkers) workerNum = 0;
            }

            // check if any of the workers called shutdown
            for (int i = 0; i < m_numWorkers; i++) {
                if (m_workers[i]->shutdownCalled() ) {
                    m_done = true;
                    break;
                }
            }
        }
        catch (NIOException &nio) {
            printf("netty++ Server loop caught NIO Exception.");
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
}

ChannelHandler * Server::getChannelHandler() {
	return m_handler;
}

void Server::setChannelHandler(ChannelHandler *handler) {
    m_handler = handler;
}

// vim: ts=4:sw=4:expandtab
