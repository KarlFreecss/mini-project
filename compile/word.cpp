//Author		: freecss
//Mail			: karlfreecss@gmail.com
//Created Time 	: 2017-07-01
//Modified Time : 2018-10-06
//
//This is a course project
//
#include "word.h"

using namespace std;

void WORDPARSER::init(){
	keyWord["int"] = "int";
	keyWord["void"] = "void";
	keyWord["if"] = "if";
	keyWord["else"] = "else";
	keyWord["while"] = "while";
	keyWord["return"] = "return";
	line = 1;
}

WORDPARSER::WORDPARSER(const string & file_name){
	sfile.open(file_name, ios_base::in);
	init();
}

void WORDPARSER::open(const string & file_name){
	line = 1;
	sfile.open(file_name, ios_base::in);
}

WORDPARSER::WORDPARSER(){
	init();
}


void WORDPARSER::clearBlank(){
	char c;
	while (1) {
		if (!(sfile.get(c))) return;
		if (c == '\n') line++;
		if (c != ' ' && c != '\n' && c != '\r' && c != '\t') break;
	}
	sfile.unget();
}

int WORDPARSER::getLine(){
	return line;
}

pair<string, string> WORDPARSER::divOrNote(){
	string res = "/", word;
	char sc;
	sfile.get(sc);
	if (sc == '/') {
		word = res + '/';
	} else if (sc == '*'){
		word = res + '*';
	} else {
		sfile.unget();
		word = res;
	}
	return make_pair(word, word);
}

pair<string, string> WORDPARSER::mulOrNode(){
	string res = "*", word;
	char sc;
	sfile.get(sc);
	if (sc == '/') {
		word = res + '/';
	} else {
		sfile.unget();
		word = res;
	}
	return make_pair(word, word);
}

pair<string, string> WORDPARSER::alphaOrNumber(char c){
	string word("");
	if (isdigit(c)) {
		while(1) {
			word += c;
			sfile.get(c);
			if (!isdigit(c)) {
				sfile.unget();
				return make_pair("number", word);
			}
		}
	}

	if (isalpha(c)) {
		while(1) {
			word += c;
			sfile.get(c);
			if (!(isdigit(c) || isalpha(c))) {
				sfile.unget();
				if (keyWord.find(word) == keyWord.end())
					return make_pair("identity", word);
				return make_pair(word, word);
			}
		}
	}
	return make_pair("undefined", string("") + c);
}

pair<string, string> WORDPARSER::getWord(){
	clearBlank();
	char c;
	char sc;
	string word;
	if (!(sfile.get(c))) return make_pair("#", "");
	switch (c){
		case '=':
		case '>':
		case '<':
		case '!':
			sfile.get(sc);
			if (sc == '=') {
				word = c + string("=");
				return make_pair(word, word);
			} else {
				sfile.unget();
			}
		case '+':
		case '-':
		case ';':
		case ',':
		case '(':
		case ')':
		case '{':
		case '}':
		case '#':
			word = c + string("");
			return make_pair(word, word);
		case '/':
			return divOrNote();
		case '*':
			return mulOrNode();
		default:
			return alphaOrNumber(c);
	}
}

