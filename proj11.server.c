/**
 * Project 11 - Server
 * 
 * Brenden Hein
 */

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define BSIZE 256

/*STRUCTS*/

/*Main entry point for process*/
int main()
{
    // Creates variable to faciliate client/server interactions
    int sd_server, sd_client;
    struct sockaddr_in server, client;
    socklen_t size_client;

    // Gets a socket
    sd_server = socket( PF_INET, SOCK_STREAM, 0 );
    if (sd_server == -1)
    {
        cout << "Error creating socket" << endl;
        exit(-1);
    }
  
    // Adds to the server struct to pass on
    bzero(&server, sizeof(server));
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(0);

    // socket number, server data to pass on, how many bytes in server struct
    int bindFlag = bind(sd_server, (struct sockaddr *) &server, sizeof(server));
    if (bindFlag == -1)
    {
        cout << "Error on binding server to socket" << endl;
        perror("bind");
        exit(-2);
    }

    // Gets the hostname
    char hostname[256];
    gethostname(hostname, 256);

    // Gets the port
    socklen_t len = sizeof(server);
    int succ = getsockname(sd_server, (struct sockaddr *)&server, &len);
    if (succ == -1)
    {
        cout << "Error on getting port number" << endl;
        exit(-7);
    }

    cout << hostname << " " << ntohs(server.sin_port) << endl;

    // Listens for data to come through socket (blocked till client connects)
    int listenFlag = listen(sd_server, 5);
    if (listenFlag == -1)
    {
        cout << "Error on server listening for message" << endl;
        perror("listen");
        exit(-3);
    }

    // We got a connection, so lets accept it
    size_client = sizeof( client );
    sd_client = accept( sd_server, (struct sockaddr *) &client, &size_client );
    if (sd_client == -1)
    {
        cout << "Error on accepting message from client" << endl;
        perror("accept");
        exit(-4);
    }

    /* We have a connection now */

    while (1)
    {
        // Recieves GET request from client (or a quit)
        char fileBuffer[BSIZE];
        bzero(fileBuffer, BSIZE);
        int recCount = recv(sd_client, fileBuffer, BSIZE, 0);
        if (recCount == -1)
        {
            cout << "Error on recieving the response" << endl;
            perror("recv");
            exit(-4);
        }

        if (strcmp(fileBuffer, "quit") == 0) // the user wants to quit
        {
            break;
        }

        // Removes the F from the buffer, now that we know we aren't quitting
        char *buffer = fileBuffer;
        buffer++;

        ifstream readFile(buffer); // create a file stream object
        if (readFile.fail()) // the file failed to open
        {
            cout << buffer << " does not exist" << endl;

            // Sends flag of '0' to inform client that it has not found the file
            int sendCount = send(sd_client, "0", 1, 0);
            if (sendCount == -1)
            {
                cout << "Error on sending file not found message" << endl;
                perror("send");
                exit(-6);
            }
        }

        else
        { 
            // Sends flag of '1' to inform client that it has successfully found the file
            int sendCount = send(sd_client, "1", 1, 0);
            if (sendCount == -1)
            {
                cout << "Error on sending file not found message" << endl;
                perror("send");
                exit(-6);
            }

            // loops through the lines of the file
            while (readFile)
            {
                char fileBuffer[BSIZE];
                bzero(fileBuffer, BSIZE);

                readFile.read(fileBuffer, BSIZE-1);
                size_t cnt = readFile.gcount();

                if (!cnt) // There aren't anymore bytes to be read
                {
                    break;
                }

                // Sends the data to the client
                int sendCount = send(sd_client, fileBuffer, cnt, 0);
                if (sendCount == -1)
                {
                    cout << "Error on sending request" << endl;
                    perror("send");
                    exit(-5);
                }
            }
        
            readFile.close(); // closes the file
            break;
        }
    }

    // closes the connection
    close(sd_server);
}
