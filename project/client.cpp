#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <fstream> 
#include <list>
#include <vector>

#include <bits/stdc++.h>

#define LOCALHOST "127.0.0.1" // Host address
#define TCPMPORT "25606" 

using namespace std;
//From Beej 
//#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main()
{
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	//char s[INET6_ADDRSTRLEN];
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, TCPMPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	// 		s, sizeof s);
	// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	int count = 0;
	int maxTries = 3;
    while(1){
        cout<<"Please enter the username: ";
        char username[50];
        cin.getline(username,50); //take the message as input
        //int i, j, length,key;
        cout<<"Please enter the password: ";
        char password[50];
        cin.getline(password,50);
		char combinedInput[100];
		strcpy(combinedInput,username);
        strcat(combinedInput,",");
        strcat(combinedInput,password);
        strcat(combinedInput,"\0");

		int sentvalue = strlen(combinedInput);
		if(strlen(combinedInput)==0){
			combinedInput[0] = '\n';
			sentvalue = 1;
			printf("HERE\n");
		}

        if (send(sockfd, combinedInput, sentvalue, 0) == -1){
            perror("send");
        }
		printf("%d",strlen(combinedInput));
		printf("sent %s to server",combinedInput);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        printf("client: received '%s'\n",buf);

        if(strcmp(buf,"exit")==0){
            close(sockfd);
            exit(0);
            return 0;
        }
		if(buf[0]=='0'){
            count++;
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("Authentication failed: Username Does not exist \n");
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='1'){
            count++;
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("Authentication failed: Password does not match \n");
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='2'){
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
        	printf("Authentication is successful \n");
			while(1){
				cout<<"Please enter the course code to query: ";
				char courseNum[50];
				cin.getline(courseNum,50); //take the message as input
				cout<<"Please enter the category (Credit / Professor / Days / CourseName): ";
				char category[50];
				cin.getline(category,50);
				// string courseName = courseNum.substr(0,2);
				string courseName;
				courseName.push_back(courseNum[0]);
				courseName.push_back(courseNum[1]);
				char courseLookup[100];
				strcpy(courseLookup,courseNum);
				strcat(courseLookup,",");
				strcat(courseLookup,category);
				strcat(courseLookup,"\0");
				cout<<courseLookup<<endl;
				if (send(sockfd, courseLookup, strlen(courseLookup), 0) == -1){
					perror("send");
				}
				printf("%d",strlen(courseLookup));
				printf("sent %s to server",courseLookup);
				memset(buf, 0, sizeof buf);
				if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
					perror("recv");
					exit(1);
				}

				buf[numbytes] = '\0';
				cout<<"received:";
				cout<<buf<<endl;

			}
        }
        else{
            perror("auth");
            exit(1);
        }

		if(count == maxTries){
			break;
		}

    }
    

	close(sockfd);

	return 0;
}
