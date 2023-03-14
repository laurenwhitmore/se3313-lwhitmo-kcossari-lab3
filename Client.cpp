#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;

// This thread handles the connection to the server
class ClientThread : public Thread
{
private:
	// Reference to our connected sock
	Socket& sock;
	// array to send to the server
	ByteArray arr;
	//will be user input
	std::string userInput; 

	//Reference to our termination enabler
	bool& exit;	//if true, should break out of main function and start terminating threads (clode client)
						
	bool connect = false;

public:
	ClientThread(Socket& sock, bool&exit): sock(sock), exit(exit){} //constructor
	~ClientThread(){} //destructor
	virtual long ThreadMain()
	{
		while(1)
		{
			try{//try to establish connection
			
				sock.Open();	//try to open
				connect = true; //once it is connected 
				std::cout << "Connected!" << std::endl;
			}catch(...){}//connection failed
			if (exit || connect){
				//if exit requested or connection was established leave the loop
				break;
			}
		}
		while (exit == false)
		{
			std::cout << "Please input a string to capitalize: ";
			std::cout.flush();	//clear buffer
			// Get the data
			std::getline(std::cin, userInput);
			arr = ByteArray(userInput);	//convert to a byte array						
				if (userInput == "done") //user wants to exit if true
				{
					std::cout << "See ya!" << std::endl;
					exit = true;
					break;
				}
				// Write to the server
				int a = sock.Write(arr);

				if (a > 0)	//if buffer has something in it then server responded
				{
					// Get the response
					sock.Read(arr);
					userInput = arr.ToString();	//convert arr of bytes back into string format
					std::cout << userInput << std::endl << std::endl;
				} 
				
				else if (a < 0)	//if buffer is empty, server failed to respond
				{
					std::cout << "Server unresponive, client will now exit." << std::endl;
					exit = true;	//break out of threadMain and start terminating threads
					break;
				}				
			
		}

		return 0;
	}
};

int main(void)
{
	// Create our sock
	Socket sock("127.0.0.1", 1000);
	bool exit = false; //do not exit
	ClientThread clientThread(sock, exit);	//create client thread
	while(!exit)
	{
		sleep(1);
	}
	sock.Close();	//close the connection
	sleep(3);	//give time to exit
	std::cout << "Client Terminated."<<std::endl;
	return 0;
}
