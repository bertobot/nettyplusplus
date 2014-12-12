#include "Server.h"

Server::Server(int port, int workers, ChannelHandler *handler, TimeoutStrategy ts) {
	m_handler = handler;
	m_port = port;

	m_server = NULL;
	m_backlog = 512;
	m_numWorkers = workers;

    // TODO: make tunable
    // 20k.
    m_maxevents = 20000;

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
    m_server->makeNonBlocking();

    m_done = false;

    for (int i = 0; i < m_numWorkers; i++) {
        Worker *w = new Worker(handler, &m_ready_sockets, ts);
        w->start();
        m_workers.push_back(w);
    }


    // epoll bits

    // we create the epoll fd and then attach the server's fd to the polling event.

    m_efd = epoll_create(1);

    if (m_efd == -1)
        throw NIOException(strerror(errno));

    m_ev.events = EPOLLIN;
    m_ev.data.fd = m_server->getSocketDescriptor();

    if (epoll_ctl(m_efd, EPOLL_CTL_ADD, m_server->getSocketDescriptor(), &m_ev) )
        throw NIOException(strerror(errno));

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

    std::map<int, Socket*> clients;

    struct epoll_event events[m_maxevents];

    while (! m_done) {

        // last param is timeout in millis
        int nfds = epoll_wait(m_efd, events, m_maxevents, 1000);

        if (nfds == -1) {
            printf("error in server loop: %s\n", strerror(errno));
            break;
        }

        for (int i = 0; i < nfds; i++) {

            // if the connection hung up, close and remove

            if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLERR) {

                clients.erase(events[i].data.fd);

                shutdown(events[i].data.fd, SHUT_RDWR);

                close(events[i].data.fd);
            }

            // the ready socket is the server, we have a new connection

            Socket *ready = new Socket(events[i].data.fd);

            if (ready->getSocketDescriptor() == m_server->getSocketDescriptor() ) {
                // add client socket

                Socket *client = new Socket(m_server->accept() );

                m_ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLERR;

                m_ev.data.fd = client->getSocketDescriptor();

                epoll_ctl(m_efd, EPOLL_CTL_ADD, client->getSocketDescriptor(), &m_ev);

                clients[client->getSocketDescriptor()] = client;
            }

            // otherwise, add the ready socket to the work queue

            else {

                clients[ready->getSocketDescriptor()] = ready;

                m_ready_sockets.push(ready);
            }

        }


        // check if any of the workers called shutdown
        for (int i = 0; i < m_numWorkers; i++) {
            if (m_workers[i]->shutdownCalled() ) {
                m_done = true;
                break;
            }
        }


    }

    // client cleanup

    std::map<int, Socket*>::iterator citr = clients.begin();

    for (; citr != clients.end(); citr++) {
        Socket *s = citr->second;

        if (s) {
            s->close();
            delete s;
            s = NULL;
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
