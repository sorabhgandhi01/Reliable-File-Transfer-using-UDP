INTRODUCTION
------------
	The UDP Socket programming consists of a client and the server. The client interacts with the 
	user, gets the user commands and sends to the server. The main purpose of the server is to collect the 
	data based on the user's input and send back to the client. In this project, a reliable connectionless 
	file transmission model is implemented using User Datagram Protocol (UDP). This project is part of 
	Network Systems (CSCI 5273) course offered at University of colorado, Boulder.

BUILD and RUN STEPS
-------------------
	This project only supports Linux development environment. The following section covers in 
	details on how to build and run the client and server module.

	Note:- The build system for this project uses GNU MAKE utility.

	PROJECT BUILD STEPS
	===================

	1.) Go to the project folder, cd UDP/
	
	2.) Run make (This command compiles both client and server present in a different folder 
	    and generates an respective executable.

	RUN STEPS
	=========

	For SERVER module:
	
		1.) ./server [PORT]

		Note:- Use an unreserved port > 

	For CLIENT module:

		1.) ./client [IP ADDRESS] [PORT]

		Note:- Use the address of the server and same port number used in server.

IMPLEMENTATION SUMMARY
----------------------
	The client side implements an user interaction module and publishes the supported commands. 
	The following section covers in detail on how client and server interact on each user command.

	The supported user command includes,

	get [filename]	-	This command is used to get the specified file from server to client
	==============
		
		SERVER side Implementation
		
			1.) On recieving the 'get' request from client, the server first checks if 
			    the file name is specified or not. If filename is null, then server does 
			    not send anything back to client
			2.) Then it checks if the specified file is present and have appropriate read 
			    permission. If the file is not present or does not have appropriate 
			    permission, then server does not send anything back to the client
			3.) Set a timeout of 2 seconds for the recieve call on the server socket.
			4.) After this it opens the file, gets the file size and calculated the number 
			    of frames required to send the file
			5.) First it sends the total number of frames and then check if the recieved 
			    acknowledegement match to the total number of frames.
			6.) Finally send all the frames sequentially and check the recieved ack. If 
			    ack does not match then Keep resending the frames till the ack matches.

		CLIENT side Implementation

			
