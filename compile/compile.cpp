//Author		: freecss
//Mail			: karlfreecss@gmail.com
//Created Time 	: 2017-07-01
//Modified Time : 2018-10-06
//
//This is a course project
//

#include "word.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <set>

#define debug() printf("%s:%d\n", __FILE__, __LINE__)

using namespace std;

#define REGNUM 32

ofstream tarFile("a.asm");
ofstream tmpFile("a.o");

class WORDTABLE{
public:
	map<string, int> globalWord;
	map<string, int> localWord;
	set<string> stored;
	string regTable[REGNUM];
	string regTableAddr[REGNUM];
	int regAcessTime[REGNUM];
	int globalCount;
	int localCount;
	int regTimeCount;
	int tmpCount;
	int insertGlobalWord(const string & w);
	int insertLocalWord(const string & w);
	string getReg(const string &w, const vector<string> &);
	string getWord(const string & w, const vector<string> &);
	void deleteLocalWord(const int n);
	void clearReg();
	void initReg();
	WORDTABLE();
};

WORDTABLE::WORDTABLE() :
	globalCount(0),
	localCount(0),
	regTimeCount(0),
	tmpCount(0)
	{
	memset(regAcessTime, -1, sizeof(regAcessTime));
	for (int i = 0; i < REGNUM; ++i) regTable[i] = "";
}

int WORDTABLE::insertGlobalWord(const string &w){
	if (globalWord.find(w) != globalWord.end()) return -1;
	globalWord.insert(make_pair(w, globalCount));
	globalCount += 4;
	return 0;	
}

int WORDTABLE::insertLocalWord(const string &w){
	if (localWord.find(w) != localWord.end()) return -1;
	localWord.insert(make_pair(w, localCount));
	localCount += 4;
	return 0;
}

void WORDTABLE::deleteLocalWord(const int n){
	localCount -= n;
}

void WORDTABLE::clearReg(){
	stored.clear();
	localCount -= 4 * tmpCount;
	tmpCount = 0;
	for (int i = 0; i < REGNUM; ++i) regTable[i] = "";
	memset(regAcessTime, -1, sizeof(regAcessTime));
}

void WORDTABLE::initReg(){
	stored.clear();
	for (int i = 0; i < REGNUM; ++i) regTable[i] = "";
	memset(regAcessTime, -1, sizeof(regAcessTime));
}

string WORDTABLE::getWord(const string & w, const vector<string> & parent){
	for (int i = parent.size() - 1; i >= 0; --i) {
		if (localWord.find(parent[i] + "_" + w) != localWord.end()) {
			return to_string(localWord[parent[i] + "_" + w]) + "($s0)";
		}	
	}
	if (globalWord.find(w) != globalWord.end()) {
		return w;	
	}
	return "#";
}

string WORDTABLE::getReg(const string & _w, const vector<string> & parent){
	string w = getWord(_w, parent);
	for (int i = 8; i < 16; ++i) {
		if (regTable[i] == w) {
			regAcessTime[i] = regTimeCount++;
			return "$t" + to_string(i-8);
		}
	}
	int slot = 8;
	for (int i = 8; i < 16; ++i) {
		if (regAcessTime[i] < regAcessTime[slot]) slot = i;
	}
	if (regAcessTime[slot] != -1 && _w.find("@") != -1) {
		tarFile << string("sw $t") + to_string(slot-8) + ", " + w << endl;
		stored.insert(w);
	}
	regAcessTime[slot] = regTimeCount++;
	regTable[slot] = w;
	if (_w.find("@") == -1 || stored.find(w) != stored.end()) {
		tarFile << string("lw $t") + to_string(slot-8) + ", " + w << endl; 
	} 
	return "$t" + to_string(slot-8);
}

class COMPILE{
public:
	WORDPARSER wordReader;
	WORDTABLE wordManager;
	COMPILE(const string & source_name) : 
		wordReader(source_name), 
		tmpCount(0),
		ifCount(0),
		campCount(0),
		whileCount(0){}
	string gBlock;

	set<string> funcMap;
	vector<string> parent;
	pair<string, string> holdWord;
	int tmpCount;
	int campCount;
	int ifCount;
	int whileCount;

	string getTmp(){
		return "tmp_@" + to_string(tmpCount++);
	}

	string getIfName() {
		return "IF_" + to_string(ifCount++);
	}

	string getWhileName(){
		return "WHILE_" + to_string(whileCount++);
	}

	string getCampName() {
		return "camp_" + to_string(campCount++);
	}

	int equal(pair<string, string> & A, const string & B) {
		if (A.first == B) {
			A = wordReader.getWord();
			return true;
		}
		return false;
	}

	int errorReport(const string & expWord, const string & readWord) {
		cout << "in line " << wordReader.getLine() << string("   expect ") + expWord + " but read " + readWord << endl;
		return 0;
	}

	int realParaList_Parser(int & paraCount, vector<string> & para){
		if (holdWord.first != ",") return true;
		holdWord = wordReader.getWord();
		string res = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + res);
		para.push_back(res);
		if (!exprParser(res)) {
			cout << "in line " << wordReader.getLine() << "expr parse error" << endl;
			return false;
		}
		paraCount += 4;
	/*	string stackTmp = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + stackTmp);
		tmpFile << " push " + res << endl;
		tarFile << " sw " + wordManager.getWord(stackTmp, parent) + ", " + wordManager.getReg(res, parent) << endl;
		cout << __FILE__ << ":" << __LINE__ << " push " + res << endl;*/
		if (!realParaList_Parser(paraCount, para)) {
			cout << "in line " << wordReader.getLine() << " real para parse error" << endl;
			return false;
		}
		return true;
	}

	int realParaListParser(int & paraCount, vector<string> & para){
		string res = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + res);
		para.push_back(res);
		if (!exprParser(res)) {
			cout << "in line " << wordReader.getLine() << " real para list parse error" << endl;
			return false;
		}
		paraCount += 4;
	/*	string stackTmp = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + stackTmp);
		cout << __FILE__ << ":" << __LINE__ << " push " + res << endl;
		tmpFile << "push " + res << endl;
		tarFile << " sw " + wordManager.getWord(stackTmp, parent) + ", " + wordManager.getReg(res, parent) << endl;*/
		if (!realParaList_Parser(paraCount, para)) {
			cout << "in line " << wordReader.getLine() << " real para list parse error" << endl;
			return false;
		}
		return true;
	}

	int realParaParser(int & paraCount, vector<string> & para){
		if (realParaListParser(paraCount, para)) return true;
		return true;
	}

	int callParser(const string & res, const string & funName){
		if (holdWord.first != "(" ) return false;
		holdWord = wordReader.getWord();
		int paraCount = 0;
		vector<string> paraL;
		if (!realParaParser(paraCount, paraL)) {
			cout << "in line " << wordReader.getLine() << " real para list parse error" << endl;
			return false;
		}
		if (holdWord.first != ")") {
			return errorReport(")", holdWord.second);
		}
		if (funcMap.find("fullFunc_" + funName + to_string(paraL.size())) == funcMap.end()) {
			cout << "in line " + to_string(wordReader.getLine()) + ", function " + funName 
				+ " is undefined" << endl;
			return false;
		}
		for (int i = 0; i < paraL.size(); ++i) {
			string pTmp = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + pTmp);
			tarFile << "sw " << wordManager.getReg(paraL[i], parent) + ", " + wordManager.getWord(pTmp, parent) << endl;
		}
		tarFile << string("addi $s0, $s0, ") + to_string(wordManager.localCount - paraCount) << endl;
		tarFile << "jal " + string("fullFunc_") + funName + to_string(paraL.size()) << endl;
		tarFile << string("subi $s0, $s0, ") + to_string(wordManager.localCount - paraCount) << endl;
		cout << __FILE__ << ":" << __LINE__ << " there need some call operator" << endl;
		tmpFile << " call " << funName << endl;
		tarFile << string("move ") + wordManager.getReg(res, parent) + ", $v0" << endl;
		holdWord = wordReader.getWord();
		return true;
	}

	int FTYPEparser(const string & res, const string & identity){
		if (callParser(res, identity)) {
			return true;
		}
		cout << __FILE__ << ":" << __LINE__ << " " << string("mov ") + res + ", " + identity << endl;
		tmpFile << string("mov ") + res + ", " + identity << endl;
		tarFile << string("move ") + wordManager.getReg(res, parent) + ", " + wordManager.getReg(identity, parent) << endl;
		return true;
	}

	int factorParser(const string & res){
		if (holdWord.first == "number") {
			cout << __FILE__ << ":" << __LINE__ << " " << string("mov ") + res + ", " + holdWord.second << endl;
			tmpFile << string("mov ") + res + ", " + holdWord.second << endl;
			tarFile << string("addi ") + wordManager.getReg(res, parent) + ", $0 ," + holdWord.second << endl;
			holdWord = wordReader.getWord();
			return true;
		}
		if (holdWord.first == "(") {
			holdWord = wordReader.getWord();
			if (!exprParser(res)) {
				cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
				return false;
			}
			if (holdWord.first != ")") return errorReport(")", holdWord.second);
			holdWord = wordReader.getWord();
			return true;
		}
		if (holdWord.first == "identity") {
			string identity = holdWord.second;
			holdWord = wordReader.getWord();
			return FTYPEparser(res, identity);
		}

		/*if (holdWord.first == "identity") {
			holdWord = wordReader.getWord();
			return FTYPEparser();
		} */
		return errorReport("factor", holdWord.second);
	}

	int item_Parser(const string & res){
		if (holdWord.first == "*") {
			holdWord = wordReader.getWord();
			string facRes = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + facRes);
			if (!factorParser(facRes)) {
				cout << "in line " << wordReader.getLine() << ", factor parse error" << endl;
				return false;
			}
			cout << __FILE__ << ":" << __LINE__ << " " << string("mul ") + res + ", " + facRes + ", " + res << endl;
			tmpFile << string("mul ") + res + ", " + facRes + ", " + res << endl;
			tarFile << string("mult ") + wordManager.getReg(res, parent) + " " + wordManager.getReg(facRes, parent) << endl;
			tarFile << string("mflo ") + wordManager.getReg(res, parent) << endl;
			return item_Parser(res);
		}
		if (holdWord.first == "/") {
			holdWord = wordReader.getWord();
			string facRes = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + facRes);
			if (!factorParser(facRes)) {
				cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
				return false;
			}
			cout << __FILE__ << ":" << __LINE__ << " " << string("div ") + res + ", " + facRes + ", " + res << endl;
			tmpFile << string("div ") + res + ", " + facRes + ", " + res << endl;
			tarFile << string("div ") + wordManager.getReg(res, parent) + " " + wordManager.getReg(facRes, parent) << endl;
			tarFile << string("mflo ") + wordManager.getReg(res, parent) << endl;
			return item_Parser(res);
		}
		return true;
	}

	int plusExpr_Parser(const string & res){
		if (holdWord.first == "+") {
			holdWord = wordReader.getWord();
			string plusTmp = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + plusTmp);
			if (!itemParser(plusTmp)) {
				cout << "in line " << wordReader.getLine() << ", item parse error" << endl;
				return false;
			}
			cout << __FILE__ << ":" << __LINE__ << " add " + res + ", " + plusTmp + ", " + res << endl;
			tmpFile << "add " + res + ", " + plusTmp + ", " + res << endl;
			tarFile << "add " + wordManager.getReg(res, parent) + ", " + wordManager.getReg(plusTmp, parent) + 
				", " + wordManager.getReg(res, parent) << endl;
			if (!plusExpr_Parser(res)) {
				cout << "in line " << wordReader.getLine() << ", plus expr parse error" << endl;
				return false;
			}
			return true;
		}
		if (holdWord.first == "-") {
			holdWord = wordReader.getWord();
			string plusTmp = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + plusTmp);
			if (!itemParser(plusTmp)) {
				cout << "in line " << wordReader.getLine() << ", item parse error" << endl;
				return false;
			}
			cout << __FILE__ << ":" << __LINE__ << " sub " + res + ", " + plusTmp + ", " + res << endl;
			tmpFile << "sub " + res + ", " + plusTmp + ", " + res << endl;
			tarFile << "sub " + wordManager.getReg(res, parent) + ", " + wordManager.getReg(res, parent) + 
				", " + wordManager.getReg(plusTmp, parent) << endl;
			if (!plusExpr_Parser(res)) {
				cout << "in line " << wordReader.getLine() << ", plus expr parse error" << endl;
				return false;
			}
			return true;
		}
		return true;
	}

	int itemParser(const string & res){
		if (!factorParser(res)) {
			cout << "in line " << wordReader.getLine() << ", factor parse error" << endl;
			return false;
		}
		if (!item_Parser(res)) {
			cout << "in line " << wordReader.getLine() << ", item parse error" << endl;
			return false;
		}
		return true;
	}

	int plusExprParser(const string & res){
		if (!itemParser(res)) {
			cout << "in line " << wordReader.getLine() << ", plus expr parse error" << endl;
			return false;
		}
		if (!plusExpr_Parser(res)) {
			cout << "in line " << wordReader.getLine() << ", plus expr parse error" << endl;
			return false;
		}
		return true;
	}

	int exprCmdPrint(const string & tmp, const string & cmd){
		holdWord = wordReader.getWord();
		string cTmp = getTmp(); 
		wordManager.insertLocalWord(gBlock + "_" + cTmp);
		if (!plusExprParser(cTmp)) return false;
		cout << __FILE__ << ":" << __LINE__ << " " << cmd << " " << tmp + ", " + cTmp + ", " + tmp << endl;
		tmpFile << cmd << " " << tmp + ", " + cTmp + ", " + tmp << endl;
		string campName = getCampName();
		tarFile << cmd << " " << wordManager.getReg(tmp, parent) + ", " +
			wordManager.getReg(cTmp, parent) + ", " + 
			campName + "_true" << endl;
		tarFile << "move " << wordManager.getReg(tmp, parent) << ", $0" << endl;
		tarFile << string("j ") + campName + "_end" << endl;
		tarFile << campName + "_true:" << endl;
		tarFile << "addi " << wordManager.getReg(tmp, parent) << ", $0, 1" << endl;
		tarFile << campName + "_end:" << endl;
		if (!expr_Parser(tmp)) {
			cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
			return false;
		}
		return true;
	}

	int expr_Parser(const string & tmp){
		if (holdWord.first == "<") {
			return exprCmdPrint(tmp, "blt");
		}
		if (holdWord.first == "<=") {
			return exprCmdPrint(tmp, "ble");
		}
		if (holdWord.first == ">") {
			return exprCmdPrint(tmp, "bgt");
		}
		if (holdWord.first == ">=") {
			return exprCmdPrint(tmp, "bge");
		}
		if (holdWord.first == "==") {
			return exprCmdPrint(tmp, "beq");
		}
		if (holdWord.first == "!=") {
			return exprCmdPrint(tmp, "bne");
		}
		return true;
	}

	int exprParser(const string & tmp){
		return plusExprParser(tmp) && expr_Parser(tmp);
	}

	int elseContentParser(const string & if_id){
		cout << __FILE__ << ":" << __LINE__ << " " << if_id + "_else:" << endl;
		tmpFile << if_id + "_else:" << endl;
		tarFile << if_id + "_else:" << endl;
		if (holdWord.first != "else") return true;
		holdWord = wordReader.getWord();
		return contentBlockParser(if_id + "_else");
	}

	int ifContentParser(){
		if (holdWord.first != "if") return false;
		holdWord = wordReader.getWord();
		if (holdWord.first != "(") return errorReport("(", holdWord.second);
		holdWord = wordReader.getWord();
		string tmp = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + tmp);
		string ifName = getIfName();
		if (!exprParser(tmp)) {
			cout << "in line " << wordReader.getLine() << ", if condition is error" << endl;
			return false;
		}
		if (holdWord.first != ")") return errorReport(")", holdWord.second);
		cout << __FILE__ << ":" << __LINE__ << string(" beq ") + tmp + ", $0 " + ", " + ifName + "_else" << endl;
		tmpFile << string("bne ") + tmp + ", $0 " + ", " + ifName + "_else" << endl;
		tarFile << string("beqz ") + wordManager.getReg(tmp, parent) + ", " + ifName + "_else" << endl;

		holdWord = wordReader.getWord();
		if (!contentBlockParser(ifName)) {
			cout << "in line " << wordReader.getLine() << ", content block parse error" << endl;
			return false;
		}
		cout << __FILE__ << ":" << __LINE__ << " goto " + ifName + "_end" << endl;
		tmpFile << " goto " + ifName + "_end" << endl;
		tarFile << "j " + ifName + "_end" << endl;		

		if (!elseContentParser(ifName)) {
			cout << "in line " << wordReader.getLine() << ", else parse error" << endl;
			return false;
		}
		cout << __FILE__ << ":" << __LINE__ << " " << ifName + "_end:" << endl;
		tmpFile << ifName + "_end:" << endl;
		tarFile << ifName + "_end:" << endl;
		return true;
	}

	int whileContentParser(){
		if (holdWord.first != "while") return false;
		holdWord = wordReader.getWord();
		string whileName = getWhileName();
		
		tarFile << whileName + "_loop:" << endl;
		if (holdWord.first != "(") return errorReport("(", holdWord.second);
		holdWord = wordReader.getWord();
		string tmp = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + tmp);
		if (!exprParser(tmp)) {
			cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
			return false;
		}
		cout << __FILE__ << ":" << __LINE__ << string(" beq ") + tmp + ", $0 " + ", " + whileName + "_exit" << endl;
		tmpFile << string("beq ") + tmp + ", $0 " + ", " + whileName + "_exit" << endl;
		tarFile << string("beqz ") + wordManager.getReg(tmp, parent) + ", " + whileName + "_exit" << endl;		

		if (holdWord.first != ")") return errorReport(")", holdWord.second);
		holdWord = wordReader.getWord();
		if (!contentBlockParser(whileName)) {
			cout << "in line " << wordReader.getLine() << ", content Block  parse error" << endl;
			return false;
		}
		
		cout << __FILE__ << ":" << __LINE__ << string("goto ") + whileName + "_loop:" << endl;
		tmpFile << string("goto ") + whileName + "_loop:" << endl;
		tarFile << string("j ") + whileName + "_loop" << endl;

		cout << __FILE__ << ":" << __LINE__ << " " << whileName + "_exit:" << endl;
		tmpFile << whileName + "_exit:" << endl;
		tarFile << whileName + "_exit:" << endl;
	}

	int copyContentParser(){
		if (holdWord.first != "identity") return false;
		string identity = holdWord.second;
		holdWord = wordReader.getWord();
		if (holdWord.first != "=") return errorReport("=", holdWord.second);
		holdWord = wordReader.getWord();
		string res = getTmp();
		wordManager.insertLocalWord(gBlock + "_" + res);
		if (!exprParser(res)) {
			cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
			return false;
		}
		cout << __FILE__ << ":" << __LINE__ << " mov " << identity << ", " << res << endl;
		tmpFile << "mov " << identity << ", " << res << endl;
		tarFile << "sw " << wordManager.getReg(res, parent) << ", " << wordManager.getWord(identity, parent) << endl;
		if (holdWord.first != ";") return errorReport(";", holdWord.second);
		holdWord = wordReader.getWord();
		return true;
	}

	int returnValue(){
		if (holdWord.first != ";") {
			string tmp = getTmp();
			wordManager.insertLocalWord(gBlock + "_" + tmp);
			if (exprParser(tmp) == false) {
				cout << "in line " << wordReader.getLine() << ", expr parse error" << endl;
				return false;
			}
			cout << __FILE__ << ":" << __LINE__ << " " << "ret " << tmp << endl;
			tmpFile << "ret " << tmp << endl;
			tarFile << string("move ") + "$v0, " << wordManager.getReg(tmp, parent) << endl;
		}
		for (int i = 0; i < 8; ++i) {
			string rTmp = string("reg_") + "$t" + to_string(i);
			tarFile << "lw $t" + to_string(i) + ", " + wordManager.getWord(rTmp, parent) << endl;
		}
		string rTmp = string("reg_") + "$ra";
		tarFile << "lw $ra, " + wordManager.getWord(rTmp, parent) << endl;
		
		tarFile << "jr $ra" << endl;
		if (holdWord.first == ";") {
			holdWord = wordReader.getWord();
			return true;
		}
		return false;
	}

	int returnContentParser(){
		if (holdWord.first != "return") return false;
		holdWord = wordReader.getWord();
		cout << __FILE__ << ":" << __LINE__ << " There needs write func return operator" << endl;
		tmpFile << " There needs write func return operator" << endl;
		return returnValue();
	}

	int contentParser(){
		if (ifContentParser()) { wordManager.clearReg(); return true;}
		if (whileContentParser()) { wordManager.clearReg(); return true;}
		if (copyContentParser()) { wordManager.clearReg(); return true;}
		if (returnContentParser()) { wordManager.clearReg(); return true;}
		return false;
	}

	int contentString_Parser(){
		if (contentParser() && contentString_Parser()) return true;
		return true;
	}

	int innerClaim_Parser(){
		if (holdWord.first != ";") return true;
		holdWord = wordReader.getWord();
		return innerParaClaimParser() && innerClaim_Parser();
	}

	int innerParaClaimParser(){
		if (holdWord.first != "int") return true;
		holdWord = wordReader.getWord();
		string varient = holdWord.second;
		wordManager.insertLocalWord(gBlock + "_" + varient);
		if (holdWord.first != "identity") return errorReport("identity", holdWord.second);
		cout << __FILE__ << ":" << __LINE__ << " a new local vari " << varient << endl;
		tmpFile << " a new local vari " << varient << endl;
		holdWord = wordReader.getWord();
		return true;
	}

	int contentStringParser(){
		return contentParser() && contentString_Parser();
	}

	int innerClaimParser(){
		if (innerParaClaimParser() && innerClaim_Parser()) return true;
		return true;
	}

	int contentBlockParser(const string & blockName){
		if (holdWord.first != "{") return errorReport("{", holdWord.second);
		parent.push_back(blockName);
		string lastBlock = gBlock;
		gBlock = blockName;
		int lastLocalCount = wordManager.localCount;
		holdWord = wordReader.getWord();
		cout << __FILE__ << ":" << __LINE__ << " block_" << blockName << "_begin:" << endl;
		tmpFile <<  " block_" << blockName << "_begin:" << endl;
		tarFile <<  " block_" << blockName << "_begin:" << endl;
		if (!innerClaimParser()) {
			cout << "in line " << wordReader.getLine() << ", inner claim  error" << endl;
			return false;
		}
		if (!contentStringParser()) {
			cout << "in line " << wordReader.getLine() << ", content string parse error" << endl;
			return false;
		}
		cout << __FILE__ << ":" << __LINE__ << " block_" << blockName << "_end:" << endl;
		tarFile << " block_" << blockName << "_end:" << endl;
		if (holdWord.first != "}") return errorReport("}", holdWord.second);
		holdWord = wordReader.getWord();
		gBlock = lastBlock;
		wordManager.clearReg();
		wordManager.deleteLocalWord(wordManager.localCount - lastLocalCount);
		parent.pop_back();
		return true; 
	}

	int paraParser(const string & id, int & paraNum){
		if (holdWord.first != "int") return false;
		holdWord = wordReader.getWord();
		if (holdWord.first != "identity") return errorReport("identity", holdWord.second);
		string para = holdWord.second;
		wordManager.insertLocalWord(gBlock + "_" + para);
		holdWord = wordReader.getWord();
		cout << __FILE__ << ":" << __LINE__ << " func " << id << " has para : " << para << endl;
		tmpFile << id << " has para : " << para << endl;
		++paraNum;
		return true;
	}

	int paraList_Parser(const string & id, int &paraNum){
		if (holdWord.first == ",") {
			holdWord = wordReader.getWord();
			return paraParser(id, paraNum) && paraList_Parser(id, paraNum);
		}
		return true;
	}
	
	int paraListParser(const string & id, int & paraNum){
		return paraParser(id, paraNum) && paraList_Parser(id, paraNum);	
	}

	int formParaParser(const string & id, int & paraNum){
		if (holdWord.first == "void") {
			cout << __FILE__ << ":" << __LINE__ << " func " << id << " don't have para" << endl;
			tmpFile << " func " << id << " don't have para" << endl;
			holdWord = wordReader.getWord();
			return true;
		}
		return paraListParser(id, paraNum);
	}

	int funcClaimParser(const string & id){
		string lastBlock = gBlock;
		string blockTmp = string("fullFunc_") + id;
		gBlock = blockTmp;
		parent.push_back(gBlock);
		int paraNum = 0;

		if (holdWord.first == "(") {
			holdWord = wordReader.getWord();
			if (!formParaParser(id, paraNum)) return errorReport("form para", holdWord.second);
			if (holdWord.first != ")") {
				errorReport(")", holdWord.first);
				return false;
			}
			holdWord = wordReader.getWord();
			if (funcMap.find(blockTmp + to_string(paraNum)) != funcMap.end()) {
				cout << "already define func " + id + 
					", in line define again" + to_string(wordReader.getLine()) << endl;
				return false;
			}
			funcMap.insert(blockTmp + to_string(paraNum));
			cout << __FILE__ << ":" << __LINE__ << "   add a new func " << id << endl;
			tmpFile << string("func_") + id + ":" << endl;
			tarFile << ".text " << endl;
			tarFile << "fullFunc_" + id + to_string(paraNum) + ":" << endl;
			for (int i = 0; i < 8; ++i) {
				string rTmp = string("reg_") + "$t" + to_string(i);
				wordManager.insertLocalWord(gBlock + "_" + rTmp);
				tarFile << "sw $t" + to_string(i) + ", " + wordManager.getWord(rTmp, parent) << endl;
			}
			string rTmp = string("reg_") + "$ra";
			wordManager.insertLocalWord(gBlock + "_" + rTmp);
			tarFile << "sw $ra, " + wordManager.getWord(rTmp, parent) << endl;

			wordManager.initReg();

			if (contentBlockParser(id) == false) return errorReport("contentBlock", holdWord.second);
			parent.pop_back();
			gBlock = lastBlock;
			wordManager.deleteLocalWord(36);
			return true;
		}
		errorReport("(", holdWord.first);
		return false;
	}

	int claimTypeParser(const string & id){
		if (holdWord.first == ";") {
			holdWord = wordReader.getWord();
			cout << __FILE__ << ":" << __LINE__ << "   add a globa varient " << id << endl;
			tmpFile << id << " .word 0" << endl;
			tarFile << id << ": .word 0" << endl;
			return true;
		}
		return funcClaimParser(id);
	}

	int claimParser(){
		debug();
		if (holdWord.first == "int") {
			holdWord = wordReader.getWord();
			if (holdWord.first != "identity") return errorReport("identity", holdWord.first);
			string id = holdWord.second;
			holdWord = wordReader.getWord();
			return claimTypeParser(id);
		} else if (holdWord.first == "void") {
			holdWord = wordReader.getWord();	
			string id = holdWord.second;
			if (holdWord.first != "identity") return errorReport("identity", holdWord.first);
			holdWord = wordReader.getWord();	
			return funcClaimParser(id);
		}
		debug();
		return false;
	}

	int claimStringParser(){
		debug();
		if (claimParser()) return claimStringParser();
		return true;
	}

	int programParser(){
		debug();
		return claimStringParser();
	}

	int parser() {
		holdWord = wordReader.getWord();
		debug();
		tarFile << ".text " << endl;
		tarFile << "addi $s0, $0, 0x10010000" << endl;
		tarFile << "j fullFunc_main0" << endl;
		tarFile << ".data" << endl;
		return programParser();
	}
};

int main(int argv, char ** argc){
	COMPILE test(argc[1]);
	cout << test.parser() << endl;
	/*for (;;) {
		pair<string, string> word = test.getWord();
		cout << test.getLine() << ": " << word.first << " " << word.first << endl;
		if (word.first == "#") break;
	}*/
	return 0;
}
