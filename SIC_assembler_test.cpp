// SP HW2 SIC/XE Assembler
// 資訊三乙 10827234 彭桂綺
//
// 只需輸入檔名(包含副檔名) 即可執行 ，
// 輸出結果到  "asm_SIC_output.txt"


#include <iostream> // cout, endl
#include <iomanip>	// setw
#include <fstream>	// open, is_open, close, write, ignore
#include <string>
#include <string.h>	// string, find_last_of, substr, clear
#include <cstdlib>	// atoi, system
#include <vector> 	// vector, push_back
#include <sstream>  // stringstream
#include "memory.h"
#include <algorithm>

#define HASHSIZE 100

using namespace std;

struct Index {
	int table = 0;
	int location = 0; // 由1 開始計數
};


typedef struct hT {
	string word;
	int hashValue = 0;
	bool haveData = false;
}hashTable;

class Tokens {
public:
	vector<string> variables;
	vector<vector<string> > varGroup; // 將input 資料一個個分開 一行為一組
	vector<Index> allIndex;
	vector<vector<Index> > indexGroup; // 每行token analysis 結果自成一組

	void initialize();
};

typedef class Table {
private:
	vector<string> table1_Instruction;
	vector<string> table2_Pseudo;
	vector<string> table3_Register;
	vector<string> table4_Delimiter;
	vector<hT> table5_Symbol;
	vector<hT> table6_Num;
	vector<hT> table7_String;

public:
	Table() {
		readTable("Table1.table", table1_Instruction);
		readTable("Table2.table", table2_Pseudo);
		readTable("Table3.table", table3_Register);
		readTable("Table4.table", table4_Delimiter);
	} // 讀進table1-4

	void initialize(); // 初始化 5-7 Table
	void readTable(string tableName, vector<string>& table);

	Index findTable(char* temp); // find table 1-7 except 4
	Index findDelimiterTable(char temp); // 找 Table 4
	Index insertTable(string temp, int hashNum, int option); // insert table 5 6 7

	void output(); // 呼叫writeFile
	void writeFile(vector<hT>, string);
} *Tableptr;


class DataT {
public:
	string first = "", second = "", third = "", str = "", opcode = "", format = "";
	string comment = "";
	bool error = false;

	DataT(string first_, string second_, string third_, string str_, string opcode_, string format_, bool error_) {
		first = first_;
		second = second_;
		third = third_;
		opcode = opcode_;
		format = format_;
		str = str_;
		error = error_;
	} // Data
};

class Pair {
public:
	string symbol = "";
	string location = "";


	Pair(string symbol_, string location_) {
		symbol = symbol_;
		location = location_;
	}
};

bool readFile(fstream& temp, vector<string>& lines);
int isWhiteSpaceOrDelimiter(char ch);
string toUpper(string s);
string toLower(string s);
int calHash(char* temp);

string decToHexStr(int temp);		// 10 進為 int 轉 16 進位 string
int hexStrToDecInt(string hexStr);	// 16 進位 string 轉 10 進為 int
string decToBinStr(int decimal);	// 10 進為 int 轉 2 進位 string


int main() {
	fstream input, outputF;		// 檔案物件
	char temp = '\0';	// 暫存一個一個字元
	char pseudoBYTE = '\0'; // 存 X 或 C ( X'F1' or C'EOF' )
	char cont = 'n';	// 是否繼續執行其他檔案
	int i = 0, j = 0;
	int option = -1, count = -1, hashNum = 0, loc = 0; // count: 計算 buffer 位置 // loc: 紀錄輸出token位置

	vector<string> lines; // 檔案的資訊切成一行一行

	vector<Index> tempIndex; // push 進 vector 中
	vector<string> tempWord; // push 進 vector 中
	Tokens TokenItem;
	Tableptr TableItem = new Table();
	Index index;
	char buffer[100] = { '\0' };
	bool wasQuote = false; // 當讀到 ' 將此設成 TRUE ,讀到第二次 ' 後做一些事，再將此設為FALSE

	// {Mnemonic, Format, Opcode}
	string op_TAB[][3] = { {"ADD","3","18"},{"ADDF","3","58"},{"ADDR","2","90"},{"AND","3","40"},{"CLEAR","2","B4"},
		{"COMPF","3","88"},{"COMPR","2","A0"},{"COMP","3","28"},{"DIVF","3","64"},{"DIVR","2","9C"},{"DIV","3","24"},
		{"FIX","1","C4"},{"FLOAT","1","C0"},{"HIO","1","F4"},{"J","3","3C"},{"JEQ","3","30"},{"JGT","3","34"},{"JLT","3","38"},
		{"JSUB","3","48"},{"LDA","3","00"},{"LDB","3","68"},{"LDCH","3","50"},{"LDF","3","70"},{"LDL","3","08"},{"LDS","3","6C"},
		{"LDT","3","74"},{"LDX","3","04"},{"LPS","3","E0"},{"UML","3","20"},{"MULF","3","60"},{"MULR","2","98"},{"NORM","1","C8"},
		{"OR","3","44"},{"RD","3","D8"},{"RMO","2","AC"},{"RSUB","3","4C"},{"SHIFTL","2","A4"},{"SHIFTR","2","A8"},{"SIO","1","F0"},
		{"SSK","3","EC"},{"STA","3","0C"},{"STB","3","78"},{"STCH","3","54"},{"STF","3","80"},{"STI","3","D4"},{"STL","3","14"},
		{"STSW","3","E8"},{"STS","3","7C"},{"STT","3","84"},{"STX","3","10"},{"SUBF","3","5C"},{"SUBR","2","94"},{"SUB","3","1C"},
		{"SVC","2","B0"},{"TD","3","E0"},{"TIO","1","F8"},{"TIXR","2","B8"},{"TIX","3","2C"},{"WD","3","DC"} };


	vector<DataT> Data;
	vector<string> Length;
	vector<string> Location;
	vector<Pair> SYM_TAB;
	vector<string> Target;

	int n = 0, DecLoc = 4096, k = 0, len = 0;
	string HexLoc = "0", str1 = " ", str2 = " ", str3 = " ", op = " ", format = " ", base = " ";
	bool isOpCode = false, isLine = false, error = false;
	bool had1 = false, had2 = false, had3 = false;

	cout << " # 資訊三乙 10827234 彭桂綺 " << endl << endl;
	cout << "╔═══════════════════════════════════╗" << endl;
	cout << "║  SP Homework2 : SIC/XE Assembler  ║" << endl;
	cout << "╚═══════════════════════════════════╝" << endl;

	do {

		/* Initialize */ TokenItem.initialize(), TableItem->initialize();
		/* Initialize */ lines.clear(), Data.clear(), Length.clear(), Location.clear(), SYM_TAB.clear(), Target.clear();
		/* Initialize */ loc = 0, count = -1, DecLoc = 4096, len = 0;

		if (readFile(input, lines)) { 	// read records from a file

			cout << "### LexcicalAnalysis ###" << endl;
			outputF.open("SIC_output.txt", ios::out | ios::trunc);

			for (i = 0; i < lines.size(); i++) {
				//outputF << lines.at(i).substr(0, lines.at(i).size() - 1) << endl;
				//cout << lines.at(i).substr(0, lines.at(i).size() - 1) << endl;

				TokenItem.varGroup.push_back(tempWord);
				TokenItem.indexGroup.push_back(tempIndex);
				wasQuote = false;

				//用于存放分割后的字符串 
				// TokenItem.varGroup.at(i)
				// 
				//待分割的字符串，含有很多空格 
				string tempLine = lines.at(i);
				//暂存从word中读取的字符串 
				string result;
				//将字符串读到input中 
				stringstream processString(tempLine);
				//依次输出到result中，并存入res中 
				while (processString >> result)
					TokenItem.varGroup.at(i).push_back(result);
				//输出res 
				/*
				for (int k = 0; k < TokenItem.varGroup.at(i).size(); k++)
					cout << TokenItem.varGroup.at(i)[k] << endl;
				*/

				for (j = 0; j < lines.at(i).size(); j++) {
					temp = lines.at(i).at(j); // 將 一行資料的 第 j 個字元 assign 給 temp

					option = isWhiteSpaceOrDelimiter(temp);
					switch (option) {
					case 0: // 存進 buffer
						buffer[++count] = temp;
						break;

					case 1: // 是 Delimiter
						if (wasQuote && temp == '\'') { // 之前讀到的東西是由兩個單引號 (') 夾起來的
							// insert string hash table
							if (strcmp(buffer, "\0")) { // buffer 有東西
								hashNum = calHash(buffer); // hash function

								if (pseudoBYTE == 'X')  // 16進位數字
									index = TableItem->insertTable(buffer, hashNum, 2);
								else if (pseudoBYTE == 'C')  // 字串 pseudoBYTE == 'C' 
									index = TableItem->insertTable(buffer, hashNum, 3);
								else // or 其他 symbol
									index = TableItem->insertTable(buffer, hashNum, 1);

								TokenItem.variables.push_back(buffer);
								TokenItem.allIndex.push_back(index);
								TokenItem.indexGroup.at(i).push_back(index);
							} // if

							index = TableItem->findDelimiterTable(temp);
							TokenItem.variables.push_back(buffer);
							TokenItem.allIndex.push_back(index);
							TokenItem.indexGroup.at(i).push_back(index);

							pseudoBYTE = '\0';
							wasQuote = false;
						} // else
						else {
							if (strcmp(buffer, "\0") != 0 && strcmp(buffer, "X") != 0 && strcmp(buffer, "C") != 0) {
								index = TableItem->findTable(buffer);
								TokenItem.variables.push_back(buffer);
								TokenItem.allIndex.push_back(index);
								TokenItem.indexGroup.at(i).push_back(index);
							} // if

							if (strcmp(buffer, "X") != 0 || strcmp(buffer, "C") != 0)
								pseudoBYTE = buffer[0];

							index = TableItem->findDelimiterTable(temp);
							TokenItem.variables.push_back(buffer);
							TokenItem.allIndex.push_back(index);
							TokenItem.indexGroup.at(i).push_back(index);


							if (temp == '.') // 註解
								j = lines.at(i).size(); // 後面直接不讀

							if (temp == '\'')
								wasQuote = true;
						} // else

						// 清除buffer
						count = -1;
						memset(buffer, '\0', sizeof(buffer)); // 將buffer 中所有位元變成 '\0'
						break;

					case 2: // 是 WhiteSpace
						// 呼叫 找hashtable function
						if (strcmp(buffer, "\0") != 0) {
							index = TableItem->findTable(buffer);
							TokenItem.variables.push_back(buffer);
							TokenItem.allIndex.push_back(index);
							TokenItem.indexGroup.at(i).push_back(index);
						} // if

						// 清除buffer
						count = -1;
						memset(buffer, '\0', sizeof(buffer)); // 將buffer 中所有位元變成 '\0'
						break;

					} // switch

				} // inner for

				/*
				for (loc; loc < TokenItem.allIndex.size(); loc++) { //
					outputF << "(" << TokenItem.allIndex.at(loc).table << "," << TokenItem.allIndex.at(loc).location << ")";
				} // for

				outputF << endl;
				*/
			} // outter for

			cout << "=== Complete analysis ===" << endl << endl;
			cout << "### Assembler ###" << endl;

			/*
			for (i = 0; i < TokenItem.varGroup.size(); i++) {
				for (j = 0; j < TokenItem.varGroup.at(i).size(); j++) {
					//cout << "(" << TokenItem.indexGroup.at(i).at(j).table << "," << TokenItem.indexGroup.at(i).at(j).location << ")";
					cout << TokenItem.varGroup.at(i).at(j) << "<>";
				} // for

				cout << endl;
			} // for
			*/

			for (i = 0; i < TokenItem.varGroup.size(); i++) {
				for (j = 0; j < TokenItem.varGroup.at(i).size(); j++) {
					string tempString = TokenItem.varGroup.at(i).at(j); // 處理某一行的一個 string

					if (tempString.compare("START") == 0 || tempString.compare("END") == 0 || tempString.compare("WORD") == 0 || tempString.compare("BYTE") == 0 || tempString.compare("RESB") == 0 || tempString.compare("RESW") == 0 || tempString.compare("BASE") == 0) {
						if (had2)
							error = true;
						else {
							str2 = tempString;
							had2 = true;
							isOpCode = true;
						}
					} // if
					else {
						string s = tempString;
						if (tempString.find("+") != string::npos) { // tempString 含有 +
							s = tempString.substr(1, tempString.length());
							len = 1;
						} // if
						for (int cc = 0; cc < 59; cc++) { // op_TAB has 59 elements
							if (s.compare(op_TAB[cc][0]) == 0) {
								if (had2 == true)
									error = true;
								else {
									str2 = tempString; //放入second
									had2 = true;
									op = op_TAB[cc][2];
									len = len + stoi(op_TAB[cc][1]); //放入長度
									format = to_string(len);
									isOpCode = true;
								} // else

								break;
							} // if
						} // for

					} // else
					

					if (str2.compare("RSUB") == 0)
						isLine = true;

					if (tempString.compare(".") == 0) { // \改變位置/
						str1 = ".";
						error = true;
						isLine = true;
						/*
						if (had1)
							error = true;
						else {
							str1 = ".";
							had1 = true;
							isLine = true;
						} // else
						*/
					} // if
					else if (!isOpCode){
						if (had1)
							error = true;
						else {
							str1 = tempString;
							had1 = true;
						} // else
					} // if

					else if (!str2.compare("RSUB") == 0 && !str1.compare(".") == 0) { // 已經有 operand 且非 RSUB
						
						if (had3)
							error = true;
						
						else {
							if(j + 1 >= TokenItem.varGroup.at(i).size())  // 少東西
								error = true;
							else {
								int anotherOP = false;
								for (int cc = 0; cc < 59; cc++) { // op_TAB has 59 elements
									if (TokenItem.varGroup.at(i).at(j + 1).compare(op_TAB[cc][0]) == 0) {
										anotherOP = true;
										break;
									} // if
								} // for

								if (anotherOP)
									error = true;
								else {
									str3 = TokenItem.varGroup.at(i).at(++j);
									had3 = true;
									isLine = true;
								} // else
							} // else
						} // else

					} // else if


					if (isLine) {
						Data.push_back(DataT(str1, str2, str3, str1 + str2 + str3, op, format, error));
						if (error)
							len = 0;
						Length.push_back(to_string(len));
						str1 = " "; str2 = " "; str3 = " "; len = 0; op = " "; format = " ", error = false;
						isLine = false;
						isOpCode = false;
						had1 = false, had2 = false, had3 = false;
					} // if

				}// inner for

			} // outter for()

			for (i = 0; i < Data.size(); i++) {
				// 計算位置//
				if (Data.at(i).str.find(".") != string::npos || Data.at(i).str.find("BASE") != string::npos) {
					if (Data.at(i).str.find("BASE") != string::npos) // 包含BASE字串
						base = Data.at(i).third;//先儲存base暫存器的運算元之後利用SYM_TAB找出位置(Base Relative)
					Location.push_back("");

					HexLoc = decToHexStr(DecLoc += hexStrToDecInt(Length.at(i - 1)));
					// HexLoc = (Integer.toHexString(DecLoc += Integer.parseInt(Length.at(i - 1), 16))).toUpperCase();
				} // if
				else {
					if (i > 1) {
						HexLoc = decToHexStr(DecLoc += hexStrToDecInt(Length.at(i - 1)));
						// HexLoc = (Integer.toHexString(DecLoc += Integer.parseInt(Length.at(i - 1), 16))).toUpperCase();

						if (i == Data.size() - 1)//END無位置
							HexLoc = "";
					} // if
					else// 起始位置從1開始
						HexLoc = "1000";
					Location.push_back(HexLoc);
				} // else

				//計算長度
				if (Data.at(i).second.compare("BYTE") == 0) {
					if (Data.at(i).third.find("C") != string::npos) {// 當C'EOF'時長度3
						char c[50] = { '\0' };
						strcpy_s(c, Data.at(i).third
							.substr(Data.at(i).third.find('\'') + 1, Data.at(i).third.length() - 3)
							.c_str());
						Length.erase(Length.begin() + i);
						Length.insert(Length.begin() + i, to_string(strlen(c)));
						//cout << c << "< strlen:: " << to_string(strlen(c)) << endl;
					} // if
					else {
						Length.erase(Length.begin() + i);
						Length.insert(Length.begin() + i, "1");// 當X'F1'時長度1
					} // else
				} // fi
				else if (Data.at(i).second.find("RESW") != string::npos) {// 當RESW時數字*3
					Length.erase(Length.begin() + i);

					string hexStr;
					hexStr = decToHexStr(stoi((Data.at(i).third)) * 3);
					Length.insert(Length.begin() + i, hexStr);
					//Length.insert(Length.begin() + i, Integer.toHexString(Integer.parseInt(Data.at(i).third) * 3));
				} // else if
				else if (Data.at(i).second.find("RESB") != string::npos) {
					Length.erase(Length.begin() + i);

					string hexStr;
					hexStr = decToHexStr(stoi(Data.at(i).third));
					Length.insert(Length.begin() + i, hexStr);
					//Length.insert(Length.begin() + i, Integer.toHexString(Integer.parseInt(Data.at(i).third)));
				} // else if
				else if (Data.at(i).second.find("WORD") != string::npos) {  // WORD 3
					Length.erase(Length.begin() + i);
					Length.insert(Length.begin() + i, "3");
				} // else if
				else if (Data.at(i).second.find("CLEAR") != string::npos) {  // CLEAR 2
					Length.erase(Length.begin() + i);
					Length.insert(Length.begin() + i, "2");
				} // else if

				// 建立SYM_TAB//
				if ((Data.at(i).first.find(" ") == string::npos) && (i != 0) && (Data.at(i).first.find(".") == string::npos)) { // 不包含 " ", "."
					if (Data.at(i).first.compare(base) == 0)
						base = HexLoc;
					SYM_TAB.push_back(Pair(Data.at(i).first, HexLoc));
				} // if

			} // for


			/// pass2建立目的碼///
			for (i = 0; i < Data.size(); i++) {
				string s;
				if (Data.at(i).format.compare("2") == 0 || Data.at(i).format.compare("3") == 0 || Data.at(i).format.compare("4") == 0) {
					string str = "", num = "";
					for (k = 0; k < SYM_TAB.size(); k++) {
						if (Data.at(i).third.find(SYM_TAB.at(k).symbol) != string::npos) {
							num = SYM_TAB.at(k).location;

							s = s + Data.at(i).opcode + num;
							break;
						} // if

					} // if


					if (s.size() == 0) {
						if (Data.at(i).second.compare("RSUB") == 0)
							s = s + Data.at(i).opcode + "0000";
					} // if

					s.append(str);

				} // if


				if (Data.at(i).second.compare("BYTE") == 0) {
					string str = "";
					char c[50] = { '\0' };
					strcpy_s(c, Data.at(i).third.substr(Data.at(i).third.find('\'') + 1, Data.at(i).third.length() - 3).c_str());
					for (k = 0; k < strlen(c); k++) {
						if (Data.at(i).third.find("C") != string::npos) {

							str += decToHexStr(c[k]);
							//str += (Integer.toHexString(c[k]).toUpperCase());// ASCii由10->16目的碼
						} // if
						else
							str += (c[k]);
					} // for

					s.append(str);
				} // if
				if (Data.at(i).second.compare("WORD") == 0) {
					string str = decToHexStr(stoi(Data.at(i).third));
					stringstream ss;
					ss << setw(6) << setfill('0') << str;
					ss >> str;
					s.append(str);
				}  // if

				Target.push_back(s);
				//if (i == i)
				//	cout << i << " Target:" << Target.at(i) << endl;

			} // for

			cout << std::left << "Line\t" << setw(10) << "Loc" << setw(35) << "Source statement" << setw(15) << "Object code" << endl << endl;
			outputF << std::left << "Line\t" << setw(10) << "Loc" << setw(35) << "Source statement" << setw(15) << "Object code" << endl << endl;
			for (j = 0; j < Data.size(); j++) {
				cout << setw(3) << (j + 1) * 5 << "\t";
				outputF << setw(3) << (j + 1) * 5 << "\t";
				if (Data.at(j).error == false) {
					cout << std::left << setw(10) << Location.at(j) << setw(10) << Data.at(j).first << setw(10) << Data.at(j).second << setw(15) <<
						Data.at(j).third << setw(10) << Target.at(j) << endl;
					outputF << std::left << setw(10) << Location.at(j) << setw(10) << Data.at(j).first << setw(10) << Data.at(j).second << setw(15) <<
						Data.at(j).third << setw(10) << Target.at(j) << endl;
				} //eif
				else {
					if (Data.at(j).first.compare(".") == 0) {
						cout << std::left << lines.at(j + 1);
						outputF << std::left << lines.at(j + 1);
					} // if
					else {
						cout << "\t此行錯誤!\t" << std::left << lines.at(j + 1);
						outputF << "\t此行錯誤!\t" << std::left << lines.at(j + 1);
					} // else
				} // else
			} // for

		} // if


		outputF.close();
		TableItem->output(); // 輸出 table5-7
		cout << endl << "###### EXECUTION SUCCEED ######" << endl;
		cout << endl << "Do you want to process other files? (y/N): ";
		cin >> cont;
	} while (cont == 'y' || cont == 'Y');

	system("pause");
	return 0;
} // main

bool readFile(fstream& temp, vector<string>& lines) { // read records from a file
	string fileName = "";
	do {
		cout << endl << "Input a file name (0:quit!): ";
		cin >> fileName;

		if (fileName == "0") // quit
			return false;

		temp.open(fileName, ios::in);

		if (!temp)  // 找不到檔案!
			cout << endl << "### " << fileName << " does not exist! ###" << endl;
	} while (!temp);

	string  line; //儲存讀入的每一行
	while (getline(temp, line)) {//會自動把\n換行符去掉
		line += "\n";
		lines.push_back(line);
	} // while

	temp.close();
	return true;

} // readInputFile()

int isWhiteSpaceOrDelimiter(char ch) {
	if (ch == ',' || ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
		ch == ':' || ch == ';' || ch == '?' || ch == '.' || ch == '=' ||
		ch == '#' || ch == '@' || ch == '\'')
		return 1;
	else if (ch == ' ' || ch == '\t' || ch == '\n')
		return 2;
	else
		return 0;
} // isWhiteSpaceOrDelimiter()

string toUpper(string s) {
	transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return toupper(c); });
	return s;
} // toUpper(string s)

string toLower(string s) {
	transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
	return s;
} // toUpper(string s)

string decToHexStr(int temp) {
	stringstream ss;
	ss << std::hex << temp; // 10 進為 int 轉 16 進位 string

	return toUpper(ss.str());
} // 

int hexStrToDecInt(string hexStr) { // 16進位string 轉 10 進為 int
	stringstream ss;
	int x;
	ss << std::hex << hexStr;
	ss >> x; // 16進位string 轉 10 進為 int

	return x;
} // 

string decToBinStr(int decimal) { // 10進位 轉為2進位字串
	char a[20] = { '\0' };
	int i, temp = decimal;

	if (decimal == 0)
		a[0] = '0';
	else {
		for (i = 0; decimal > 0; i++) {
			a[i] = decimal % 2 + '0';
			decimal = decimal / 2;
		} // for
	} // else

	string str(a);
	std::reverse(str.begin(), str.end());
	// cout << endl << "decimal:" << temp << "<   BIN:" << str << endl << endl;
	return str;
} // 

int calHash(char* temp) {
	int ASCIISum = 0;
	string t1(temp);

	for (int cc = 0; cc < t1.length(); cc++)  // 算出 ASCII 合
		ASCIISum += temp[cc];
	return ASCIISum % HASHSIZE; // hash function
} // calHash

void Tokens::initialize() {
	this->allIndex.clear();
	this->indexGroup.clear();
	this->variables.clear();
	this->varGroup.clear();
} // Tokens::initialize()

void Table::initialize() {
	this->table5_Symbol.clear();
	this->table5_Symbol.resize(HASHSIZE);
	this->table6_Num.clear();
	this->table6_Num.resize(HASHSIZE);
	this->table7_String.clear();
	this->table7_String.resize(HASHSIZE);

} // Table::initialize()

void Table::readTable(string tableName, vector<string>& table) {
	fstream input;
	input.open(tableName, ios::in);

	if (!input) { // 找不到檔案!
		cout << endl << "### " << tableName << " does not exist! ###" << endl;
	} // if
	else {
		string  line; // 儲存讀入的一行資訊
		while (getline(input, line)) { // 會自動把\n換行符去掉
			table.push_back(line);
		} // while

		input.close();
	} // else
} // Table::readTable(string tableName, vector<string>& table)

Index Table::findTable(char* temp) {
	// 回傳index 位置
	// 123table大小寫相同!!!!!!
	Index index;
	string t1(temp); // convert char* to string
	string upperT = toUpper(t1); // 將輸入字串轉成大寫
	string lowerT = toLower(t1); // 將輸入字串轉成小寫
	int i = 0, ASCIISum = 0, hashNum = 0;
	bool found = false;

	if (!found) {
		for (i = 0; i < this->table1_Instruction.size(); i++) {
			if (upperT == this->table1_Instruction.at(i) || lowerT == this->table1_Instruction.at(i)) {
				index.table = 1;
				index.location = i + 1;
				found = true;
			} // if
		} // for
	} // if

	if (!found) {
		for (i = 0; i < this->table2_Pseudo.size(); i++) {
			if (upperT == this->table2_Pseudo.at(i) || lowerT == this->table2_Pseudo.at(i)) {
				index.table = 2;
				index.location = i + 1;
				found = true;
			} // if
		} // for
	} // if

	if (!found) {
		for (i = 0; i < this->table3_Register.size(); i++) {
			if (upperT == this->table3_Register.at(i) || lowerT == this->table3_Register.at(i)) {
				index.table = 3;
				index.location = i + 1;
				found = true;
			} // if
		} // for
	} // if


	if (!found) {

		hashNum = calHash(temp); // hash function

		if (temp[0] >= '0' && '9' >= temp[0])
			index = Table::insertTable(t1, hashNum, 2);// Num
		else
			index = Table::insertTable(t1, hashNum, 1); // Symbol

	} // if

	return index;
} // Table::findTable(char *temp)()

Index Table::insertTable(string temp, int hashNum, int option) {
	Index index;
	int i = 0, j = 0, location = hashNum; // count 計算放的位置  // hashNum 不用 -1
	bool found = false;

	switch (option) {
	case 1: // Symbol
		for (i = 0; i < this->table5_Symbol.size(); i++) {
			if (temp == this->table5_Symbol.at(i).word) {
				index.table = 5;
				index.location = i;
				found = true;
			} // if
		} // for

		while (!found) {
			if (location >= HASHSIZE)
				location = 0;

			if (this->table5_Symbol.at(location).hashValue == false && location < HASHSIZE) { //此位置為空
				this->table5_Symbol.at(location).haveData = true;		// 這個設為非空
				this->table5_Symbol.at(location).word = temp;			// 將字存入
				this->table5_Symbol.at(location).hashValue = hashNum;	// 將 哈希值存入
				index.location = location;
				index.table = 5;
				break;
			} // if

			location++;
		} // while

		break;

	case 2: // Num
		for (i = 0; i < this->table6_Num.size(); i++) {
			if (temp == this->table6_Num.at(i).word) {
				index.table = 6;
				index.location = i;
				found = true;
			} // if
		} // for

		while (!found) {
			if (location >= HASHSIZE)
				location = 0;

			if (this->table6_Num.at(location).hashValue == false && location < HASHSIZE) { //此位置為空
				this->table6_Num.at(location).haveData = true;		// 這個設為非空
				this->table6_Num.at(location).word = temp;			// 將字存入
				this->table6_Num.at(location).hashValue = hashNum;	// 將 哈希值存入
				index.location = location;
				index.table = 6;
				break;
			} // if

			location++;
		} // while
		break;

	case 3: // String
		for (i = 0; i < this->table7_String.size(); i++) {
			if (temp == this->table7_String.at(i).word) {
				index.table = 7;
				index.location = i;
				found = true;
			} // if
		} // for
		while (!found) {
			if (location >= HASHSIZE)
				location = 0;

			if (this->table7_String.at(location).hashValue == false && location < HASHSIZE) { //此位置為空
				this->table7_String.at(location).haveData = true;		// 這個設為非空
				this->table7_String.at(location).word = temp;			// 將字存入
				this->table7_String.at(location).hashValue = hashNum;	// 將 哈希值存入
				index.location = location;
				index.table = 7;
				break;
			} // if

			location++;
		} // while
		break;

	default: index.location = 0;	index.table = 0;
	} // switch

	return index;
} // Table::insertTable()

Index Table::findDelimiterTable(char temp) {
	Index index;
	int i = 0;
	//cout << "table:" << temp << endl;

	for (i = 0; i < this->table4_Delimiter.size(); i++) {
		if (temp == this->table4_Delimiter.at(i).c_str()[0]) {
			index.table = 4;
			index.location = i + 1;
		} // if
	} // for

	return index;
} // Table::findDelimiterTable()

void Table::writeFile(vector<hT> temp, string fileName) {
	fstream outfile;
	outfile.open(fileName, ios::out | ios::trunc); // open a file

	if (outfile.is_open()) {
		for (int i = 0; i < temp.size(); i++) {
			outfile << temp.at(i).word << endl;
		} // for
	} // if

	outfile.close();
} // Table::writeFile(vector<hT> temp, string fileName)

void Table::output() {
	this->writeFile(this->table5_Symbol, "Table5.table");
	this->writeFile(this->table6_Num, "Table6.table");
	this->writeFile(this->table7_String, "Table7.table");
} // Table::output()


