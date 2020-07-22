# CS372 - Project 2

A simple chat program utilizing the sockets API and TCP protocols. 

To compile the ftserver program type "make ftserver" into the command line.
the executable is called ftserver.
To run the program type "ftserver [portnum]" into the command line. Port Num
must be betwee 1024 and 65535.

To run the client side program type either into the following into the command line:

python ftclient.py [server][port num] -l [data port num]
python ftclient.py [server][port num] -g [file name][data port num]

As with the server, poth port nums must be between 1024 and 65535.
