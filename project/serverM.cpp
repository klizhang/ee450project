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
    int sockTCP, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr_client; // connector's address information (client)
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	int rv;
    int numbytes;
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
	struct addrinfo hintsUDP, *servinfoUDP, *pUDP;
	int rvUDP;

	memset(&hintsUDP, 0, sizeof hintsUDP);
	hintsUDP.ai_family = AF_INET; // set to AF_INET to use IPv4
	hintsUDP.ai_socktype = SOCK_DGRAM;

	if ((rvUDP = getaddrinfo(LOCALHOST, UDPMPORT, &hintsUDP, &servinfoUDP)) != 0) {
		fprintf(stderr, "getaddrinfo UDP: %s\n", gai_strerror(rvUDP));
		return 1;
	}

	// loop through all the results and make a socket
	for(pUDP = servinfoUDP; pUDP != NULL; pUDP = pUDP->ai_next) {
		if ((sockUDP = socket(p->ai_family, p->ai_socktype,
				pUDP->ai_protocol)) == -1) {
			perror("talker UDP: socket");
			continue;
		}
        if (bind(sockUDP, pUDP->ai_addr, pUDP->ai_addrlen) < 0) {
            close(sockUDP);
            perror("talker UDP: bind");
            continue;
        }
		break;
	}
	if (pUDP == NULL) {
		fprintf(stderr, "talker UDP: failed to create socket\n");
		return 2;
	}
    freeaddrinfo(servinfoUDP);

    /*Get the address for serverC */
    struct addrinfo hintsC, *servinfoC, *pC;
    memset(&hintsC, 0, sizeof hintsC);
	hintsC.ai_family = AF_INET; // set to AF_INET to use IPv4
	hintsC.ai_socktype = SOCK_DGRAM;
	int rvC;
    if ((rvC = getaddrinfo(LOCALHOST, CREDPORT, &hintsC, &servinfoC)) != 0) {
		fprintf(stderr, "getaddrinfo C: %s\n", gai_strerror(rvC));
		return 1;
	}

	// loop through all the results and make a socket
	for(pC = servinfoC; pC != NULL; pC = pC->ai_next) {
		if ((socket(pC->ai_family, pC->ai_socktype,
				pC->ai_protocol)) == -1) {
			perror("talker C: socket");
			continue;
		}
		break;
	}

	if (pC == NULL) {
		fprintf(stderr, "talker C: failed to create socket Credentials\n");
		return 2;
	}
    //freeaddrinfo(servinfoC);
    /*Source from Beej
    Get address of serverCS
    */
    struct addrinfo hintsCS, *servinfoCS, *pCS;
    memset(&hintsCS, 0, sizeof hintsCS);
	hintsCS.ai_family = AF_INET; // set to AF_INET to use IPv4
	hintsCS.ai_socktype = SOCK_DGRAM;
	int rvCS;
    if ((rvCS = getaddrinfo(LOCALHOST, CSPORT, &hintsCS, &servinfoCS)) != 0) {
        fprintf(stderr, "getaddrinfo CS: %s\n", gai_strerror(rvCS));
        return 1;
    }
    // loop through all the results and make a socket
    for(pCS = servinfoCS; pCS != NULL; pCS = pCS->ai_next) {
        if ((socket(pCS->ai_family, pCS->ai_socktype,
                pCS->ai_protocol)) == -1) {
            perror("talker CS: socket");
            continue;
        }

        break;
    }
    if (pCS == NULL) {
        fprintf(stderr, "talker CS: failed to create socket\n");
        return 2;
    }
    //freeaddrinfo(servinfoCS);

    struct addrinfo hintsEE, *servinfoEE, *pEE;
    memset(&hintsEE, 0, sizeof hintsEE);
	hintsEE.ai_family = AF_INET; // set to AF_INET to use IPv4
	hintsEE.ai_socktype = SOCK_DGRAM;
    int rvEE;
    int numbytesEE;

    memset(&hintsEE, 0, sizeof hintsEE);
    hintsEE.ai_family = AF_INET; // set to AF_INET to use IPv4
    hintsEE.ai_socktype = SOCK_DGRAM;

    if ((rvEE = getaddrinfo(LOCALHOST, EEPORT, &hintsEE, &servinfoEE)) != 0) {
        fprintf(stderr, "getaddrinfo EE: %s\n", gai_strerror(rvEE));
        return 1;
    }
    // loop through all the results and make a socket
    for(pEE = servinfoEE; pEE != NULL; pEE = pEE->ai_next) {
        if ((socket(pEE->ai_family, pEE->ai_socktype,
                pEE->ai_protocol)) == -1) {
            perror("talker EE: socket");
            continue;
        }

        break;
    }
    if (pEE == NULL) {
        fprintf(stderr, "talker EE: failed to create socket\n");
        return 2;
    }
    //freeaddrinfo(servinfoEE);
    printf("The main server is up and running.\n");

    while(1){
        sin_size = sizeof their_addr_client;
		new_fd = accept(sockTCP, (struct sockaddr *)&their_addr_client, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;	
		}
		//cout<<"new child created"<<endl;
		if (!fork()) { // this is the child process
			close(sockTCP); // child doesn't need the listener 
			char buf[MAXBUFLEN];
            int count = 0;
            int maxTries = 3;
			while (count<maxTries){
				memset(buf, 0, sizeof buf);
				if ((numbytes = recv(new_fd, buf, MAXBUFLEN-1, 0)) == -1) {
					perror("recv");
					exit(1);
				}
				if(numbytes == 0){
					//printf("Disconnected \n"); ///////////////////////////////
					break;
				}
                //HANDLE THE USERNAME AND PASSWORD
                char input[100];
                strcpy(input,buf);
                char * token = strtok(input, ",");
                char * username = token;
                token = strtok(NULL, ",");
                //char * password = token;
                encrypt(buf);
                printf("The main server received the authentication for %s using TCP over port %s \n",username,TCPMPORT);
                if ((numbytes = sendto(sockUDP, buf, strlen(buf), 0,
                    pC->ai_addr, pC->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
                }
                //printf("talker: sent %d bytes to %s\n", numbytes, buf);
                printf("The main server sent an authentication request to serverC.\n");

                char responseC[MAXBUFLEN];
                struct sockaddr_storage their_addr;
                socklen_t addr_len;
                addr_len = sizeof their_addr;
                if ((numbytes = recvfrom(sockUDP, responseC, MAXBUFLEN-1 , 0,
                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }
                //cout<<responseC<<endl;
                printf("The main server received the result of the authentication request from ServerC using UDP over port %s. \n",CREDPORT);
                //char messageClient[MAXBUFLEN];
                if(responseC[0]=='0' || responseC[0]=='1'){
                    count++;
                }
                else if(responseC[0]!='2'){
                    perror("auth");
                    exit(1);
                }
				
				if (send(new_fd, responseC, strlen(responseC), 0) == -1){
					perror("send");
					close(new_fd);
					exit(0);
				}
                printf("The main server sent the authentication result to the client.\n");
                if(responseC[0]=='2'){
                    freeaddrinfo(servinfo);
                    //HERE GOES COURSES REQUESTS
                    while(1){
                        char courseInput[MAXBUFLEN];
                        memset(courseInput, 0, sizeof courseInput);
                        if ((numbytes = recv(new_fd, courseInput, MAXBUFLEN-1, 0)) == -1) {
                            perror("recv");
                            exit(1);
                        }
                        if(numbytes == 0){ //client disconnected
                            //printf("Disconnected \n"); ///////////////////////////////
                            break;
                        }
                        //cout<<courseInput<<endl;
                        char courses[100];
                        strcpy(courses,courseInput);
                        char * token = strtok(courses, ",");
                        char * courseNum = token;
                        token = strtok(NULL, ",");
                        char * category = token;
                        // /Credit / Professor / Days / CourseName
                        if (strcmp(category,"Credit")==0 || strcmp(category,"Professor")==0 || strcmp(category,"Days")==0 || strcmp(category,"CourseName")==0){
                            printf("The main server received from %s to query course %s about %s using TCP over port %s.\n",username,courseNum,category,TCPMPORT);
                            char course[3];
                            memset(course, 0, sizeof course);
                            memcpy(course, courseInput , 2);
                            //cout<<"the course is:";
                            //cout<<course<<endl;
                            if(strcmp(course,"CS")==0){
                                if ((numbytes = sendto(sockUDP, courseInput, strlen(courseInput), 0,
                                pCS->ai_addr, pCS->ai_addrlen)) == -1) {
                                    perror("talker: sendto");
                                    exit(1);
                                }
                                //printf("talker: sent %d bytes to CSServer %s\n", numbytes, courseInput);
                                cout<<"The main server sent a request to serverCS"<<endl;
                                cout<<courseInput<<endl; //--------------------------------
                                //char buf[MAXBUFLEN];
                                memset(buf, 0, sizeof buf);
                                struct sockaddr_storage their_addr;
                                socklen_t addr_len;
                                addr_len = sizeof their_addr;
                                if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                    (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                    perror("recvfrom");
                                    exit(1);
                                }
                                // cout<<buf<<endl;
                                printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                if (send(new_fd, buf, strlen(buf), 0) == -1){
                                    perror("send");
                                    close(new_fd);
                                    exit(0);
                                }
                                printf("The main server sent the query information to the client.\n");
                                memset(buf, 0, sizeof buf);
                            }
                            //else if(courseInput.compare("EE")==0){
                            else if(strcmp(course,"EE")==0){
                                if ((numbytesEE = sendto(sockUDP, courseInput, strlen(courseInput), 0,
                                pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                                perror("talker: sendto");
                                exit(1);
                                }
                                //printf("talker: sent %d bytes to EEServer %s\n", numbytesEE, courseInput);
                                cout<<"The main server sent a request to serverEE"<<endl;
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
                                //cout<<buf<<endl;
                                printf("The main server received the response from serverEE using UDP over port %s.\n",EEPORT);
                                if (send(new_fd, buf, strlen(buf), 0) == -1){
                                    perror("send");
                                    close(new_fd);
                                    exit(0);
                                }
                                printf("The main server sent the query information to the client.\n");
                                memset(buf, 0, sizeof buf);
                                
                            }
                            else{
                                //cout<<"Wrong course input"<<endl;
                                memset(buf, 0, sizeof buf);
                                strcpy(buf,"Wrong input");
                                if (send(new_fd, buf, strlen(buf), 0) == -1){
                                    perror("send");
                                    close(new_fd);
                                    exit(0);
                                }
                                printf("The main server sent the query information to the client.\n");
                                memset(buf, 0, sizeof buf);
                            }
                        }
                        else{
                            list<char*> courseList;
                            char courses2[100];
                            char *token2;
                            strcpy(courses2,courseInput);
                            
                            /* get the first token */
                            token2 = strtok(courses2, ",");
                            
                            /* walk through other tokens */
                            while( token2 != NULL ) {
                                courseList.push_back(token2);
                                //printf( " %s\n", token2 );
                                token2 = strtok(NULL, ",");
                            }
                            for (std::list<char*>::iterator it = courseList.begin(); it != courseList.end(); ++it){
                                std::cout << *it<<endl;
                                printf("The main server received from %s to query course %s about everything using TCP over port %s.\n",username,courseNum,TCPMPORT);
                                char course2[3];
                                memset(course2,0,sizeof course2);
                                memcpy(course2, *it , 2);
                                cout<<course2<<endl;
                                //cout<<"the course is:";
                                //cout<<course<<endl;
                                char response[50];
                                memset(response, 0, sizeof response);
                                char courseCredit[50];
                                strcpy(courseCredit,*it);
                                strcat(courseCredit,",Credit");
                                char courseProf[50];
                                strcpy(courseProf,*it);
                                strcat(courseProf,",Professor");
                                char courseDays[50];
                                strcpy(courseDays,*it);
                                strcat(courseDays,",Days");
                                char courseName[50];
                                strcpy(courseName,*it);
                                strcat(courseName,",CourseName");
                                cout<<"COURSE IS:";
                                cout<<course2<<endl;
                                if(strcmp(course2,"CS")==0){
                                    strcat(response,*it);
                                    strcat(response,": ");
                                    if ((numbytes = sendto(sockUDP, courseCredit, strlen(courseCredit), 0,
                                    pCS->ai_addr, pCS->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverCS"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    struct sockaddr_storage their_addr;
                                    socklen_t addr_len;
                                    addr_len = sizeof their_addr;
                                    if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);

                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytes = sendto(sockUDP, courseProf, strlen(courseProf), 0,
                                    pCS->ai_addr, pCS->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverCS"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytes = sendto(sockUDP, courseDays, strlen(courseDays), 0,
                                    pCS->ai_addr, pCS->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverCS"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytes = sendto(sockUDP, courseName, strlen(courseName), 0,
                                    pCS->ai_addr, pCS->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverCS"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytes = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                    strcat(response,buf);
                                    cout<<response<<endl;
                                    //cout<<buf<<endl;
                                    // printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                    if (send(new_fd, response, strlen(response), 0) == -1){
                                        perror("send");
                                        close(new_fd);
                                        exit(0);
                                    }
                                    printf("The main server sent the query information to the client.\n");
                                    //memset(response, 0, sizeof response);

                                }
                                //else if(courseInput.compare("EE")==0){
                                else if(strcmp(course2,"EE")==0){
                                    strcat(response,*it);
                                    strcat(response,": ");
                                    if ((numbytesEE = sendto(sockUDP, courseCredit, strlen(courseCredit), 0,
                                    pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverEE"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    struct sockaddr_storage their_addr;
                                    socklen_t addr_len;
                                    addr_len = sizeof their_addr;
                                    if ((numbytesEE = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverEE using UDP over port %s.\n",EEPORT);

                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytesEE = sendto(sockUDP, courseProf, strlen(courseProf), 0,
                                    pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverEE"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytesEE = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverEE using UDP over port %s.\n",EEPORT);
                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytesEE = sendto(sockUDP, courseDays, strlen(courseDays), 0,
                                    pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverEE"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytesEE = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverEE using UDP over port %s.\n",EEPORT);
                                    strcat(response,buf);
                                    strcat(response,", ");

                                    if ((numbytesEE = sendto(sockUDP, courseName, strlen(courseName), 0,
                                    pEE->ai_addr, pEE->ai_addrlen)) == -1) {
                                        perror("talker: sendto");
                                        exit(1);
                                    }
                                    cout<<"The main server sent a request to serverEE"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    if ((numbytesEE = recvfrom(sockUDP, buf, MAXBUFLEN-1 , 0,
                                        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
                                        perror("recvfrom");
                                        exit(1);
                                    }
                                    printf("The main server received the response from serverEE using UDP over port %s.\n",EEPORT);
                                    strcat(response,buf);
                                    cout<<response<<endl;
                                    //cout<<buf<<endl;
                                    // printf("The main server received the response from serverCS using UDP over port %s.\n",CSPORT);
                                    if (send(new_fd, response, strlen(response), 0) == -1){
                                        perror("send");
                                        close(new_fd);
                                        exit(0);
                                    }
                                    printf("The main server sent the query information to the client.\n");
                                    //memset(response, 0, sizeof response);








                                    
                                }
                                else{
                                    //cout<<"Wrong course input"<<endl;
                                    memset(buf, 0, sizeof buf);
                                    strcat(buf,"Didn't find the course: ");
                                    strcat(buf,*it);
                                    cout<<buf<<endl;
                                    // strcat(buf.": ")
                                    // strcat(buf,"Wrong input");
                                    if (send(new_fd, buf, strlen(buf), 0) == -1){
                                        perror("send");
                                        close(new_fd);
                                        exit(0);
                                    }
                                    printf("The main server sent the query information to the client.\n");
                                    memset(buf, 0, sizeof buf);
                                }
                            }
                        }
                        
                        //char encryptedInput[100];

                    }
                    //freeaddrinfo(servinfo);

                }

			}
			if(count==maxTries){
                printf("Authentication Failed for 3 attempts. Client will shut down. \n");
                //freeaddrinfo(servinfo);
                //close(sockfd);
            }
            
			close(new_fd);
			exit(0);
		}
		//close(new_fd);  // parent doesn't need this
		//exit(0);
    }	
}