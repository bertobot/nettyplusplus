#ifndef __SocketPool_h_
#define __SocketPool_h_

#include <MySocket/Socket.h>
#include "Select.h"
#include <vector>

class SocketPool {
protected:
    std::vector<Socket*> pool;
    Select select;
    

public:
    SocketPool();

    void add(Socket *socket);

    void setTimeout(double seconds, double milliseconds);
    std::vector<int> canRead();
    std::vector<int> canWrite();

    bool empty();

    virtual ~SocketPool();
};

#endif
