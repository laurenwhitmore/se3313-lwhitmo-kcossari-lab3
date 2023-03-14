#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
// This thread handles each client connection
class SocketThread : public Thread
{
private:
    // Reference to our connected socket
    Socket& sock;
    // The data we are receiving
    ByteArray data;
    std::string userInput;
    bool& exit;

public:
    SocketThread(Socket& socket, bool& exit): sock(sock), exit(exit){}//constructor
    ~SocketThread(){}//destructor
    Socket& GetSocket(){return sock;}
    virtual long ThreadMain()
    {
        while(exit == false)
        {
            try {
                sock.Read(data);
                userInput = data.ToString();
                //convert to uppercase
                for(int i=0; i<userInput.length(); i++){
                    userInput[i]=toupper(userInput[i]);
                }
                data= ByteArray(userInput);
                // Send it back
                sock.Write(data);
            }
            catch (...){
                exit = 1;
            }
        }
        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    bool exit = false;
    std::vector <SocketThread *> threadArr; 
public:
    ServerThread(SocketServer& server): server(server){}
    ~ServerThread()
    {
        //close and delete all threads if the server is stopped
    for (auto thread : threadArr) {
                Socket& closeSocket = thread -> GetSocket();
                closeSocket.Close();    // close the connection 
                delete thread;
            }
            exit = true;   //terminate thread loops 
        }

    virtual long ThreadMain()
    {
         while(exit == false)
        {
            try{
                // Wait for a client socket connection
                Socket* newCon = new Socket(server.Accept());
                // Pass a reference to this pointer into a new socket thread
                Socket& socketRef = *newCon;
                threadArr.push_back(new SocketThread(socketRef, exit));
            }
            catch(...){
                exit = 1;
            }
        }
    }
};


int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to exit server...";
    std::cout.flush();
    SocketServer server(1000);    
    //create server thread
    ServerThread serverThread(server);
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();
    // Shut down and clean up the server
    server.Shutdown();
    return 0;
}
