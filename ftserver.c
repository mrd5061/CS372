/* Programmer Name: Meghan Dougherty
 * program name: ftServe.c or client(executable)
 * Course: CS372 Intro to Computer Networks
 * Last Modified: 12/1/2019
 *
 * Description: 
 * The server side implementation of project 2, a simple file transfer program.
 * Creates a port via a number supplied through command line input. Then listens
 * on said port for a connection from the client (ftclient.py). A TCP control connection
 * is then created and the server waits for a command from ftclient. Based on the command
 * recieved (-l or -g and the name of a file) the server will then send a directory list 
 * or the contents of a file (respectively). ftserver sends this on a new data connection 
 * created by the server program. After the transfer occurs, both connections are closed
 * and the server waits for a new connection until terminated by SIGINT
 *
 * General Sources:
 * Beej's guide, as supplied in the Project 2 spec file:
 * beej.us/guide/
 * Linux man pages for specifics on various functions (read, write, etc). 
 * man7.org/linux/man-pages/index.html
 *
 * Specific sources are cited next to the pertinent code. 
 */ 
#define _XOPEN_SOURCE 600 //stackoverflow.com/questions/751870/netdb-h-not-linking-properly
			  // Was getting error messages about the compiler not recognizing the
			  // hints struct. Utilized a fix from this source

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

/*createAddress(port number)
 * pre-conditions:
 * 	port number refers to a valid server port number
 * post-conditions:
 * 	returns a filled out addrinfo struct based on the port number
 *
 * Description:
 * * Takes the port number and creates an addrinfo struct. This is to create an addrinfo
 * struct without a specified IP address.
 */ 
struct addrinfo* createAddress(char* portNum)
{
	struct addrinfo *res;
	struct addrinfo hints;
	int status;

	//clear the hints struct.
	memset(&hints, 0, sizeof hints);

	//use either IPv4 or IPv6
	hints.ai_family = AF_INET;
	//TCP connection
	hints.ai_socktype = SOCK_STREAM;
	//auto fill in IP
	hints.ai_flags = AI_PASSIVE;

	//Print an error with related status if something goes wrong
	if((status = getaddrinfo(NULL, portNum, &hints, &res) != 0))
	{
		printf("getaddrinfo: %s\n", gai_strerror(status));

	}
	//return the struct
	return res;

}

/*createAddressIP(ip address, port number)
 * pre-conditions:
 * 	ip address refers to a valid ip address
 * 	port number refers to a valid server port number
 * post-conditions:
 * 	returns a filled out addrinfo struct based on the port number and ip address
 *
 * Description:
 * * Takes the port number and creates an addrinfo struct. This is to create an addrinfo
 * struct with a specified IP address.
 */ 
struct addrinfo* createAddressIP(char* ipAddr, char* portNum)
{
	struct addrinfo *res;
	struct addrinfo hints;
	int status;

	//clear the hints struct.
	memset(&hints, 0, sizeof hints);

	//use either IPv4 or IPv6
	hints.ai_family = AF_INET;
	//We're creating a TCP stream socket
	hints.ai_socktype = SOCK_STREAM;

	//Print an error with related status if something goes wrong
	if((status = getaddrinfo(ipAddr, portNum, &hints, &res) != 0))
	{
		printf("getaddrinfo: %s\n", gai_strerror(status));

	}
	//return the struct
	return res;

}

/*createSocket(addrinfo)
 * pre-conditions:
 * 	addrinfo is a valid and filled out addrinfo struct
 * post-conditions:
 * 	returns an unbound socket.
 *
 * Description:
 * 	Uses a supplied addrinfo struct to create a new, unbound, unconnected socket.
 */ 
int createSocket(struct addrinfo *res)
{
	//create the socket. 
	int sockfd;
	if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))== -1)
	{
		//if there was an error,report it and quit. 
		printf("error creating socket\n");
		exit(1);
	}
	//return the created socket
	return sockfd;
}

/*connectSocket(socket, addrinfo)
 * pre-conditions:
 * 	addrinfo is a valid and filled out addrinfo struct
 * 	socket is a valid socket
 * post-conditions:
 * 	returns a connected socket.
 *
 * Description:
 * 	Uses a supplied addrinfo struct to create a connected socket.
 */ 

void connectSocket(int sockfd, struct addrinfo *res)
{
	//connect the socket. 
	int status=0;
	if((status = connect(sockfd, res->ai_addr, res->ai_addrlen))==-1)
	{
		//if an error occured, report it and quit.
		printf("error connecting socket\n");
		exit(1);
	}
}
/*bindSocket(socket, addrinfo)
 * pre-conditions:
 * 	addrinfo is a valid and filled out addrinfo struct
 * 	socket is a connected socket
 * post-conditions:
 * 	returns a bound socket.
 *
 * Description:
 * 	Uses a supplied addrinfo struct to bind a connected socket.
 */ 
int bindSocket(int sockfd, struct addrinfo * res)
{
	//bind the socket
	if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
	{
		//if there is an error, close the socket, report it and quit.
		close(sockfd);
		perror("error binding socket\n");
		exit(1);
	}
}
/*initArray(size)
 * pre-conditions: 
 * 	size is a valid, non negative integer
 * post-conditions
 * 	returns an initialized array of c strings
 * description:
 * 	Creates and allocates an array of pointers of N size to be used as an array of strings
 */ 
char ** initArray(int size)
{
	//allocate memory for the the array.
	char ** ary = malloc(size*sizeof(char*));
	//allocate memory for each array member.
	for(int i=0; i< size; i++)
	{
		ary[i] = malloc(100*sizeof(char));
		memset(ary[i],0,sizeof(ary[i]));
	}
	//return the array.
	return ary;
}
/*freeArray(array)
 * pre-conditions: 
 * 	array is a valid, array of pointers to strings
 * post-conditions
 * 	memory allocated to array is freed.
 * description:
 *	frees the memory allocated to the specified array of pointers. 
 */ 
void freeArray(char** ary)
{
	//free each member of the array
	for(int i=0; i< sizeof(ary); i++)
	{
		free(ary[i]);
		ary[i] = NULL;
	}
	//free the array
	free(ary);
	ary=NULL;
}
/*getDir(array)
 * pre-conditions: 
 * 	array is a valid, array of pointers to strings
 * post-conditions
 * 	the array is now filled with file names from the opened directory.
 * 	returns the number of files in the directory.
 * description:
 *	opens the directory the ftserver executable is located in. copies each file name
 *	into the files array.  
 * 
 * 	The structure for this function was referenced from here:
 * 	
 * 	gnu.org/sofware/libc/manual/html_node/Simple-Directory-Lister.html#Simple-Directory-Lister
 */ 
int getDir(char ** files)
{
	//create a pointer to a directory and a direct struct to read the entries
	DIR* d;
	struct dirent * dir; 

	//open the current directory
	d = opendir("./");

	int i =0;

	//if the directory was valid.
	if(d)
	{
		//copy all of the entry's names into the files array.
		while((dir=readdir(d))!=NULL)
		{
			strcpy(files[i], dir->d_name);
			i++;
		
		}
		//close the directory
		closedir(d);
	}
	//if there was an error, print it.
	else
		printf("Error opening directory");
	return i;
}
/*sendDir(ip address, port number, files array, size)
 * pre-conditions: 
 *	ip address is a valid address from the connected client
 *	port number is a valid port number
 *	files array is a valid array filled with the contents of a directory
 *	size is the number of files in a directory
 * post-conditions
 * 	the files array is completely sent to the connected client
 * 	the created socket is closed and the res struct is free.
 * description:
 *	creates a new data connection with the client based on the provided ip address
 *	and port number. Then the socket is connected and the function sends the contents
 *	of the files array. After this, a done messgae is sent and the socket is closed.
 * 	
 */ 


void sendDir( char* ipAddr, char* port, char** files, int size)
{
	 
	sleep(2);
	// create a done message to be sent when transfer is complete.
	char * doneMsg = "Done";
	int new_sock;
	//create an addrinfo struct with a specific IP address.
	struct addrinfo *res = createAddressIP(ipAddr, port);
	//create and connect a new socket. 
	new_sock = createSocket(res);
	connectSocket(new_sock, res);
	//send the contents of the files array
	for(int i=0; i<size; i++)
	{
		
		send(new_sock, files[i], 100,0);
	}
	//send the done message
	send(new_sock, doneMsg, 4, 0);
	//close the socket.
	close(new_sock);
	freeaddrinfo(res);


}
/*verifyFile(files array, size, file name)
 * pre-conditions: 
 *	files array is a valid array filled with the contents of a directory
 *	size is the number of files in a directory
 *	file name is a string of characters denoting a file name.
 * post-conditions
 	the isHere variable indicates whether or not the file is located in the
	supplied file array
	returns isHere
 * description:
 *	validates that a client supplied file name is located in the server
 *	side directory.
 * 	
 */ 
int verifyFile(char ** files, int size, char * fileName)
{
	
	//create the variable to hold the result.
	int isHere=0;
	//search for the file. 
	for(int i=0; i< size; i++)
	{
		
		//if found, set the isHere variable to 1
		if(strcmp(files[i],fileName)==0)
			isHere = 1;
	}
	
	//return the result.
	return isHere;
}	
/*sendFile(ip address, port number, files array, size)
 * pre-conditions: 
 *	ip address is a valid address from the connected client
 *	port number is a valid port numbe
 *	file name is a valid  file name. 
 * post-conditions
 * 	the contents of the supplied file (if found) is sent to the client
 * 	the created socket is closed.
 * description:
 *	creates a new data connection with the client based on the provided ip address
 *	and port number. Then the socket is connected and the function sends the contents
 *	of the files array. After this, a done messgae is sent and the socket is closed.
 * 	
 */ 
void sendFile(char * ipAddr, char* port, char * fileName)
{
	sleep(2);
	//create addrinfo struct and other necessary variables.
	struct addrinfo *res;
	int new_sock;
	char buffer[100]; // transfer buffer.
	char* doneMsg = "__done__";//done message to indicate transfer is complete

	//create and connect a new socket based on the supplied ip address and port.
	res = createAddressIP(ipAddr, port);
	new_sock = createSocket(res);
	connectSocket(new_sock, res);

	//clear out the buffer.
	memset(buffer,'\0',sizeof(buffer));

	//open the file read only.
	int fd = open(fileName, O_RDONLY);
	while(1)
	{
		//set the bytes read counter to 0
		int bytesR = 0;
		//read from the file into the buffer.
		bytesR = (read(fd, buffer, sizeof(buffer)));
	
		//if the bytes read is 0, we've reached the end of the file.
		if(bytesR==0)
			break;
		//if bytes read is less than 0, there is an error. Report it and return.
		if(bytesR <0)
		{
			printf("Cannot Read File\n");
			return;
		}
		
		//create a temporary buffer to send the data to the client.
		char * temp = buffer;

		//Send the read bytes to the client.
		//keeps sending until all information is sent. 
		while(bytesR > 0)
		{
			//send the information to the client and keep track of what was sent.
			int bytesW = send(new_sock, temp, sizeof(buffer),0);
			//if the bytes written is less than 0, there was an error. Report and return.
			if(bytesW <0)
			{
				printf("Send Error!");
				return;
			}
			//subtract the bytes written from the bytes read.
			bytesR -= bytesW;
			//update where we are in the temp buffer to continue reading.
			temp += bytesW;
		}
		//clear out the buffer for the next round of reading.
		memset(buffer,'\0',sizeof(buffer));
	}
	//send the done message to the client to indicate transfer is complete.
	send(new_sock, doneMsg, sizeof(doneMsg),0);
	//close the socket.
	close(new_sock);
	//print to the terminal that transfer is complete.
	printf("Transfer Complete\n");
	//free the addrinfo struct.
	freeaddrinfo(res);
}
/* newConnection(socket)
 * pre-conditions: 
 *	socket is a valid, connected and bound socket.
 * post-conditions
 *	commands based on client communication are executed.
 * description:
 * 	handles a new connection with a client. retrieves the commands specified
 * 	in the client side command line arguments (IP address, data port, etc).
 * 	Executes the necessary functions based on the desired transfer (-l or -g)
 * 	and prints error messages if any occur.
 * 	
 */ 
void newConnection(int sockfd)
{
	//create messages to send to the client. 
	char * posMsg = "good";
	char* negMsg = "bad";

	//initialize buffers to hold the information recieved from the client.
	char port[100];
	char command[2];
	char ipAddr[100];
	
	memset(port,'\0',sizeof(port));
	memset(command, '\0', sizeof(command));
	memset(ipAddr, '\0', sizeof(ipAddr));
	//initialize the files array of strings and buffers for the file name.
	char ** fileList;
	char  fileName[100];
	char finFileName[100];

	memset(fileName,'\0',sizeof(fileName));
	memset(finFileName,'\0', sizeof(finFileName));
	//recieve the port number and send a confirmation
	recv(sockfd, port, sizeof(port),0);
	send(sockfd, posMsg, 4,0);

	//recieve the command and send a confirmation
	
	recv(sockfd, command, sizeof(command), 0);
	send(sockfd, posMsg, 4,0);

	//recieve the ip address.
	recv(sockfd, ipAddr, sizeof(ipAddr),0);

	//print to terminal that we are starting a new connection with a client.
	printf("rnew connection from %s\n", ipAddr);
	//If the client sent the directory list command.
	if(strcmp(command,"-l") == 0)
	{
		//print that we are sending the directory.
		printf("Sending directory\n");
		//send a confirmation to the client.
		send(sockfd, posMsg, 4,0);
		//initialize an array.
		fileList = initArray(100);
		//get the number of files and fill in the files array
		int numFiles = getDir(fileList);
		//send the directory to the client.
		sendDir(ipAddr, port, fileList, numFiles);
		printf("Transfer Complete\n");
		//free the files array
		freeArray(fileList);
	}
	//if the client wants a specific file.
	else if(strcmp(command,"-g")==0)
	{
		//send a confirmation message.
		send(sockfd, posMsg, 4,0);
		//clear the fileName buffer and recieve the file name 
		//from the client.
		memset(fileName, 0, sizeof(fileName));
		recv(sockfd, fileName, sizeof(fileName)-1,0);
		//print the file name
		printf("File %s requested\n",fileName);

		//initialize the file list
		fileList = initArray(1000);
		//populate the file list. This is used to verify the
		//specified file exists.
		int numFiles = getDir(fileList);
		int isHere = verifyFile(fileList, numFiles, fileName);
		
		//if the file is found in the directory.
		if(isHere==1)
		{
			
			//print that we are sending the file.
			printf("sending file %s\n", fileName);
			//send a confirmation message.
			send(sockfd, posMsg, 4,0);
		
			//add ./ to the beginning of the file name. 
			memset(finFileName,0,sizeof(finFileName));
			strcpy(finFileName,"./");
			strcat(finFileName,fileName);
			
			//send the new formatted fileName to the client.
			sendFile(ipAddr, port, finFileName);
			 
		}
		//if the file does not exist, print an error message
		//and send a bad message to the client.
		else
		{
			printf("file not found\n");
			send(sockfd, negMsg, 3,0);
		}
		//free the files array.
		freeArray(fileList);
	}
	//If, somehow, the command wasn't -f or -g.
	else
	{
		//send a negative message and print an error message.
		send(sockfd, negMsg, 3,0);
		printf("Invalid Command");
	}



}
/* listenSock(socket)
 * pre-conditions: 
 *	socket is a valid, connected and bound socket.
 * post-conditions
 *	the specified socket is now passive and listening for new connections
 * description:
 *	sets up the specified socket to listen to up to 5 new connections. 	
 */ 
void listenSock(int sockfd)
{	
	//Listen for new connections, if there is an error, print 
	//a message and exit.
	if(listen(sockfd, 5) == -1)
	{
		close(sockfd);
		perror("error cannot listen");
		exit(1);
	}
}
/* waitConn(socket)
 * pre-conditions: 
 *	socket is a valid, connected and bound socket.
 * post-conditions
 *	the specified socket is closed
 * description:
 *	waits for a new connection on the specified socket. If one is found, 
 *	call the new Connection function to begin the transfer. If not, wait
 *	until one is found. 
 */ 
void waitConn(int sockfd)
{
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	int new_fd;
	while(1)
	{
		//accept a new connection on the socket.
		sin_size = sizeof(their_addr);
		new_fd = accept(sockfd,(struct sockaddr *) &their_addr, &sin_size);

		//if no connection is found, loop back to the top.
		if(new_fd == -1)
		{
			continue;
		}
	
		// If a new connection is found, implement the transfer
		newConnection(new_fd);
		//close the connection
		close(new_fd);
	}
}
/*Main:

 * pre-conditions:
 * 	One command line input, a valid port number
 * post-conditions:
 * 	none
 * Description:
 * 	main driver function for the server side file transfer program. creates a socket
 * 	and listens for new connections. 
 */ 
int main(int argc, char* argv[])
{
	//make sure the user provided a port number. 
	if(argc != 2)
	{
		printf("Please include port num");
		exit(1);
	}
	int port = strtol(argv[1],NULL,10);	
	if(port > 665535 || port < 1024)
	{
		printf("Please enter a valid port num\n");
		exit(1);
	}
	printf("Server listening on port %s\n", argv[1]);
	
	//create the port, then bind it.  
	struct addrinfo* res = createAddress(argv[1]);
	int sockfd = createSocket(res);
	bindSocket(sockfd, res);

	//listen for new connections.
	listenSock(sockfd);
	waitConn(sockfd);

	//free the addrinfo struct and close the socket. 
	freeaddrinfo(res);
	close(sockfd);
	return 0;
}
