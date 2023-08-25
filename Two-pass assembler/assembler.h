#ifndef assembler
#define assembler

#include<bits/stdc++.h>
using namespace std;

#define ILEN 1          // Instruction length
#define WLEN 1          // Word length

#define MAXS 1024
#define MAXW 10

int ERROR = 0;

int PROGLEN;            // store program length
char PROGNAME[MAXS];    // store program length
int STARTADDR;          // store program length

unordered_map<string, string> optab;
unordered_map<string, string> symtab;
unordered_map<string, string> littab;
vector<string>tempLiterals;

// Utility files
const char *SYMTAB = "symbolTable.txt";
const char *LITTAB = "literalTable.txt";
const char *OPTAB = "opcodeTable.txt";
const char *INTM = "intermediate.txt";

// Utility functions
int break_line(char *str, int start, char *words[]);
const char *search_symtab(char *LABEL);
int insert_symtab(char *LABEL, int LOCCTR);
int insert_littab(string LIT, int LOCCTR);
const char *search_optab(char *OPCODE);
int insert_temp_literals(char *LITERAL);
vector<string> get_temp_literals();
const char *search_littab(char *lit);

// Funtions for pass 1 and 2
int pass_one(FILE*, FILE*);
int pass_two(FILE*, FILE*, FILE*);

#endif
