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
#define CREDPORT "21606"	// the port users will be connecting to
#define CSPORT "22606"
#define EEPORT "23606"
#define UDPMPORT "24606" 
#define TCPMPORT "25606"
#define MAXBUFLEN 100
#define BACKLOG 10	 // how many pending connections queue will hold

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

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main() {

    /*Source from Beej
    Create TCP socket
    */
    int sockTCP, new_fd,numbytes;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int rv;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP
	if ((rv = getaddrinfo(LOCALHOST, TCPMPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockTCP = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
		if (setsockopt(sockTCP, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		if (bind(sockTCP, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockTCP);
			perror("server: bind");
			continue;
		}
		break;
	}
	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockTCP, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	/*Source from Beej
    create UDP socket
    */
    
    int sockUDP;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // set to AF_INET to use IPv4
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(LOCALHOST, UDPMPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockUDP = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
        if (bind(sockUDP, p->ai_addr, p->ai_addrlen) < 0) {
            close(sockUDP);
            perror("talker: bind");
            continue;
        }
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "talker: failed to create socket\n");
		return 2;
	}
    freeaddrinfo(servinfo);
    /*Get the address for serverC */
	struct addrinfo *pC;
    //struct addrinfo hints, *servinfo, *p;
    if ((rv = getaddrinfo(LOCALHOST, CREDPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(pC = servinfo; pC != NULL; pC = pC->ai_next) {
		if ((socket(pC->ai_family, pC->ai_socktype,
				pC->ai_protocol)) == -1) {
			perror("talker: socket");
			continue;
		}
		break;
	}

	if (pC == NULL) {
		fprintf(stderr, "talker: failed to create socket Credentials\n");
		return 2;
	}

    /*Source from Beej
    Get address of serverCS
    */
	struct addrinfo *pCS;

    if ((rv = getaddrinfo(LOCALHOST, CSPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(pCS = servinfo; pCS != NULL; pCS = pCS->ai_next) {
        if ((socket(pCS->ai_family, pCS->ai_socktype,
                pCS->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }
    if (pCS == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

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
        if ((socket(pEE->ai_family, pEE->ai_socktype,
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

    printf("The main server is up and running.\n");

    while(1){
        sin_size = sizeof their_addr;
		new_fd = accept(sockTCP, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			// continue;
			
		}
		cout<<"new child created"<<endl;

		if (!fork()) { // this is the child process

			close(sockTCP); // child doesn't need the listener -------------------???------------------
			//printf("%d",numbytes);
			char buf[MAXBUFLEN];
			
			while (1){
				memset(buf, 0, sizeof buf);
				if ((numbytes = recv(new_fd, buf, MAXBUFLEN-1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				cout<<numbytes<<endl;
				if(numbytes == 0){
					
					break;
				}
				// inet_ntop(their_addr.ss_family,
				// get_in_addr((struct sockaddr *)&their_addr),
				// s, sizeof s);
				// printf("server: got connection from %s\n", s);
				//fflush(stdout);
				// string mess = "Hello, world";
				// int n = mess.length();
				// char message[n + 1];
				// strcpy(message, mess.c_str());
				//cout<<buf<<endl;
				printf("%s \n",buf);
				printf("%d \n",strlen(buf));
				if (send(new_fd, buf, strlen(buf), 0) == -1){
					perror("send");
					close(new_fd);
					exit(0);
				}

			}
			
			close(new_fd);
			exit(0);
		}
		//close(new_fd);  // parent doesn't need this
		//exit(0);
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
        if ((numbytes = sendto(sockUDP, encryptedInput, strlen(encryptedInput), 0,
			 p->ai_addr, p->ai_addrlen)) == -1) {
		perror("talker: sendto");
		exit(1);
	    }

        printf("talker: sent %d bytes to %s\n", numbytes, encryptedInput);

        char buf[MAXBUFLEN];
        struct sockaddr_storage their_addr;
        socklen_t addr_len;
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        cout<<buf<<endl;
        if(buf[0]=='0'){
            count++;
            printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
            printf("Authentication failed: Username Does not exist \n");
            printf("Attempts remaining: %d \n",maxTries-count);
        }
        else if(buf[0]=='1'){
            count++;
            printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
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
	    //close(sockfd);
    }
    else{
        printf("%s received the result of authentication using TCP over port %s.",username,CREDPORT);
        printf("Authentication is successful \n");
        freeaddrinfo(servinfo);
	    //close(sockfd);
        

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
            struct sockaddr_in sin;
            socklen_t len = sizeof(sin);
            if (getsockname(sockUDP, (struct sockaddr *)&sin, &len) == -1)
                perror("getsockname");
            else
                printf("port number in use: %d\n", ntohs(sin.sin_port));

            if(courseName.compare("CS")==0){
                if ((numbytes = sendto(sockUDP, combinedInput, strlen(combinedInput), 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
                }
                printf("talker: sent %d bytes to %s\n", numbytes, courseNum);

                char buf[MAXBUFLEN];
                memset(buf, 0, sizeof buf);
                struct sockaddr_storage their_addr;
                socklen_t addr_len;
                addr_len = sizeof their_addr;
                if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                cout<<buf<<endl;
                memset(buf, 0, sizeof buf);
            }
            else if(courseName.compare("EE")==0){
                if ((numbytesEE = sendto(sockUDP, combinedInput, strlen(combinedInput), 0,
                pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
                }
                printf("talker: sent %d bytes to %s\n", numbytesEE, courseNum);

                char buf[MAXBUFLEN];
                memset(buf, 0, sizeof buf);
                struct sockaddr_storage their_addr;
                socklen_t addr_len;
                addr_len = sizeof their_addr;
                if ((numbytesEE = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
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
        freeaddrinfo(servinfo);




    }
	

	
   
}