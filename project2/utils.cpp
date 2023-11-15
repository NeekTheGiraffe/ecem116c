#include "utils.h"

#include <fstream>
#include <iostream>
#include <sstream>

std::vector<Instruction> parseTrace(const std::string& filename)
{
    std::ifstream fin;

	// opening file
	fin.open(filename.c_str());
	if (!fin){ // making sure the file is correctly opened
		std::cout << "Error opening " << filename << std::endl;
		exit(1);
	}
	
	// reading the text file
	std::string line;
	std::vector<Instruction> myTrace;
	int TraceSize = 0;
	std::string s1,s2,s3,s4;
    while( std::getline(fin,line) )
    {
        std::stringstream ss(line);
        std::getline(ss,s1,','); 
        std::getline(ss,s2,','); 
        std::getline(ss,s3,','); 
        std::getline(ss,s4,',');
        myTrace.push_back(Instruction()); 
        myTrace[TraceSize].MemR = stoi(s1);
        myTrace[TraceSize].MemW = stoi(s2);
        myTrace[TraceSize].adr = stoi(s3);
        myTrace[TraceSize].data = stoi(s4);
        //cout<<myTrace[TraceSize].MemW << endl;
        TraceSize+=1;
    }
    fin.close();
    return myTrace;
}
