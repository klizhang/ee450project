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

int main() {
    //https://www.tutorialspoint.com/cplusplus-program-to-implement-caesar-cypher#
    cout<<"Enter the message: ";
    char msg[100];
    cin.getline(msg,100); //take the message as input
    //int i, j, length,key;
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
    printf("Encrypted message: %s \n", msg);
   
}