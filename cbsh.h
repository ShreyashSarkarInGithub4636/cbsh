#ifndef CBSH_H
#define CBSH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>
#include "config.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 256
#define MAX_NUM_LINES 1000
#define MAX_VARIABLES 100
#define MAX_DATA_VALUES 1000

// Token types
typedef enum {
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_EOF,
    TOKEN_NEWLINE,
    TOKEN_COLON
} TokenType;

// Keywords
typedef enum {
    KW_LIST, KW_NEW, KW_PRINT, KW_INPUT, KW_IF, KW_THEN, KW_FOR, KW_NEXT,
    KW_SQR, KW_RND, KW_SIN, KW_LET, KW_USR, KW_DATA, KW_READ, KW_REM,
    KW_CLEAR, KW_STOP, KW_TAB, KW_RESTORE, KW_ABS, KW_END, KW_INT,
    KW_RETURN, KW_STEP, KW_GOTO, KW_GOSUB, KW_SET, KW_TO, KW_RUN, KW_NONE,
    kW_LOAD, KW_DIR
} Keyword;

// A structure to represent a token
typedef struct {
    TokenType type;
    Keyword keyword; // If it's a keyword
    char value[MAX_LINE_LENGTH]; // Store number, string, identifier, etc.
} Token;

// A structure to represent a program line
typedef struct {
    int lineNumber;
    Token tokens[MAX_LINE_LENGTH]; // Array of tokens for this line
    int numTokens;
} Line;

// Variable data types (not fully implemented!!!!)
typedef enum {
    VAR_TYPE_NUMERIC,
    VAR_TYPE_STRING
} VarType;

// A structure to represent a variable
typedef struct {
    char name[MAX_LINE_LENGTH];
    VarType type;
    double numValue; // Store numeric values
    char strValue[MAX_LINE_LENGTH]; // Store string values
    
    int forNextLine;  // Line number of the matching NEXT
    double forStep;   // Step value of the loop
    double forEnd;    // End value of the loop
    int forStartLine; // Line number of the FOR statement
} Variable;

// Global data structures
extern Line program[MAX_NUM_LINES];
extern int numLines;
extern Variable variables[MAX_VARIABLES];
extern int numVariables;
extern double dataValues[MAX_DATA_VALUES];
extern int numDataValues;
extern int dataReadPtr; // Pointer for READ statement
extern int currentLine; // Current line being executed
extern int nextLine; // Next line to be executed
extern bool running;

// for gosub
#define MAX_GOSUB_STACK 100
extern int gosubStack[MAX_GOSUB_STACK];
extern int gosubStackPtr;

// Environment variables
extern bool emu_amiga_m68k;

// --- Function Declarations ---
void executeLine(Line *line);
void executeSet(Token *tokens, int numTokens);
Token getNextToken(char *line, int *pos);
void tokenizeLine(char *line, Line *lineStruct);
Variable *findVariable(const char *name);
double getNumericValue(Token *token);
char *getStringValue(Token *token);
double evaluateExpression(Token *tokens, int numTokens);
bool variableExists(const char *name);
void addOrUpdateVariable(const char *name, VarType type, double numValue, const char *strValue);
void executeList(int startLine, int endLine);
void executeNew();
void executePrint(Token *tokens, int numTokens);
void executeInput(Token *tokens, int numTokens);
void executeLet(Token *tokens, int numTokens);
void executeIf(Token *tokens, int numTokens);
void executeFor(Token *tokens, int numTokens);
void executeNext(Token *tokens, int numTokens);
void executeData(Token *tokens, int numTokens);
void executeRead(Token *tokens, int numTokens);
void executeRestore();
void executeGoto(Token *tokens, int numTokens);
void executeGosub(Token *tokens, int numTokens);
void executeReturn();
void executeEnd();
void runProgram(int startLine);
void addLine(Line *newLine);

#endif // CBSH_H
