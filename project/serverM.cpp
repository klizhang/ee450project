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
#define SERVERPORT "21606"	// the port users will be connecting to
#define CSPORT "22606"
#define EEPORT "23606"
#define MAXBUFLEN 100

using namespace std;

void encrypt(char msg[50]){
//https://www.tutorialspoint.com/cplusplus-program-to-implement-caesar-cypher#
    int key = 4;
    //int length = strlen(msg);
    char ch;
    for(int i = 0; msg[i] != '\0'; ++i) {
        ch = msg[i];
        //encrypt for lowercase letter
        if (ch >= 'a' && ch <= 'z'){
            ch = ch + key;
            if (ch > 'z') {
                ch = ch - 'z' + 'a' - 1;
            }  
            msg[i] = ch;
        }
        //encrypt for uppercase letter
        else if (ch >= 'A' && ch <= 'Z'){
            ch = ch + key;
            if (ch > 'Z'){
                ch = ch - 'Z' + 'A' - 1;
            }
            msg[i] = ch;
        }
        //encrypt for digit
        else if(ch >= '0' && ch <= '9'){
            ch = ch + key;
            if (ch > '9'){
                ch = ch - '9' + '0' - 1;
            }
            msg[i] = ch;
        }
    }
}


int main() {
    
    //SOURCE from beej
    int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(LOCALHOST, SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}
    int count = 0;
    int maxTries = 3;
    char username[50];
    while(count < maxTries){
        cout<<"Please enter the username: ";
        //char username[50];
        cin.getline(username,50); //take the message as input
        //int i, j, length,key;
        encrypt(username);
        cout<<"Please enter the password: ";
        char password[50];
        cin.getline(password,50);
        encrypt(password);
        printf("Encrypted message: %s \n", username);
        printf("Encrypted password: %s \n", password);
        char encryptedInput[100];
        strcpy(encryptedInput,username);
        strcat(encryptedInput,",");
        strcat(encryptedInput,password);
        strcat(encryptedInput,"\0");
        printf("encryted msg: %s\n",encryptedInput);
        if ((numbytes = sendto(sockfd, encryptedInput, strlen(encryptedInput), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	    }

        printf("talker: sent %d bytes to %s\n", numbytes, encryptedInput);

        char buf[MAXBUFLEN];
        struct sockaddr_storage their_addr;
        socklen_t addr_len;
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        cout<<buf<<endl;
        if(buf[0]=='0'){
            count++;
            printf("%s received the result of authentication using TCP over port %s.",username,SERVERPORT);
            printf("Authentication failed: Username Does not exist \n");
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='1'){
            count++;
            printf("%s received the result of authentication using TCP over port %s.",username,SERVERPORT);
            printf("Authentication failed: Password does not match \n");
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='2'){
            cout<<"Success"<<endl;
            break;
        }
        else{
            perror("auth");
            exit(1);
        }
        



    }
    if(count==maxTries){
        printf("Authentication Failed for 3 attempts. Client will shut down. \n");
        freeaddrinfo(servinfo);
	    close(sockfd);
    }
    else{
        printf("%s received the result of authentication using TCP over port %s.",username,SERVERPORT);
        printf("Authentication is successful \n");
        freeaddrinfo(servinfo);
	    close(sockfd);
        if ((rv = getaddrinfo(LOCALHOST, CSPORT, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }
        // loop through all the results and make a socket
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }
        if (p == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            return 2;
        }

        int sockfdEE;
        struct addrinfo hintsEE, *servinfoEE, *pEE;
        int rvEE;
        int numbytesEE;

        memset(&hintsEE, 0, sizeof hintsEE);
        hintsEE.ai_family = AF_INET; // set to AF_INET to use IPv4
        hintsEE.ai_socktype = SOCK_DGRAM;

        if ((rvEE = getaddrinfo(LOCALHOST, EEPORT, &hintsEE, &servinfoEE)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rvEE));
            return 1;
        }
        // loop through all the results and make a socket
        for(pEE = servinfoEE; pEE != NULL; pEE = pEE->ai_next) {
            if ((sockfdEE = socket(pEE->ai_family, pEE->ai_socktype,
                    pEE->ai_protocol)) == -1) {
                perror("talker: socket");
                continue;
            }

            break;
        }
        if (pEE == NULL) {
            fprintf(stderr, "talker: failed to create socket\n");
            return 2;
        }

        //HERE GOES COURSES REQUESTS
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
            char combinedInput[100];
            strcpy(combinedInput,courseNum);
            strcat(combinedInput,",");
            strcat(combinedInput,category);
            strcat(combinedInput,"\0");
            cout<<combinedInput<<endl;

            if(courseName.compare("CS")==0){
                if ((numbytes = sendto(sockfd, combinedInput, strlen(combinedInput), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
                }
                printf("talker: sent %d bytes to %s\n", numbytes, courseNum);

                char buf[MAXBUFLEN];
                struct sockaddr_storage their_addr;
                socklen_t addr_len;
                addr_len = sizeof their_addr;
                if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                cout<<buf<<endl;
                memset(buf, 0, sizeof buf);
            }
            else if(courseName.compare("EE")==0){
                if ((numbytesEE = sendto(sockfdEE, combinedInput, strlen(combinedInput), 0,
                pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
                }
                printf("talker: sent %d bytes to %s\n", numbytesEE, courseNum);

                char buf[MAXBUFLEN];
                struct sockaddr_storage their_addr;
                socklen_t addr_len;
                addr_len = sizeof their_addr;
                if ((numbytesEE = recvfrom(sockfdEE, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                cout<<buf<<endl;
                memset(buf, 0, sizeof buf);
                
            }
            else{
                cout<<"Wrong course input"<<endl;
            }
            //char encryptedInput[100];

        }
        




    }
	

	
   
}