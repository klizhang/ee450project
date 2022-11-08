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

using namespace std;

struct cs_struct{
    string courseID;
    int numCredits;
    string prof;
    string days;
    string courseName;
};

int main() {
    //https://stackoverflow.com/questions/6399822/reading-a-text-file-fopen-vs-ifstream
    string line;
    ifstream myfile ("cs.txt");
    vector<cs_struct> csList;
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
        cout << "Unable to open file" <<endl;
        return 0; 
    }
    cs_struct cs;
    for (size_t i=0;i<v.size();i+=5)
    {
        cs.courseID = v[i];
        cs.numCredits = stoi(v[i+1]);
        cs.prof = v[i+2];
        cs.days = v[i+3];
        cs.courseName = v[i+4];
        csList.push_back(cs);
        
    }
    for(int i=0;i<csList.size();i++)
    {
        cout<<csList[i].courseID<<endl;
        cout<<csList[i].numCredits<<endl;
        cout<<csList[i].prof<<endl;
    }

    return 0;
}
