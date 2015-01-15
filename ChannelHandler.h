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
	virtual void onMessageReceived(Channel &channel, std::string &payload) { }

	/*
		This method is called when an exception occurs on a channel.
	*/
	virtual void onException() { }

	/*
		This method is called when a channel is newly accepted.
	*/
	virtual std::string preamble() { return ""; }

	/*
		This method is called when a channel is about to be closed.
		If this method returns true, the listening thread in Server
		will shutdown.  Returns false by default.
	*/
	virtual bool shutdownOnExit(Channel &channel) { return false; }

	/*
		This method is called when (and only when) the Worker select expires and no socket was ready.
	*/
	virtual void onIdle() { }
};

#endif

