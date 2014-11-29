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
    Select();
    Select(const std::vector<int>&);
    ~Select() {}

    std::vector<int> canRead(long int sec=1, long int usec=0);
    std::vector<int> canWrite(long int sec=1, long int usec=0);
    std::vector<int> canReadWrite(long int sec=1, long int usec=0);
    
    bool hasEventError();

    void add(int);
    void remove(int);
    void clear();

    void wait(long int sec, long int usec);

    bool empty();

    std::vector<int> getDescriptors();

    // returns the number of stale descriptors cleared.
    int removeStale();



    bool debug;

private:
    void add_fd(int fd);

    void remove_fd(int);

    void reset_fds();

    void init();

    int searchIndex(int fd, int start, int end);

    fd_set
        read_fds,
        write_fds,
        error_fds;

    std::vector<int> fds;

    mutex mLock;

    int max;

};
#endif
// vim: ts=4:sw=4:expandtab

