#ifndef __Select_h_
#define __Select_h_

#include <MySocket/NIOException.h>
#include <MyThread/mutex.h>

#include <vector>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

class Select {
public:
    Select(long int sec=1, long int nsec=0);
    Select(const std::vector<int>&, long int sec=1, long int nsec=0);
    virtual ~Select();

    std::vector<int> canRead();
    std::vector<int> canWrite();
    std::vector<int> canReadWrite();
    
    bool hasEventError();

    void add(int);
    void remove(int);
    void clear();

    void wait();

    void setTimeout(long sec, long nsec);
    void setTimeout(double t);

    bool empty();

    std::vector<int> getDescriptors();

private:
    void add_fd(int fd);

    void remove_fd(int);

    void init();

    int searchIndex(int fd, int start, int end);

    fd_set
        read_fds,
        write_fds,
        error_fds;

    long int sec, nsec;

    std::vector<int> fds;

    mutex mLock;

};
#endif
// vim: ts=4:sw=4:expandtab

