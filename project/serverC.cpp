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

#define LOCAL_HOST "127.0.0.1" // Host address

using namespace std;

struct creds_struct{
    string username;
    string password;
};



int main() {
    //https://stackoverflow.com/questions/6399822/reading-a-text-file-fopen-vs-ifstream
    string line;
    ifstream myfile ("cred.txt");
    vector <creds_struct> creds;
    vector<string> v;
    //creds_struct creds;
    if (myfile.is_open())
    {
        int i=0;
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
        cout << "Unable to open file"; 
    }
    creds_struct cred;
    for (size_t i=0;i<v.size();i+=2)
    {
        cred.username = v[i];
        cred.password = v[i+1];
        creds.push_back(cred);
        
    }
    for(int i=0;i<creds.size();i++)
    {
        cout<<creds[i].username<<endl;
        cout<<creds[i].password<<endl;
    }

    return 0;
}
