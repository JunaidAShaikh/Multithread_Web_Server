all: myhttpd.cpp
	g++ -o myhttpd myhttpd.cpp -lpthread
