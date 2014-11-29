#include "Select.h"

Select::Select() {
    init();
    debug = false;
}

Select::Select(const std::vector<int> &list) {
    init();
    debug = false;

    for (unsigned int i = 0; i < list.size(); i++)
        add(list[i]);
}

void Select::remove(int fd) {

    if (fds.empty() )
        return;

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

    if (fds.size() > 0)
        max = fds[fds.size() - 1];
    else
        max = 0;

    if (debug) printf("[Select::remove] removed %d\n", fd);

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

    max = 0;
}

void Select::clear() {
    mLock.lock();
    fds.clear();
    init();
    mLock.unlock();
}

std::vector<int> Select::canRead(long int sec, long int usec) {
    std::vector<int> result;

    struct timeval lts = { sec, usec };

    reset_fds();

    if (fds.size() == 0) {
        if (debug) printf("[Select::canRead] fds is empty.  skipping.\n");
        return result;
    }

    int nfds = max + 1;

    // weird
    if (nfds < 1) {
        if (debug) printf("[Select::canRead] nfds at impossible value: %d.  skipping.\n", nfds);
        
        return result;
    }

    // debug
    if (debug) {
        printf("[Select::canRead] fds: [ ");

        for (unsigned int i = 0; i < fds.size(); i++)
            printf("%d ", fds[i]);

        printf("], nfds: %d, lts = { %ld, %ld }\n", nfds, lts.tv_sec, lts.tv_usec);
    }

    //int ready = select(nfds, &read_fds, NULL, &error_fds, &lts);
    int ready = select(nfds, &read_fds, NULL, NULL, &lts);

	// TODO: 08.07.2014
	// not sure if this should except anymore. capture the error.
	
    if (ready == -1) {
		if (debug) {
            printf("select error: (max: %d) %s\n", nfds - 1, strerror(errno));

            for (unsigned int i = 0; i < fds.size(); i++) {
                printf("%i ", fds[i]);
            }

            printf("\n");
        }

	    throw NIOException(strerror(errno));
    }

    mLock.lock();

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (FD_ISSET(fds[i], &read_fds) )
            result.push_back(fds[i]);
    }

    mLock.unlock();

    return result;
}

std::vector<int> Select::canWrite(long int sec, long int usec) {

    std::vector<int> result;

    struct timeval lts = { sec, usec };

    reset_fds();

    if (fds.size() == 0) {
        if (debug) printf("[Select::canWrite] fds is empty.  skipping.\n");
        return result;
    }

    int nfds = max + 1;

    // weird
    if (nfds < 1) {
        if (debug) printf("[Select::canWrite] nfds at impossible value: %d.  skipping.\n", nfds);
        
        return result;
    }

    if (debug) {
        printf("[Select::canWrite] fds: [ ");

        for (unsigned int i = 0; i < fds.size(); i++)
            printf("%d ", fds[i]);

        printf("], nfds: %d, lts = { %ld, %ld }\n", nfds, lts.tv_sec, lts.tv_usec);
    }

    //int ready = select(nfds, NULL, &write_fds, &error_fds, &lts);
    int ready = select(nfds, NULL, &write_fds, NULL, &lts);

	// TODO: 08.07.2014
	// not sure if this should except anymore. capture the error.
	
    if (ready == -1) {
        if (debug) {
            printf("select error: (max: %d) %s\n", nfds - 1, strerror(errno));

            for (unsigned int i = 0; i < fds.size(); i++) {
                printf("%i ", fds[i]);
            }

            printf("\n");
        }

	    throw NIOException(strerror(errno));
    }

    mLock.lock();

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (FD_ISSET(fds[i], &write_fds) )
            result.push_back(fds[i]);
    }

    mLock.unlock();

    return result;
}

void Select::wait(long int sec, long int usec) {
    int nfds = 0;
    struct timeval lts = { sec, usec };

    select(nfds, NULL, NULL, NULL, &lts);
}

bool Select::empty() {
	return (fds.size() == 0);
}

// this method does an insert-sort 
void Select::add(int fd) {

    if (fd < 0) {
        if (debug) printf("[Select::add] rejecting add of invalid fd: %d\n", fd);
        return;
    }

    mLock.lock();

    // debug, dups
    if (debug) {
        printf("[Select::add] adding %d to [", fd);

        for (unsigned int i = 0; i < fds.size(); i++)
            printf(" %d", fds[i]);

        printf(" ]\n");
    }

	if (fds.size() == 0) fds.push_back(fd);

	else {
		int index = searchIndex(fd, 0, fds.size() );

        // TODO: no dups.  may need to change that behavior.
        if (fd == fds[index]) {
            if (debug) printf("[Select::add] duplicate detected. skipping.\n");
            return;
        }

		if (fd > fds[index]) index++;

		fds.push_back(-1);

		for (int i = fds.size() - 1; i > index; i--)
			fds[i] = fds[i-1];

		fds[index] = fd;
	}

    add_fd(fd);

    max = fds[fds.size() - 1];

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

int Select::removeStale() {
    int result = 0;

    printf("Select::removeStale\n"); // debug

    mLock.lock();

    for (unsigned int i = 0; i < fds.size(); i++) {
        if (fds[i] < 0 || fds[i] > 65535) {
            printf("%d ", fds[i]);  // debug

            result++;

            // slide everybody down one.
            for (unsigned int j = i; j < fds.size() - 1; j++)
                fds[j] = fds[j+1];

            fds.pop_back();
        }
    }
            
    mLock.unlock();

    printf("Select::removeStale - removed %d stale fds.\n", result);

    return result;
}

void Select::reset_fds() {
    mLock.lock();
    
    for (unsigned int i = 0; i < fds.size(); i++)
        add_fd(fds[i]);

    mLock.unlock();
}

// vim: ts=4:sw=4:expandtab

