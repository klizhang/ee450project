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
	// unsigned int clientPort;
	// struct sockaddr_in clinetAddress;
	// bzero(&clinetAddress, sizeof(clinetAddress));
	// socklen_t len = sizeof(clinetAddress);
	// getsockname(sockfd, (struct sockaddr *) &clinetAddress, &len);
	// clientPort = ntohs(clinetAddress.sin_port);
	// printf("Client's dynamic port number : %u\n", clientPort);
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(sockfd, (struct sockaddr *)&sin, &len) == -1)
		perror("getsockname");
	else
		printf("port number in use: %d\n", ntohs(sin.sin_port));
	// inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	// 		s, sizeof s);
	// printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	printf("The client is up and running.\n");
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
		// if(strlen(combinedInput)==0){
		// 	combinedInput[0] = '\n';
		// 	sentvalue = 1;
		// 	printf("HERE\n");
		// }

        if (send(sockfd, combinedInput, sentvalue, 0) == -1){
            perror("send");
        }
		// printf("%d",strlen(combinedInput));
		// printf("sent %s to server",combinedInput);
		printf("%s sent an authentication request to the main server.\n",username);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        //printf("client: received '%s'\n",buf);

        // if(strcmp(buf,"exit")==0){
        //     close(sockfd);
        //     exit(0);
        //     return 0;
        // }
		if(buf[0]=='0'){
            count++;
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username Does not exist \n",username,ntohs(sin.sin_port));
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='1'){
            count++;
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match \n",username,ntohs(sin.sin_port));
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='2'){
            //printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("%s received the result of authentication using TCP over port %d. Authentication is successful \n",username,ntohs(sin.sin_port));
			while(1){
				cout<<"Please enter the course code to query: ";
				char courseNum[50];
				cin.getline(courseNum,50); //take the message as input
				char courses[50];
				strcpy(courses,courseNum);
				// char * token = strtok(courses, ",");
				// char * courseNum = token;
				// token = strtok(NULL, ",");
				// char * category = token;
				// vector<string> array;
				// char * list[10];
				list<char*> list;
				char * token = strtok(courses, " ");
				while(token != NULL){
					printf("%s\n",token);
					list.push_back(token);
					token = strtok(NULL, " ");
				}
				if(list.size()>1){
					long unsigned int size = list.size(); 
					char multicourse[50];
					memset(multicourse, 0, sizeof multicourse);
					for (std::list<char*>::iterator it = list.begin(); it != list.end(); ++it){
						strcat(multicourse,*it);
						strcat(multicourse,",");
						}
					multicourse[strlen(multicourse)-1]='\0';

					
					cout<<multicourse<<endl;
					cout<<size<<endl;
					if(size<10){
						if (send(sockfd, multicourse, strlen(multicourse), 0) == -1){
							perror("send");
						}
						//printf("%d",strlen(courseLookup));
						// printf("sent %s to server",courseLookup);
						printf("%s sent a request with multiple CourseCode to the main server\n",username);
						unsigned int i =0;
						printf("CourseCode: Credits, Professor, Days, Course Name\n");
						while(i<list.size()){
							memset(buf, 0, sizeof buf);
							cout<<"before"<<endl;
							if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
								perror("recv");
								exit(1);
							}
							cout<<"after"<<endl;
							buf[numbytes] = '\0';
							// if(strcmp(buf,"Wrong input,Wrong input,Wrong input,Wrong input")==0){
							// 	printf("Course not found\n");
							// }
							char check[10] = "Wrong";
							if(strstr(buf,check)!=NULL){
								char *ptr = strtok(buf,":");
								printf("%s: Course not found\n",ptr);
							}
							else{
								cout<<"aaa"<<endl;
								printf("%s\n",buf);
							}
							i++;
						}
					}
					else{
						cout<<"Error: too many inputs (input must be less than 10)"<<endl;
					}
					
				}
				else{
					cout<<"Please enter the category (Credit / Professor / Days / CourseName): ";
					char category[50];
					cin.getline(category,50);
					char courseLookup[100];
					memset(courseLookup,0,sizeof courseLookup);
					strcat(courseLookup,"0");
					strcat(courseLookup,courseNum);
					strcat(courseLookup,",");
					strcat(courseLookup,category);
					strcat(courseLookup,"\0");
					//cout<<courseLookup<<endl;
					if (send(sockfd, courseLookup, strlen(courseLookup), 0) == -1){
						perror("send");
					}
					//printf("%d",strlen(courseLookup));
					// printf("sent %s to server",courseLookup);
					printf("%s sent a request to the main server\n",username);
					memset(buf, 0, sizeof buf);
					if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
						perror("recv");
						exit(1);
					}
					buf[numbytes] = '\0';
					// cout<<"received:";
					cout<<buf<<endl;
					printf("The client received the response from the Main server using TCP over port %d.\n",ntohs(sin.sin_port));
					if(strcmp(buf,"Wrong input")==0){
						printf("Didn’t find the course: %s\n\n",courseNum);
					}
					else if(strcmp(buf,"Wrong category")==0){
						printf("Didn’t find the category: %s\n\n",category);
					}
					else{
						printf("The %s of %s is %s.\n\n",category,courseNum,buf);
					}
					printf("-----Start a new request----- \n");
				}

				
			}
        }
        else{
            perror("auth");
            exit(1);
        }

		if(count == maxTries){
			printf("Authentication Failed for 3 attempts. Client will shut down.\n");
			break;
		}

    }
    

	close(sockfd);

	return 0;
}
