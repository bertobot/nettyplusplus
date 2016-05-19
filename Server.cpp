#include "Server.h"

Server::Server(int port, int workers, ChannelHandler *handler, TimeoutStrategy ts) {
    debug = false;

	m_handler = handler;
	m_port = port;

	m_server = NULL;
	m_backlog = 512;
	m_numWorkers = workers;

    bzero(&m_ev, sizeof(m_ev));

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

    if (debug) printf("efd: %d\n", m_efd);

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

            if (errno == EINTR)
                printf("signal interrupt occurred.  resuming.\n");

            else
                break;
        }

        for (int i = 0; i < nfds; i++) {

            int rfd = events[i].data.fd;

            // if the connection hung up, close and remove

            if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLHUP || events[i].events & EPOLLERR) {

                if (debug) printf("server %d: %d hangup or error.\n", m_efd, rfd);

                epoll_ctl(m_efd, EPOLL_CTL_DEL, rfd, NULL);

                if (clients[rfd])
                    clients[rfd]->close();

                clients.erase(rfd);
            }

            // the ready socket is the server, we have a new connection

            else if (rfd == m_server->getSocketDescriptor() ) {
                // add client socket

                Socket *client = new Socket(m_server->accept() );

                if (! client->isValid() ) {
                    if (debug) printf("new client connection is invalid!\n");
                    continue;
                }

                client->makeNonBlocking();

                if (debug) printf("server %d: %d client connect.\n", m_efd, client->getSocketDescriptor() );

                m_ev.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;

                m_ev.data.fd = client->getSocketDescriptor();

                epoll_ctl(m_efd, EPOLL_CTL_ADD, client->getSocketDescriptor(), &m_ev);

// if netty++ is built with preamble, can use preamble function
#ifdef PREAMBLE
                std::string preamble = m_handler->preamble();
                if (preamble.empty() ) client->writeLine(preamble);
#endif

                clients[client->getSocketDescriptor()] = client;

            }

            // otherwise, add the ready socket to the work queue

            else if (events[i].events & EPOLLIN) {

                if (debug) printf("server %d: %d client ready.\n", m_efd, rfd);

                // 128k buffer
                int size = 8192;

                char *rbuffer = new char[size + 1];

                std::string payload;

                int rsf = 0;

                while (true) {
                    bzero(rbuffer, size + 1);

                    int rc = read(rfd, rbuffer, size);
            
                    if (rc > 0) {

                        payload += rbuffer;

                        rsf += rc;

                        if (rc < size) break;
                    }

                    else if (rc == 0) {
                        printf("Server::run eof reached.\n");
                        break;
                    }

                    else if (rc == -1 && (errno == EWOULDBLOCK || errno == EAGAIN) ) {
                        printf("Server::run eagain reached.  trying again.\n");
                        //break;
                    }

                    else {
                        // error of some kind
                        if (debug) printf("Server::run error encountered while reading for fd: %d\n", rfd);
                        break;
                    }
                }

                delete [] rbuffer;
                rbuffer = NULL;

                if (debug) printf("Server::run payload '%s', length: %lu\n", payload.c_str(), payload.length() );

                m_ready_sockets.push(std::pair<Socket*, std::string>(clients[rfd], payload) );

                // re-arm
                m_ev.data.fd = rfd;
                m_ev.events = EPOLLIN | EPOLLRDHUP | EPOLLONESHOT;
                epoll_ctl(m_efd, EPOLL_CTL_MOD, rfd, &m_ev);
            }
    
            else {
                if (debug) printf("server %d: %d reached unknown event state.\n", m_server->getSocketDescriptor(), rfd);
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

        if (citr->second) {
            citr->second->close();
            delete citr->second;
        }
    }

}

void Server::stop() {
	m_server->close();

    m_done = true;

    // send poison pill (null channel) which will tell the worker to stop.

	for (int i = 0; i < m_numWorkers; i++)
        m_ready_sockets.push(std::pair<Socket*, std::string>(NULL, "") );

    // TODO: distinguish beteween stop and dtor.
    // this method, once called, destroys server.
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
