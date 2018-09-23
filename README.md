INTRODUCTION
------------
	This project on UDP Socket programming consists of a client and the server. The client interacts 
	with the user, gets the user command and sends it to the server. The main purpose of the server 
	is to collect the data based on the user's input and send back to the client. In this project, 
	a reliable connectionless file transmission model is implemented using User Datagram Protocol 
	(UDP). This project is part of Network Systems (CSCI 5273) course.

BUILD and RUN STEPS
-------------------
	This project only supports Linux development environment. The following section covers in detail on 
	how to build and run the client and server module.

	Note:- The build system for this project uses GNU MAKE utility.

	PROJECT BUILD STEPS
	===================

	1.) Go to the project folder, cd [FolderName]/
	
	2.) Run make (This command compiles both client and server present in a different folder 
	    and generates an respective executable)

	RUN STEPS
	=========

	For SERVER module:
	
		1.) ./server [PORT]

		Note:- Use an unreserved port > 5000

	For CLIENT module:

		1.) ./client [IP ADDRESS] [PORT]

		Note:- Use the address of the server and same port number used in server.

IMPLEMENTATION SUMMARY
----------------------
	The client side implements an user interaction module and publishes the supported commands. 
	The following section covers in detail on how client and server interact on each user command.

	The supported user command includes,

	get [filename]		-	This command is used to get the specified file from server to client
	==============
		
		SERVER side Implementation
		
			1.) On recieving the 'get' request from client, the server first checks if 
			    the filename is specified or not. If filename is null, then server does 
			    not send anything back to client
			2.) Then it checks if the specified file is present and have appropriate read 
			    permission. If the file is not present or does not have appropriate 
			    permission, then server does not send anything back to the client
			3.) If the specified file is valid, then it sets a timeout of 2 seconds for 
			    the recieve call on the server socket
			4.) After this it opens the specified file, gets the file size and calculates 
			    the number of frames required to send the file
			5.) First it sends the total number of frames and then checks if the recieved 
			    acknowledegement matches to the total number of frames
			6.) Finally it sends all the frames sequentially and check the recieved ack. If 
			    ack does not match then it keeps resending the frames till the ack matches

		CLIENT side Implementation

			1.) Server does not transfer any message if the filename is NULL or not present
			    in the directory
			2.) So, it sets a timeout of 2 seconds for the recieve call on the client packet. If
			    client does not recieve the number of frames in 2 seconds, timeout will occur
			3.) Disable the timeout after receiving the total number of frames
			4.) Recieve all the frames sequentially and then send frame ID as the Ack
			5.) Discard the duplicate frames.
			6.) Write the recieved data frame into a file.

	put [filename]		-	This command is used to transfer the specified file into server.
	==============
	
	The implementation is similiar to 'get' request except here the client will initiate the
	transmission, transfer the file and the server will recieve the file.

	delete [filename]	-	This command is used to delete the specified file in the server side
	=================

		SERVER side Implementation

			1.) On recieving the 'delete' request from client, the server first checks if
			    the filename is specified or not. If filename is NULL, then the server 
			    sends an negative number or zero, to indicate the error
			2.) If the filename is valid, then the file is removed and a positive ack is sent

	ls			-	list all the files present in the server side
	==

		In this case, the server scans all the files and directories in the present folder and transfers
		the entire list to the client.

	exit			-	Close the client and server
	====
