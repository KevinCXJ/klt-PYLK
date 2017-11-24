// main.cpp : Defines the entry point for the console application.
//

#include <stdio.h> 

extern "C" {
	void RunExample_trk_PYLK(int argc, char* argv[]);
}

int main(int argc, char* argv[])
{
	 RunExample_trk_PYLK(argc, argv);  
	return 0;
}

