INCLUDE=-Icontrib
LIB=-Lcontrib/MyThread -Lcontrib/MySocket -Lcontrib/libstrmanip++
CC=g++ $(CFLAGS) -Wall -std=c++11
OBJ=\
Worker.o \
Server.o 

all: $(OBJ)
	ar rs libnetty++.a $(OBJ)

sample: sample.o all
	$(CC) $(LIB) -o sample sample.o $(OBJ) -lMyThread -lpthread -lSocket -lstrmanip++

.cpp.o:
	$(CC) $(INCLUDE) -c $<

clean:
	rm -f *.o

mrproper: clean
	rm -f *~
	rm -f libnetty++.a
	rm -f server
	rm -f sample

install: all
	mkdir -p /usr/local/include/netty++
	install -D -m 666 *.h /usr/local/include/netty++
	install -D -m 666 *.a /usr/local/lib

uninstall:
	rm -rf /usr/local/include/netty++
	rm -rf /usr/local/lib/libnetty++.a

