all: serverM serverCS serverEE serverC 

serverM: serverM.cpp
	g++ -g -Wall -std=c++11 serverM.cpp -o serverM

serverCS: serverCS.cpp
	g++ -g -Wall -std=c++11 serverCS.cpp -o serverCS

serverEE: serverEE.cpp
	g++ -g -Wall -std=c++11 serverEE.cpp -o serverEE

serverC: serverC.cpp
	g++ -g -Wall serverC.cpp -o serverC

clean:
	rm -f *.o all

