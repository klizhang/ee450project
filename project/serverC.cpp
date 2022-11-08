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
#define MYPORT "21606"
#define MAXBUFLEN 100

using namespace std;

struct creds_struct{
    string username;
    string password;
};

void split(char buf[], char username[], char password[]) {
    char *start_ptr = buf;
    char *tab_ptr = strchr(start_ptr, ',');
    if (tab_ptr != NULL){ //There is 1st tab
      *tab_ptr++ = '\0';
      username = start_ptr;
      start_ptr = tab_ptr;
      printf("start points to: %s",start_ptr);

      password = start_ptr;

    }
    else{ 
      printf("Error,no 1st tab found \n");
      exit(-1);
    }

}

/*SOURCE from beej*/
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
    //https://stackoverflow.com/questions/6399822/reading-a-text-file-fopen-vs-ifstream
    string line;
    ifstream myfile ("cred.txt");
    vector<creds_struct> creds;
    vector<string> v;
    //creds_struct creds;
    if (myfile.is_open())
    {
        while ( myfile.good() )
        {   
            getline (myfile,line);
            //https://www.geeksforgeeks.org/program-to-parse-a-comma-separated-string-in-c/#:~:text=Get%20the%20substring%20if%20the,and%20stored%20in%20the%20vector
            stringstream ss(line);
            // string substr;
            while (ss.good()) {
                string substr;
                getline(ss, substr, ',');
                v.push_back(substr);
            }
        }
        myfile.close();
    }
    else 
    {  
        cout << "Unable to open file" << endl; 
        return 0;
    }
    creds_struct cred;
    for (size_t i=0;i<v.size();i+=2)
    {
        cred.username = v[i];
        int length = v[i+1].length()-1;
        v[i+1][length] = '\0';
        cred.password = v[i+1];
        creds.push_back(cred);
    }
    
    //SOURCE from beej
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    //char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    //hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(LOCALHOST, MYPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);

    printf("The ServerC is up and running using UDP on port %s \n",MYPORT);
    while(1){
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        // printf("The ServerC received an authentication request from the Main Server. %s\n",
        //     inet_ntop(their_addr.ss_family,
        //         get_in_addr((struct sockaddr *)&their_addr),
        //         s, sizeof s));
        printf("The ServerC received an authentication request from the Main Server.\n");
        //printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        //printf("listener: packet contains \"%s\"\n", buf);

        vector<string> v;
        stringstream ss(buf);
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            v.push_back(substr);
        }
        string username = v[0];
        string password = v[1];
        char auth[2];
        //int auth = 0;
        //char* auth[MAXBUFLEN];
        auth[0] = '0';
        //strcpy(auth,'0');
        //auth = "FAIL_NO_USER";
        for(size_t i=0;i<creds.size();i++)
        {    
            if(strcmp(username.c_str(),creds[i].username.c_str())==0){
                cout<<"got username right"<<endl;
                //auth = 1;     
                //auth = "FAIL_PASS_NO_MATCH";
                //strcpy(auth,'1');
                auth[0]='1';
                if(strcmp(password.c_str(),creds[i].password.c_str())==0){
                    cout<<"both right"<<endl;
                    //auth = 2;
                    //auth = "PASS";
                    //strcpy(auth,'2');
                    auth[0]='2';
                    break;
                }
                break;
            }
        }
        auth[1] = '\0';

        if ((numbytes = sendto(sockfd, auth, strlen(auth), 
          0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
		    perror("serverC: sendto");
		    exit(1);
	    }
        cout<<"The ServerC finished sending the response to the Main Server.";
        cout<<auth<<endl;
    }
    close(sockfd);
    return 0;
}
