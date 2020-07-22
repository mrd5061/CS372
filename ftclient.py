"""
Programmer Name: Meghan Dougherty
Program Name: ftclient.py or fltclient(executable)
Course name: CS372
Last Modified: 12/1/19

Description:

The client side program for project 2, a simple file transfer system. 
Connects to a server on a port specified by the first command line argument
Then sends a request for either a directory listing or a file ( plus file 
name) on a data connection. The ip address and port num for the data connection
are the last two command line arguments. Prints the directory listing or 
saves the transfered file contents into a new file. 

Specific sources are noted next to pertinetn code.
"""

#!/bin/python
import sys
from socket import *
from os import path

"""
createSock()

pre-conditions:
	the last command line argyment is a valid port number
post conditions:
	a valid socket is created
description:
	creates, binds and listens on a socket specified by the last
	command line argument
"""
def createSock():
	#stackabuse.com/command-line-arguments-in-python
	#get the last command line argument. Command line
	#arguments vary based on whether the command is -g or -l
	servPort = int(sys.argv[len(sys.argv)-1]);

	# create, bind, and listen on the socket.
	servSock = socket(AF_INET, SOCK_STREAM)
	servSock.bind(('',servPort))
	servSock.listen(1)
	dataSock, address = servSock.accept()

	#return the socket.
	return dataSock

"""
connectServ()
pre-conditions: 
	the first command line argument is a valid flip server name
post-conditions:
	the client socket is connected.
description:
	creates and connects the socket used to communicate with the server
"""
def connectServ():
	#create the server name by combining the first command line 
	#argument with ".engr.oregonstate.edu"
	servName = sys.argv[1]+".engr.oregonstate.edu"

	#retrieve the server port number
	servPort = int(sys.argv[2])
	clientSock = socket(AF_INET,SOCK_STREAM)

	#connect the server socket.
	clientSock.connect((servName,servPort))
	return clientSock

"""
getIP
pre conditions:
	None
post conditions:
	returns the local IP address
"""
def getIP():
	#stackoverflow.com/questions/166506/finding-local-ip-addresses-using-pythons-stdlib/9267833
	#create a socket 
	s = socket(AF_INET,SOCK_DGRAM)
	s.connect(("8.8.8.8", 80))
	#return the sockets name, the local IP address
	return s.getsockname()[0]

"""
getFile (dataSock)
pre-conditions:
	dataSock is a valid connected data socket
post-conditions:
	a file is written to the local directory
description:
	recieves file data from the server and writes it to a local file. 
	If the file already exists in the directory, the funciton appends
	_copy.txt to the file name. 
"""
def getFile(dataSock):

	#get the file name
	fileName = sys.argv[4]
	#see if the file exists in the local dir.
	if path.isfile(fileName):
		#if it does, append _.copy.txt to the file name
		fileName = fileName.split(".")[0] + "_copy.txt"
	#open the local file for writing.
	f = open(fileName, "w")
	#recieve the first 100 characters
	buff = dataSock.recv(100)
	
	#keep recieving until the done message is recieved
	while buff != "__done__":
		#strip any null characters the sender may have added
		buff = buff.rstrip("\x00")
		#write the recieved data
		f.write(buff)
		#recieve more data
		buff = dataSock.recv(100)
"""
getDir(dataSock)
pre-conditions: 
	dataSock is a valid, connected data socket
post-conditions:
	the recieved directory list is printed to the terminal
description:
	recieves the directory list from the server and prints it to the
	terminal.
"""
def getDir(dataSock):

	#get the first file name and print it.
	fileName = dataSock.recv(100)

	print fileName

	#Keep looping until the done message is recieved.
	while fileName != "Done":

		print fileName
		fileName = dataSock.recv(100)
"""
getData(clientSock)
pre-conditions: 
	clientSock is a valid, connected socket.
	command line inputs are valid per program requirements
post-conditions:
	command line input is sent to the server
"""
def getData(clientSock):
	
	#print appropriate messages to terminal based on user command choice
	if sys.argv[3] == "-g":
		print "Retrieving file {}".format(sys.argv[4])
	elif sys.argv[3] == "-l":
		print "Retrieving directory list"
	
	#Send the data socket number
	clientSock.send(sys.argv[len(sys.argv)-1])

	clientSock.recv(1024)
	
	# send the command (-g or -l)
	clientSock.send(sys.argv[3])
	
	clientSock.recv(1024)

	# send the IP address
	clientSock.send(getIP())
	
	response = clientSock.recv(1024)
	
	#if the ip was invalid, print an error message and quit.
	if response == "bad":
		print "Invalid command"
		exit(1)
	# if the user wants a file, send the file name.
	if sys.argv[3] == "-g":
		clientSock.send(sys.argv[4])
		response2 = clientSock.recv(1024)
	
		#send an error message if the file wasn't found.
		if response2 == "bad":
			print ("file not found")
			return
	#create the data socket
	dataSock = createSock()

	#call the appropriate function based on the user command.
	if sys.argv[3] == "-g":
		getFile(dataSock)
	else: 
		getDir(dataSock)
	#close the data socket.
	dataSock.close()
"""
main()
pre-conditions: none
post-conditions: none
description: 
	main driver function for the ftclient program. Validates user input
	and initiates the transfer. 
"""

if __name__ == "__main__":
	# validate that we have the correct number of arguments.	
	if len(sys.argv) < 5 or len(sys.argv) > 6:
		print "Incorrect command line arguments"
		exit (1)
	#Validate that the server name is either flip1 flip2 or flip3
	elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1]!="flip3"):
		print "Incorrect server name"
		exit(1)
	#make sure both ports number are in a valid range.
	elif(int(sys.argv[2]) < 1024 or int(sys.argv[2]) > 665535):
		print "Incorrect Port Num"
		exit(1)
	elif (sys.argv[len(sys.argv)-1] < 1024 or sys.argv[len(sys.argv)-1] < 665535):
		print "Incorrect Data Port Num"

	#validate the user command. 
	elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):
		print "Incorrect command"
		exit(1)
	
	#create the client socket 
	clientSock = connectServ()
	#initiate the file transfer
	getData(clientSock)
