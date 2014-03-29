netty++

C++ port of http://netty.io/

Here's an example - an Echo Server, that uses the nett++ library:

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
    int opt_workers = 1;

    int c;

    // getopt

    while (1) {
        static struct option long_options[] = {
            /* These options set a flag. */

            /*
            {"verbose", no_argument,       &verbose_flag, 1},
            {"brief",   no_argument,       &verbose_flag, 0},
            */

            /* These options don't set a flag.  We distinguish them by their indices. */

            /*
            {"add",     no_argument,       0, 'a'},
            {"append",  no_argument,       0, 'b'},
            {"delete",  required_argument, 0, 'd'},
            {"create",  required_argument, 0, 'c'},
            {"file",    required_argument, 0, 'f'},
            */

            // max connections
            {"connections", required_argument, 0,  'c'},

            // daemon mode
            {"daemon",      no_argument,        0,  'd'},

            // port
            {"port",        required_argument,  0,  'p'},

            {0, 0, 0, 0}
        };

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "c:df:l:m:n:p:", long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
            case 0:
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                std::cout << "option " << long_options[option_index].name << std::endl;

                if (optarg)
                    std::cout << " with arg " << optarg << std::endl;

                break;

            case 'w':
                opt_workers = atoi(optarg);
                break;

            case 'p':
                opt_port = atoi(optarg);
                break;

            default:
                //abort ();
                break;
        }
    }

    // end getopt

    // signal catcher first!
    (void) signal(SIGINT,signal_cleanup);
    (void) signal(SIGPIPE,signal_pipe);

    EchoChannelHandler *ech = new EchoChannelHandler();

    // listen on opt_port, with opt_workers to process ech
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
