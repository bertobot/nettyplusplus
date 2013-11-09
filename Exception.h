#ifndef __Exception_h_
#define __Exception_h_

#include <string>

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

class Exception {
private:
	std::string message;

public:
    Exception(const std::string &m = "Generic Exception.") : message(m) { }
    virtual ~Exception() throw() {}
    virtual const char* what() const throw() { return message.c_str(); }
    
    void printStackTrace() {
        void *array[255];
        size_t size;
        char **strings;
        size_t i;

        size = backtrace (array, 10);
        strings = backtrace_symbols (array, size);

        printf ("Obtained %zd stack frames.\n", size);

        for (i = 0; i < size; i++)
            printf ("%s\n", strings[i]);

        free (strings);
    }
};

#endif

// vim: ts=4:sw=4:expandtab
