#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <errno.h>
#include <fstream>

using namespace std;

int main(int argc, char *argv[])
{

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0)
    {
        cout<<"unable to create socket at client side"<<endl;
        return -1;
    }

    int port = atoi(argv[2]);
    string ipAddress = argv[1];

    struct sockaddr_in clientstruct;
     bzero((char*)&clientstruct,sizeof(clientstruct));
    clientstruct.sin_family = AF_INET;
    clientstruct.sin_port = htons(port);
    inet_pton(AF_INET,ipAddress.c_str() , &clientstruct.sin_addr);


    int ccon = connect(cfd, (struct sockaddr*)&clientstruct, sizeof(clientstruct));
    if (ccon <0)
    {
        cout<<"unable to establish connection to server"<<endl;
        return -2;
    }

   
    int ret;
    string command;
    char outbuf[1500];
    while(1)
    {
                   
        cout<<"Enter Command:"<<endl;
        getline(cin,command);
		if(command[0]=='T')
		{
		   close(cfd);
			break;
		}
		
		if(command[0]!= 's')
		{
			cout<<"Invalid command"<<endl;
			continue;
		}
		
        string filename=command.substr(command.find(' ')+1,command.find('\0'));
		
            
		fstream newfile;
		newfile.open(filename.c_str(),ios::in);
		string buf = "";
		if (newfile.is_open())
		{
			string tp;
			while(getline(newfile, tp))
			{
				buf += tp;
				buf += '\n';
			}
		}
		newfile.close(); 
		
		char *fcontent = new char[buf.length()+1];
		
		strcpy(fcontent, buf.c_str());

		
		ret = send(cfd, fcontent, strlen(fcontent), 0);
        if (ret < 0)
		{
			cout<<"ERROR "<<endl;
			break;
		}
		
		
        int nbytes;
	bzero (outbuf, 1500);
	string output = "";
	while(1)
        {
			
	    if(0 < recv(cfd, outbuf, sizeof(outbuf),0))
	    {
	        string temp(outbuf);
		string s1 = "Program";
		string s2 = "Process";
		cout<<outbuf<<endl;
		if(temp.substr(0, s1.length()).compare(s1) == 0)
			break;
		if(temp.substr(0, s2.length()).compare(s2) == 0)
		        break;
	        bzero (outbuf, 1500);
	    }
	    else
	    	break;
        }
    }
    return 0;
}
