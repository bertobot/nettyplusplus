#include "SocketPool.h"

SocketPool::SocketPool()
{

}

void SocketPool::setTimeout(double seconds, double milliseconds)
{
    select.setTimeout(seconds, milliseconds);
}

void SocketPool::add(Socket* socket)
{
    pool.push_back(socket);
}

std::vector< int > SocketPool::canRead()
{
    return select.canRead();
}

std::vector< int > SocketPool::canWrite()
{
    return select.canWrite();
}

bool SocketPool::empty()
{
    return pool.empty();
}

SocketPool::~SocketPool()
{

}
