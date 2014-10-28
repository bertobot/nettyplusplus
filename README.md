netty++

C++ port of http://netty.io/

Here's an example - an Echo Server, that uses the netty++ library:

```c++
#include "Server.h"
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>

void signal_cleanup(int);
void signal_pipe(int);

// Derive ChannelHandler that simply echo's to the screen what was received.
class EchoChannelHandler : public ChannelHandler {
public:
	EchoChannelHandler() { }

	virtual ~EchoChannelHandler() { }

	void onMessageReceived(Channel &channel) {
		std::string str = channel.readLine();
		channel.write(str + "\r\n");
	}

};

int main(int argc, char **argv) {
    int opt_port = 33000;
    int opt_workers = 4;

    // signal catcher first!
    (void) signal(SIGINT,signal_cleanup);
    (void) signal(SIGPIPE,signal_pipe);

    EchoChannelHandler *ech = new EchoChannelHandler();

    // listen on opt_port, with opt_workers to process the Echo Channel Handler
    Server server(opt_port, opt_workers, ech);

    server.run();

    // or server.start() for threaded run.

    return 0;
}

void signal_cleanup(int sig) {
    std::cout << "sigint received.  going down." << std::endl;
    exit(sig);
}

void signal_pipe(int sig) {
    std::cout << "sigpipe caught.  doing nothing!" << std::endl;
}

```
