#ifndef __SelectSocket_h_
#define __SelectSocket_h_

#include "Select.h"
#include <MySocket/Socket.h>

class SelectSocket {
public:
    SelectSocket(long int sec = 1, long int usec = 0) : mSelect(sec, usec) {}
    virtual ~SelectSocket() {}

    std::vector<Socket> canRead();
	std::vector<Socket> canWrite();
	std::vector<Socket> canReadWrite();

	void add(const Socket &s);
	void remove(const Socket &s);

	void setTimeout(long int sec, long int nsec);
	void setTimeout(double);

	bool empty();

	void clear();

	std::vector<int> getDescriptors();

private:
	Select mSelect;

};

#endif
// vim: set ts=4:sw=4:expandtab
