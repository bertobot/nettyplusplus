#ifndef __SelectSocket_h_
#define __SelectSocket_h_

#include "Select.h"
#include <MySocket/Socket.h>

class SelectSocket {
public:
    SelectSocket() : mSelect() {}
    ~SelectSocket() {}

    std::vector<Socket> canRead(long int sec=1, long int nsec=0);
	std::vector<Socket> canWrite(long int sec=1, long int nsec=0);

	void add(const Socket &s);
	void remove(const Socket &s);

	bool empty();

	void clear();

	std::vector<int> getDescriptors();

private:
	Select mSelect;

};

#endif
// vim: set ts=4:sw=4:expandtab
