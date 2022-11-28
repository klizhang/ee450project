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
#define EEPORT "23606"
#define MAXBUFLEN 100

using namespace std;

struct ee_struct{
    string courseID;
    int numCredits;
    string prof;
    string days;
    string courseName;
};

int main() {
    //https://stackoverflow.com/questions/6399822/reading-a-text-file-fopen-vs-ifstream
    string line;
    ifstream myfile ("ee.txt");
    vector<ee_struct> eeList;
    vector<string> v;
    //creds_struct creds;
    if (myfile.is_open())
    {
        while ( myfile.good() )
        {   
            getline (myfile,line);
            //https://www.geeksforgeeks.org/program-to-parse-a-comma-separated-string-in-c/#:~:text=Get%20the%20substring%20if%20the,and%20stored%20in%20the%20vector
            stringstream ss(line);
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
    ee_struct ee;
    for (size_t i=0;i<v.size();i+=5)
    {
        ee.courseID = v[i];
        ee.numCredits = stoi(v[i+1]);
        ee.prof = v[i+2];
        ee.days = v[i+3];
        ee.courseName = v[i+4];
        eeList.push_back(ee);
        
    }
    //SOURCE from beej
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(LOCALHOST, EEPORT, &hints, &servinfo)) != 0) {
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

    printf("The ServerEE is up and running using UDP on port %s \n",EEPORT);
    while(1){
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        buf[numbytes] = '\0';
        vector<string> v;
        stringstream ss(buf);
        while (ss.good()) {
            string substr;
            getline(ss, substr, ',');
            v.push_back(substr);
        }
        string courseNum = v[0];
        string category = v[1];
        printf("The ServerEE received a request from the Main Server about the %s of %s.\n",category.c_str(),courseNum.c_str());
        string responseEE = "Wrong input";
        for(size_t i=0;i<eeList.size();i++)
        {    
            if(strcmp(courseNum.c_str(),eeList[i].courseID.c_str())==0){
                if(strcmp(category.c_str(),"Credit")==0){
                    responseEE = to_string(eeList[i].numCredits);
                    printf("The course information has been found: The %s of %s is %s \n",category.c_str(),courseNum.c_str(),responseEE.c_str());
                }
                else if(strcmp(category.c_str(),"Professor")==0){
                    responseEE = eeList[i].prof;
                    printf("The course information has been found: The %s of %s is %s \n",category.c_str(),courseNum.c_str(),responseEE.c_str());
                }
                else if(strcmp(category.c_str(),"Days")==0){
                    responseEE = eeList[i].days;
                    printf("The course information has been found: The %s of %s is %s \n",category.c_str(),courseNum.c_str(),responseEE.c_str());
                }
                else if(strcmp(category.c_str(),"CourseName")==0){
                    responseEE = eeList[i].courseName;
                    printf("The course information has been found: The %s of %s is %s \n",category.c_str(),courseNum.c_str(),responseEE.c_str());
                }
                else{
                    responseEE = "Wrong category";
                    printf("Wrong category: %s, the only valid categories are (Credit / Professor / Days / CourseName) \n",category.c_str());
                }
                break;
            }
        }
        if(strcmp(responseEE.c_str(),"Wrong input")==0){
            printf("Didn’t find the course: %s \n",courseNum.c_str());
        }
 
        int n = responseEE.length();
        char char_array_EE[n + 1];
        strcpy(char_array_EE, responseEE.c_str());
        if ((numbytes = sendto(sockfd, char_array_EE, strlen(char_array_EE), 
          0,(struct sockaddr *)&their_addr, addr_len)) == -1) {
		    perror("serverEE: sendto");
		    exit(1);
	    }
        cout<<"The ServerEE finished sending the response to the Main Server."<<endl;
        
    }
    close(sockfd);
    return 0;
}
