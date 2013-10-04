#ifndef __ChannelHandler_h_
#define __ChannelHandler_h_

/*

This is an interface object that handles channels.

It is meant to be overriden by 'client'.

*/

#include <MySocket/Socket.h>

typedef Socket Channel;

class ChannelHandler {
public:

	ChannelHandler() { }
	virtual ~ChannelHandler() { }

	/*
		This method handles message received on a channel.
	*/
	virtual void onMessageReceived(Channel &channel) { }

	/*
		This method is called when an exception occurs on a channel.
	*/
	virtual void onException() { }

	/*
		This method is called when a channel is newly accepted.
	*/
	virtual void onStart() { }
};

#endif

