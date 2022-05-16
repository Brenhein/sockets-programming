/**
 * Project 11 - Client
 * 
 * Brenden Hein
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;

#define BSIZE 256

const std::string WHITESPACE = " \n\r\t\f\v";

/* This function trims the leading and trailing whitespaces from a string 
* \param str The string to trim
* \returns The newly trimmed string
*/
string trim(const string& str)
{
    // left trim
    size_t start = str.find_first_not_of(WHITESPACE);
	string lString = (start == std::string::npos) ? "" : str.substr(start);

    // right trim
    size_t end = lString.find_last_not_of(WHITESPACE);
	string trimmedString = (end == std::string::npos) ? "" : lString.substr(0, end + 1);

    return trimmedString;
}


/*Main entry point for process*/
int main(int argc, char** argv)
{
    // Error handling for improper amount of args
    if (argc != 3)
    {
        cout << "User did not enter the correct number of arguements\n";
        exit(-1);
    }

    // Gets the arguemtn data (could still be wrong but that'll be taken
    // care of elsewhere in the program)
    char* hostname = argv[1];
    int port = atoi(argv[2]);

    // Creates the socket
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        cout << "Error on creating socket" << endl;
        exit(-1);
    }

    // Tries to get the host
    struct hostent* server = gethostbyname(hostname);
    if (server == NULL)
    {
        cout << hostname << " is not a valid host" << endl;
        exit(-2);
    }

    // Creates struct for the client data 
    struct sockaddr_in client;
    client.sin_addr.s_addr = htonl(INADDR_ANY);
    client.sin_family = AF_INET;
    client.sin_port = htons(port);

    int connStat = connect(sd, (struct sockaddr *)&client, sizeof(client)); // Connects to the server
    if (connStat == -1)
    {
        cout << "Error on connecting to the server" << endl;
        perror("connect");
        exit(-6);
    }

    // This loop handles user input, i.e. trying to request the server to write to the clients file
    string command;
    cout << "\nEnter a command: ";
    while (getline(cin, command))
    {
        // Trims and outputs the user command
        command = trim(command);
        cout << "User command: " << command << endl; // displays the user input

        // User entered quit
        if (command == "quit")
        {
            // Lets let the server know we wanna quit so the connection doesn't just break
            int quitCnt = send(sd, "quit", 5, 0);
            if (quitCnt == -1)
            {
                cout << "Error on sending quit request" << endl;
                perror("send");
                exit(-3);
            }
            cout << endl;
            break;
        }

        string remote; // The remote filename
        string local; // The local filename

        /* 
         * Did the user entered a valid get request 
         */

        // Makes sure the first 3 characters are 'get'
        if (command.substr(0, 3) == "get" && (command[3] == ' ' || command[3] == '\t'))
        {
            // Removes the get and trims the whitespaces on the end of the 2 files
            command = trim(command.substr(3)); // trims the white space between to get and remote file
            
            // Gets the 2 files by looping through
            bool second = false;
            for (auto chr : command)
            {
                if ((chr == ' ' || chr == '\t') && !second) // we have reached the space between the 2 files
                {
                    second = true;
                }

                else if ((chr != ' ' && chr != '\t') && !second) // we are adding chars to our remote file
                {
                    remote += chr;
                }

                else if ((chr != ' ' && chr != '\t') && second) // we are adding chars to the local file
                {
                    local += chr;
                }
            }
        }

        // We have a valid request to send to the server
        if (remote != "" && local != "")
        {
            // Lets send the GET request
            remote.insert(0, "F"); // Adds a tag so that the server allows 'quit' as a filename
            int sendCount = send(sd, remote.c_str(), remote.size(), 0);
            if (sendCount == -1)
            {
                cout << "Error on sending request" << endl;
                perror("send");
                exit(-3);
            }

            // Lets the client know if the file exists
            char flagBuffer[2] = {'\0', '\0'};
            int flagCnt = recv(sd, flagBuffer, 1, 0);
            if (flagCnt == -1)
            {
                cout << "Error on recieving the response" << endl;
                perror("recv");
                exit(-5);
            }

            // Are we reading from a server file or is there no file to read from
            if (strcmp(flagBuffer, "0") == 0)
            {
                cout << "File was not found on the server.  Try again\n" << endl;
            }

            /*
             * Writes to the file
             */
        
            else
            {
                ofstream writeFile(local); // Creates/opens the file to write to

                int recCount = 1;
                while (recCount > 0)
                {
                    // Now lets get the data and write it out to our file
                    char buffer[BSIZE];
                    bzero(buffer, BSIZE);
                    recCount = recv(sd, buffer, BSIZE-1, 0); 
                    if (recCount == -1)
                    {
                        cout << "Error on recieving the response" << endl;
                        perror("recv");
                        exit(-4);
                    }

                    writeFile << buffer; // writes to the file
                }

                writeFile.close(); // closes the file
                cout << endl;
                break;
            }
        }
        
        else // handles incorrect input
        {
            cout << "Invalid Command\n\n";
        }

        cout << "Enter a command: ";
    }

    close(sd); // closes the socket
}