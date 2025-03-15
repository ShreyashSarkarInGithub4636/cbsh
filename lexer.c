#include "cbsh.h"

// Function to get the next token from a line of input
Token getNextToken(char *line, int *pos) {
    Token token;
    token.type = TOKEN_EOF; // Default
    token.keyword = KW_NONE;
    token.value[0] = '\0'; // Initialize value

    // Skip whitespace
    while (line[*pos] == ' ' || line[*pos] == '\t') {
        (*pos)++;
    }

    // Handle end of line
    if (line[*pos] == '\0' || line[*pos] == '\n' || line[*pos] == '\r') {
        token.type = TOKEN_NEWLINE;
        return token;
    }

    // Handle comments (REM or ')
    if (line[*pos] == '\'') {
        token.type = TOKEN_KEYWORD;
        token.keyword = KW_REM;
        strcpy(token.value, "REM");
        (*pos)++;
        while (line[*pos] != '\0' && line[*pos] != '\n' && line[*pos] != '\r') {
            (*pos)++;
        }
        return token;
    }

    if (isalpha(line[*pos])) {
        // Identifier or keyword
        int start = *pos;
        while (isalnum(line[*pos]) || line[*pos] == '$') { // Handle string variables (e.g., A$)
            (*pos)++;
        }
        int len = *pos - start;
        if (len >= MAX_LINE_LENGTH) {
            len = MAX_LINE_LENGTH - 1; // Prevent buffer overflow
        }
        strncpy(token.value, &line[start], len);
        token.value[len] = '\0';
        token.type = TOKEN_IDENTIFIER;

        // Check if it's a keyword
        if (strcasecmp(token.value, "LIST") == 0) token.keyword = KW_LIST;
        else if (strcasecmp(token.value, "NEW") == 0) token.keyword = KW_NEW;
        else if (strcasecmp(token.value, "PRINT") == 0) token.keyword = KW_PRINT;
        else if (strcasecmp(token.value, "INPUT") == 0) token.keyword = KW_INPUT;
        else if (strcasecmp(token.value, "LOAD") == 0) token.keyword = KW_LOAD;
        else if (strcasecmp(token.value, "DIR") == 0) token.keyword = KW_DIR;
        else if (strcasecmp(token.value, "IF") == 0) token.keyword = KW_IF;
        else if (strcasecmp(token.value, "THEN") == 0) token.keyword = KW_THEN;
        else if (strcasecmp(token.value, "FOR") == 0) token.keyword = KW_FOR;
        else if (strcasecmp(token.value, "NEXT") == 0) token.keyword = KW_NEXT;
        else if (strcasecmp(token.value, "SQR") == 0) token.keyword = KW_SQR;
        else if (strcasecmp(token.value, "RND") == 0) token.keyword = KW_RND;
        else if (strcasecmp(token.value, "SIN") == 0) token.keyword = KW_SIN;
        else if (strcasecmp(token.value, "LET") == 0) token.keyword = KW_LET;
        else if (strcasecmp(token.value, "USR") == 0) token.keyword = KW_USR;
        else if (strcasecmp(token.value, "DATA") == 0) token.keyword = KW_DATA;
        else if (strcasecmp(token.value, "READ") == 0) token.keyword = KW_READ;
        else if (strcasecmp(token.value, "REM") == 0) token.keyword = KW_REM;
        else if (strcasecmp(token.value, "CLEAR") == 0) token.keyword = KW_CLEAR;
        else if (strcasecmp(token.value, "STOP") == 0) token.keyword = KW_STOP;
        else if (strcasecmp(token.value, "TAB") == 0) token.keyword = KW_TAB;
        else if (strcasecmp(token.value, "RESTORE") == 0) token.keyword = KW_RESTORE;
        else if (strcasecmp(token.value, "ABS") == 0) token.keyword = KW_ABS;
        else if (strcasecmp(token.value, "END") == 0) token.keyword = KW_END;
        else if (strcasecmp(token.value, "INT") == 0) token.keyword = KW_INT;
        else if (strcasecmp(token.value, "RETURN") == 0) token.keyword = KW_RETURN;
        else if (strcasecmp(token.value, "STEP") == 0) token.keyword = KW_STEP;
        else if (strcasecmp(token.value, "GOTO") == 0) token.keyword = KW_GOTO;
        else if (strcasecmp(token.value, "GOSUB") == 0) token.keyword = KW_GOSUB;
        else if (strcasecmp(token.value, "SET") == 0) token.keyword = KW_SET;
        else if (strcasecmp(token.value, "TO") == 0) token.keyword = KW_TO;
        else if (strcasecmp(token.value, "RUN") == 0) token.keyword = KW_RUN;
        else if (strcasecmp(token.value, "ADD") == 0) token.keyword = KW_ADD;
        else if (strcasecmp(token.value, "DIV") == 0) token.keyword = KW_DIV;
        else if (strcasecmp(token.value, "FLOOR") == 0) token.keyword = KW_FLOOR;
        else if (strcasecmp(token.value, "SUB") == 0) token.keyword = KW_SUB;

        if (token.keyword != KW_NONE) {
            token.type = TOKEN_KEYWORD;
        }
        return token;
    } else if (isdigit(line[*pos])) {
        // Number
        int start = *pos;
        while (isdigit(line[*pos]) || line[*pos] == '.') {
            (*pos)++;
        }
        int len = *pos - start;
        if (len >= MAX_LINE_LENGTH) {
            len = MAX_LINE_LENGTH - 1;
        }
        strncpy(token.value, &line[start], len);
        token.value[len] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    } else if (line[*pos] == '"') {
        // String literal
        (*pos)++;
        int start = *pos;
        while (line[*pos] != '"' && line[*pos] != '\0' && line[*pos] != '\n' && line[*pos] != '\r') {
            (*pos)++;
        }
        if (line[*pos] == '"') {
            int len = *pos - start;
            if (len >= MAX_LINE_LENGTH) {
                len = MAX_LINE_LENGTH - 1;
            }
            strncpy(token.value, &line[start], len);
            token.value[len] = '\0';
            token.type = TOKEN_STRING;
            (*pos)++;
            return token;
        } else {
            printf("Unterminated string\n");
            token.type = TOKEN_EOF;
            return token;
        }
    } else if (strchr("+-*/=<>(),", line[*pos]) != NULL) {
        // Operator or punctuation
        token.value[0] = line[*pos];
        token.value[1] = '\0';
        token.type = TOKEN_OPERATOR;
        (*pos)++;
        return token;
    } else if (line[*pos] == ':') {
        // Colon
        token.value[0] = line[*pos];
        token.value[1] = '\0';
        token.type = TOKEN_COLON;
        (*pos)++;
        return token;
    } else {
        // Invalid character
        printf("Invalid character: %c\n", line[*pos]);
        (*pos)++;
        token.type = TOKEN_EOF;
        return token;
    }
}

// Tokenize a line of input and store tokens in the Line structure
void tokenizeLine(char *line, Line *lineStruct) {
    int pos = 0;
    lineStruct->numTokens = 0;

    // Check for a line number
    Token firstToken = getNextToken(line, &pos);
    if (firstToken.type == TOKEN_NUMBER) {
        lineStruct->lineNumber = atoi(firstToken.value);
    } else {
        lineStruct->lineNumber = 0;
        pos = 0;
    }

    // Tokenize the rest of the line
    size_t lineLength = strlen(line);
    while (pos < lineLength) {
        Token token = getNextToken(line, &pos);
        if (token.type == TOKEN_EOF || token.type == TOKEN_NEWLINE) {
            break;
        }
        lineStruct->tokens[lineStruct->numTokens++] = token;
    }
}
