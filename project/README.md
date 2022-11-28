a. KEVIN LI ZHANG
b. 7090721606
c. I have completed the project with the given requirements. The extra credit is also implemented.
d. serverC.cpp: it is the credentials server where it creates a struct and stores the data from creds.txt. it has its own UDP port.
   serverCS.cpp: it is the CS server, where it creates a struct and stores the data from CS.txt. it has its own UDP port.
   serverEE.cpp: it is the EE server, where it creates a struct and stores the data from EE.txt. it has its own UDP port.
   serverM.cpp: it is the main server, where it comunicates with the three backend servers (serverC, serverCS, serverEE) through UDP. It connects to the client using a TCP connection. it encodes the username and password from the client and verifies it with serverC. It sends the course codes from the client to the corresponding server (CS or EE).
   client.cpp: it is the user's frontend. It comunicates with the main server (serverM) with a dynamically allocated port number. It asks for username and password, and sends it to the main server for a response. Once it is authenticated, the user can lookup courses' informations from the databases (CS and EE).
e. The credential server responds with a 0 for authentication failed (wrong username). 1 for partial correct authentication (correct username but wrong password), and 2 for correct authentication. If it is a single course lookup, a 0 is appended at the beggining of the message.
g. when requested two or more non-CS/EE courses, a usleep was added to make sure that the TCP did not send the response synchronously. 
h. The Beej guide was used to create the UDP and TCP connections. Code was modified from: //https://www.tutorialspoint.com/cplusplus-program-to-implement-caesar-cypher# to create the encoding for the credentials in serverM. Modified code from //https://stackoverflow.com/questions/6399822/reading-a-text-file-fopen-vs-ifstream for reading from a txt file for the 3 backend servers. Modified code from             //https://www.geeksforgeeks.org/program-to-parse-a-comma-separated-string-in-c/#:~:text=Get%20the%20substring%20if%20the,and%20stored%20in%20the%20vector to separate string with commas for the 3 backend servers.