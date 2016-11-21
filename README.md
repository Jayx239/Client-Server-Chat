# ECEC353-Assignment3 
Drexel University ECEC-353 Systems Programming Assignment 3 Summer 2016
Created by: Jason Gallagher

Client Server Chat Group Messenger
This application is a c implementation of a client chat server utilizing shared memory and multithreading in a linux environment.

To build: Run the makefile: make

To run:
There must be atleast 2 consoles open, one for the server application and an additional console for each client.

1) start the server:
./server

2) Start the client
./client [UID - 5 character limit] [groupID - integer value]

3) Chat
To send a message, simply type the message and press enter to send.

Commands:
Direct Message: -dm
Exit: -e
