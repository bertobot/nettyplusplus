#include "SelectSocket.h"

void SelectSocket::add(const Socket &s) {
	mSelect.add(s.getSocketDescriptor() );
}

void SelectSocket::remove(const Socket &s) {
	mSelect.remove(s.getSocketDescriptor() );
}

std::vector<Socket> SelectSocket::canRead(long int sec, long int nsec) {
	std::vector<Socket> result;


	std::vector<int> ready = mSelect.canRead(sec, nsec);

	for (unsigned int i = 0; i < ready.size(); i++)
		result.push_back(Socket(ready[i]) );

	return result;
}

std::vector<Socket> SelectSocket::canWrite(long int sec, long int nsec) {
	std::vector<Socket> result;

	std::vector<int> ready = mSelect.canWrite(sec, nsec);

	for (unsigned int i = 0; i < ready.size(); i++)
		result.push_back(Socket(ready[i]) );

	return result;
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

