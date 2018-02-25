# Multithread_Web_Server
A multi-thread web server serving multiple http requests in FCFS/Shortest Job First orders.

SYNOPSIS:  myhttpd [−d] [−h] [−l file] [−p port] [−r dir] [−t time] [−n threadnum] [−s sched] 

MakeFile has been included to handle the compilation .
Compile file using the following :-
g++ myhttpd.cpp -o myttpd -lpthread

For execution the flags can be set as follows:

−d 
 Enter debugging mode.accept one connection at a time and enable logging to stdout.  Without this option, the web server should run as a daemon process in the background. 
−h
 Print a usage summary with all options and exit. 
−l file
Log all requests to the given file. See LOGGING for details. 
−p port 
Listen on the given port. If not provided, myhttpd will listen on port 8080. 
−r dir
 Set the root directory for the http server to dir.
−t time
 Set the queuing time to time seconds. The default should be 60 seconds.  
−n threadnum 
Set number of threads waiting ready in the execution thread pool to threadnum. The default should be 4 execution threads.  
−s sched 
Set the scheduling policy. It can be either FCFS or SJF. The default will be FCFS
