//Author		: freecss
//Mail			: karlfreecss@gmail.com
//Created Time 	: 2017-07-01
//Modified Time : 2018-10-06
//
//This is a course project
//

#include <cstdio>
#include <iostream>
#include <fstream>
#include <map>
#include <utility>

class WORDPARSER{
private:
	std::fstream sfile;
	int line;
	std::pair<std::string, std::string> divOrNote();
	std::pair<std::string, std::string> mulOrNode();	
	std::pair<std::string, std::string> alphaOrNumber(char c);
	std::map<std::string, std::string> keyWord;
	void clearBlank();
	void init();

public:
	WORDPARSER(const std::string & file_name);
	WORDPARSER();
	void open(const std::string & file_name);
	int getLine();
	std::pair<std::string, std::string> getWord();
};
