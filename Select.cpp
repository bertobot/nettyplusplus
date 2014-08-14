#include "Select.h"

Select::Select(long int sec, long int nsec) {
    init();

    this->sec = sec;
    this->nsec = nsec;
}

Select::Select(const std::vector<int> &list, long int sec, long int nsec) {
    init();

    this->sec = sec;
    this->nsec = nsec;

    for (unsigned int i = 0; i < list.size(); i++)
        add(list[i]);
}

Select::~Select() {
}

void Select::remove(int fd) {

    mLock.lock();

    remove_fd(fd);

    if (fds.size() == 1 && fds[0] == fd) {
        fds.pop_back();
        mLock.unlock();
        return;
    }

    int index = searchIndex(fd, 0, fds.size());

    if (fds[index] == fd) {
        for (unsigned int i = index; i < fds.size() - 1; i++)
            fds[i] = fds[i+1]; 

        fds.pop_back();
    }

    // debug
    printf("[Select::remove] removed %d\n", fd);

    mLock.unlock();
}

void Select::add_fd(int fd) {
    FD_SET(fd, &read_fds);
    FD_SET(fd, &write_fds);
    FD_SET(fd, &error_fds);
}

void Select::remove_fd(int fd) {
    FD_CLR(fd, &read_fds);
    FD_CLR(fd, &write_fds);
    FD_CLR(fd, &error_fds);
}

void Select::init() {
    // initialize sets
    FD_ZERO(&read_fds);
    FD_ZERO(&write_fds);
    FD_ZERO(&error_fds);
}

void Select::clear() {
    mLock.lock();
    fds.clear();
    init();
    mLock.unlock();
}

std::vector<int> Select::canRead() {
    // TODO: update canWrite and canReadWrite
    mLock.lock();

    std::vector<int> result;
    struct timespec lts;

    bzero(&lts, sizeof(lts));

    //lts = { sec, nsec };
    lts = { 1, 0 };

    if (fds.size() == 0) {
        printf("[Select::canRead] fds is empty.  skipping.\n");
        mLock.unlock();
        return result;
    }

    int nfds = fds[fds.size() - 1] + 1;

    // weird
    if (nfds < 1) {
        printf("[Select::canRead] nfds at impossible value: %d.  skipping.\n", nfds);
        
        mLock.unlock();
        return result;
    }

    // debug
    printf("[Select::canRead] fds: [ ");

    for (unsigned int i = 0; i < fds.size(); i++)
        printf("%d ", fds[i]);

    printf("], nfds: %d, lts = { %ld, %ld }\n", nfds, lts.tv_sec, lts.tv_nsec);

    int ready = pselect(nfds, &read_fds, NULL, &error_fds, &lts, NULL);

	// TODO: 08.07.2014
	// not sure if this should except anymore. capture the error.
	
    if (ready == -1) {
		printf("select error: (max: %d) %s\n", nfds - 1, strerror(errno));

        for (unsigned int i = 0; i < fds.size(); i++) {
            printf("%i ", fds[i]);
        }

        printf("\n");

        mLock.unlock();

	    throw NIOException(strerror(errno));
    }

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (FD_ISSET(fds[i], &read_fds) ) {
            result.push_back(fds[i]);
        }
    }

    mLock.unlock();

    return result;
}

std::vector<int> Select::canWrite() {
    mLock.lock();

    std::vector<int> result;
    struct timespec lts = { sec, nsec };

    if (fds.size() == 0) {
        mLock.unlock();
        return result;
    }

    int nfds = fds[fds.size() - 1] + 1;

    int ready = pselect(nfds, NULL, &write_fds, NULL, &lts, NULL);

    if (ready == -1) {
        printf("exception in Select: %s\n", strerror(errno) );
        mLock.unlock();
        throw NIOException(strerror(errno));
    }

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (FD_ISSET(fds[i], &write_fds) )
            result.push_back(fds[i]);
    }

    mLock.unlock();

    return result;
}

std::vector<int> Select::canReadWrite() {
    mLock.lock();

    std::vector<int> result;
    struct timespec lts = { sec, nsec };

    if (fds.size() == 0) {
        mLock.unlock();
        return result;
    }

    int nfds = fds[fds.size() - 1] + 1;

    int ready = pselect(nfds, &read_fds, &write_fds, NULL, &lts, NULL);

    if (ready == -1) {
        mLock.unlock();
        printf("exception in Select: %s\n", strerror(errno) );
        throw NIOException(strerror(errno));
    }

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (FD_ISSET(fds[i], &read_fds) )
            result.push_back(fds[i]);

		else if (FD_ISSET(fds[i], &write_fds) )
			result.push_back(fds[i]);
    }

    mLock.unlock();

    return result;
}

void Select::wait() {
    int nfds = 0;
    struct timespec lts = { sec, nsec };

    pselect(nfds, NULL, NULL, NULL, &lts, NULL);
}

void Select::setTimeout(long int s, long int ns) {
    sec = s;
    nsec = ns;
}

void Select::setTimeout(double t) {
    // first, get the base
    long t1 = t / 1;
    double t2 = (t / 1.0) - t1;

    // 1 billion nanos = 1 second
    long t3 = t2 * 1000000000;

    sec = t1;
    nsec = t3;
}

bool Select::empty() {
	return (fds.size() == 0);
}

// this method does an insert-sort 
void Select::add(int fd) {

    if (fd < 0) {
        printf("[Select::add] rejecting add of invalid fd: %d\n", fd);
        return;
    }

    mLock.lock();

    // debug, dups
    printf("[Select::add] adding %d to [", fd);

    for (unsigned int i = 0; i < fds.size(); i++)
        printf(" %d", fds[i]);

    printf(" ]\n");

	if (fds.size() == 0) fds.push_back(fd);

	else {
		int index = searchIndex(fd, 0, fds.size() );

        // TODO: no dups.  may need to change that behavior.
        if (fd == fds[index]) {
            printf("[Select::add] duplicate detected. skipping.\n");
            return;
        }

		if (fd > fds[index]) index++;

		fds.push_back(-1);

		for (int i = fds.size() - 1; i > index; i--)
			fds[i] = fds[i-1];

		fds[index] = fd;
	}

    add_fd(fd);

    mLock.unlock();

}

int Select::searchIndex(int fd, int start, int end) {
	int mid = ((end - start) / 2) + start;

	if (mid == start || mid == end)
		return mid;

	if (fd > fds[mid]) return searchIndex(fd, mid, end);

	else if (fd < fds[mid]) return searchIndex(fd, start, mid);

	return mid;
}

std::vector<int> Select::getDescriptors() {
    return fds;
}

// vim: ts=4:sw=4:expandtab

