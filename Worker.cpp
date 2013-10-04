#include "Worker.h"
/////////////////////////////////////////////////
Worker::Worker(ServerSocket *server, ChannelHandler *handler) : thread() {
    // TODO: this should throw an exception if server is null.

    shutdownflag = false;
    this->server = server;
    this->client = NULL;
    
	m_handler = handler;

    workerId = -1;
}
/////////////////////////////////////////////////
void Worker::run() {
	// TODO: make ServerException
	if (! server) throw Exception("server is null.");

	while (! shutdownflag) {
		try {
			client = new Socket(server->accept() );

			if (m_handler) {
				m_handler->onStart();

				Select s;
				s.setTimeout(1,0);

				s.add(client->getSocketDescriptor() );

				while (client->isValid() ) {
					std::vector<int> list = s.canRead();

					if (list.size() > 0) {
						m_handler->onMessageReceived(*client);
					}
				}
			}

			client->close();
		}

		catch(...) {
			printLocal("Worker::run exception");
		}

        // clean up

		delete client;
        client = NULL;

		std::stringstream ss;
		ss << "thread " << workerId << " finished.";
		printLocal(ss.str() );
	}

	std::stringstream ss;
	ss << "Worker " << workerId << " finished.";
	printLocal(ss.str() );
}
/////////////////////////////////////////////////
void Worker::stop() {
    shutdownflag = true;
}
/////////////////////////////////////////////////
int Worker::close() {
	int rc = -2;
	if (client) {
	    rc = client->close();
    }
	return rc;
}
/////////////////////////////////////////////////
void Worker::printLocal(const std::string & str) {
    time_t seconds = time(NULL);
    std::cerr << seconds << " [threadserver] " << str << std::endl;
}
/////////////////////////////////////////////////
void Worker::printLocalAndRespond(const std::string & str) {
    printLocal(str);
    respond(str);
}
/////////////////////////////////////////////////
void Worker::respond(const std::string &str) {
    if (! client) {
        // TODO: should this be an exception?
        
        printLocal("respond: client is null!\n");
        return;
    }
    
    if (client->isValid() ) {
        // TODO: make this tunable
        Select s;

        s.setTimeout(10, 0);
        s.add(client->getSocketDescriptor() );

        try {
            if (! s.canWrite().empty() )
                client->write(str + "\r\n", MSG_NOSIGNAL);
            else
                printLocal("error: couldn't write response.\n");
        }
        catch (...) {
            printLocal("fatal: couldn't write!\n");
        }
    }
    else {
        printLocal("warning: was not able to send '" + str + "' because socket wasn't ready for writing.");
    }
}
/////////////////////////////////////////////////
void Worker::setWorkerId(int id) {
	workerId = id;
}
/////////////////////////////////////////////////
int Worker::getWorkerId() const {
	return workerId;
}
/////////////////////////////////////////////////
Worker::~Worker() {
    close();

    if (client)
        delete client;

    if (server)
        delete server;
}
/////////////////////////////////////////////////
