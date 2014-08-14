#include "SelectSocket.h"

void SelectSocket::add(const Socket &s) {
	mSelect.add(s.getSocketDescriptor() );
}

void SelectSocket::remove(const Socket &s) {
	mSelect.remove(s.getSocketDescriptor() );
}

std::vector<Socket> SelectSocket::canRead() {
	std::vector<Socket> result;


	/*
	std::vector<int> fds = mSelect.getDescriptors();

	for (unsigned int i = 0; i < fds.size(); i++) {
		Socket s(fds[i]);

		if (! s.isConnected() ) {
			// debug
			printf("[SelectSocket::canRead preemptively removed a closed socket: %d\n", fds[i]);
			mSelect.remove(fds[i]);
			i--;
		}
	}
	*/

	std::vector<int> ready = mSelect.canRead();

	for (unsigned int i = 0; i < ready.size(); i++)
		result.push_back(Socket(ready[i]) );

	return result;
}

std::vector<Socket> SelectSocket::canWrite() {
	std::vector<Socket> result;

	std::vector<int> ready = mSelect.canWrite();

	for (unsigned int i = 0; i < ready.size(); i++)
		result.push_back(Socket(ready[i]) );

	return result;
}

std::vector<Socket> SelectSocket::canReadWrite() {
	std::vector<Socket> result;

	std::vector<int> ready = mSelect.canReadWrite();

	for (unsigned int i = 0; i < ready.size(); i++)
		result.push_back(Socket(ready[i]) );

	return result;
}

void SelectSocket::setTimeout(long int sec, long int nsec) {
	mSelect.setTimeout(sec, nsec);
}

void SelectSocket::setTimeout(double s) {
	mSelect.setTimeout(s);
}

bool SelectSocket::empty() {
	return mSelect.empty();
}

void SelectSocket::clear() {
	mSelect.clear();
}

std::vector<int> SelectSocket::getDescriptors() {
	return mSelect.getDescriptors();
}

