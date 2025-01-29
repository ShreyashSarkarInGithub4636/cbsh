#include "cbsh.h"

// --- Helper Functions ---

// Find the line number of the target
int findLineIndex(int lineNumber) {
    for (int i = 0; i < numLines; i++) {
        if (program[i].lineNumber == lineNumber) {
            return i;
        }
    }
    return -1;  // Not found
}

// --- Command Execution Functions ---

// Execute LIST command
void executeList(int startLine, int endLine) {
    for (int i = 0; i < numLines; i++) {
        if (program[i].lineNumber >= startLine && (endLine == -1 || program[i].lineNumber <= endLine)) {
            printf("%d ", program[i].lineNumber);
            for (int j = 0; j < program[i].numTokens; j++) {
                printf("%s ", program[i].tokens[j].value);
            }
            printf("\n");
        }
    }
}

// Execute NEW command
void executeNew() {
    numLines = 0;
    numVariables = 0;
    numDataValues = 0;
    dataReadPtr = 0;
    currentLine = 0;
    running = false;
    gosubStackPtr = 0; // Reset GOSUB stack
    for (int i = 0; i < MAX_VARIABLES; i++) {
        variables[i].name[0] = '\0'; // Invalidate the variable
    }
}

// Execute PRINT command
void executePrint(Token *tokens, int numTokens) {
    for (int i = 1; i < numTokens; i++) {
        switch (tokens[i].type) {
            case TOKEN_STRING:
                printf("%s", tokens[i].value);
                break;
            case TOKEN_IDENTIFIER: {
                Variable *var = findVariable(tokens[i].value);
                if (var) {
                    if (var->type == VAR_TYPE_NUMERIC) {
                        printf("%g", var->numValue);
                    } else {
                        printf("%s", var->strValue);
                    }
                } else {
                    printf("0"); // Default value for undefined numeric variables
                }
                break;
            }
            case TOKEN_NUMBER:
                printf("%g", atof(tokens[i].value));
                break;
            case TOKEN_OPERATOR:
                if (strcmp(tokens[i].value, ",") == 0) {
                    printf("\t");
                } else if (strcmp(tokens[i].value, ";") == 0) {
                    // No newline, just continue printing
                } else {
                    printf(" ");
                }
                break;
            case TOKEN_KEYWORD:
                if (tokens[i].keyword == KW_TAB) {
                    if (i + 1 < numTokens && tokens[i + 1].type == TOKEN_NUMBER) {
                        int spaces = atoi(tokens[i + 1].value);
                        for (int j = 0; j < spaces; j++) {
                            printf(" ");
                        }
                        i++;
                    }
                } else {
                    printf(" ");
                }
                break;
            default:
                printf(" ");
                break;
        }
    }

    if (numTokens > 1 && tokens[numTokens - 1].type == TOKEN_OPERATOR && strcmp(tokens[numTokens - 1].value, ";") == 0) {
        // No newline
    } else {
        printf("\n");
    }
}

// Execute LOAD command
void executeLoad(Token *tokens, int numTokens) {
    if (numTokens < 2 || tokens[1].type != TOKEN_STRING) {
        printf('Invalid LOAD statement, try with "cmdname" \n');
        return;
    }

    char *filename = tokens[1].value; // Filename (or command) to execute

    // Construct argv array for execvp
    char *argv[MAX_LINE_LENGTH]; // Adjust size as needed
    int argc = 0;
    argv[argc++] = filename; // The first argument is the command itself

    // Add arguments if present (for future improvements)
    for (int i = 2; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_STRING) {
            argv[argc++] = tokens[i].value;
        } else {
            // Handle other token types or add more complex argument parsing here
        }
    }
    argv[argc] = NULL; // argv array must be NULL-terminated

    // Search for the executable in PATH and execute
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(filename, argv);
        // If execvp returns, an error occurred
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        wait(NULL); // Wait for child process to finish
    } else {
        perror("fork");
    }
}

// Execute DIR command
void executeDir() {
    char cwd[MAX_LINE_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        DIR *dir = opendir(cwd);
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                printf("%s\n", entry->d_name);
            }
            closedir(dir);
        } else {
            perror("opendir");
        }
    } else {
        perror("getcwd");
    }
}

// Execute INPUT command
void executeInput(Token *tokens, int numTokens) {
    if (numTokens < 2) {
        printf("Invalid INPUT statement\n");
        return;
    }

    int varIndex = 1;
    if (tokens[1].type == TOKEN_STRING) {
        printf("%s", tokens[1].value);
        varIndex = 2;
        if (varIndex >= numTokens || tokens[varIndex].type != TOKEN_IDENTIFIER) {
            printf("Missing variable in INPUT statement\n");
            return;
        }
    } else if (tokens[1].type == TOKEN_IDENTIFIER) {
        printf("? ");
    } else {
        printf("Invalid INPUT statement\n");
        return;
    }

    Variable *var = findVariable(tokens[varIndex].value);
    if (!var) {
        if (strchr(tokens[varIndex].value, '$') != NULL) {
            addOrUpdateVariable(tokens[varIndex].value, VAR_TYPE_STRING, 0, "");
            var = findVariable(tokens[varIndex].value);
        } else {
            addOrUpdateVariable(tokens[varIndex].value, VAR_TYPE_NUMERIC, 0, "");
            var = findVariable(tokens[varIndex].value);
        }
    }

    char inputBuffer[MAX_LINE_LENGTH];
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) {
        printf("Error reading input\n");
        return;
    }

    inputBuffer[strcspn(inputBuffer, "\n")] = 0;

    if (var->type == VAR_TYPE_NUMERIC) {
        char *endptr;
        double numValue = strtod(inputBuffer, &endptr);
        if (*endptr != '\0') {
            printf("Invalid number input\n");
            var->numValue = 0;
        } else {
            var->numValue = numValue;
        }
    } else {
        strcpy(var->strValue, inputBuffer);
    }
}

// Execute LET command (and implicit assignment)
void executeLet(Token *tokens, int numTokens) {
    if (numTokens < 3) {
        printf("Invalid LET statement\n");
        return;
    }

    int assignmentOpIndex = -1;
    for (int i = 0; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_OPERATOR && strcmp(tokens[i].value, "=") == 0) {
            assignmentOpIndex = i;
            break;
        }
    }

    if (assignmentOpIndex == -1) {
        printf("Missing '=' in LET statement\n");
        return;
    }

    char varName[MAX_LINE_LENGTH];
    strncpy(varName, tokens[0].value, sizeof(varName) - 1);
    varName[sizeof(varName) - 1] = '\0';

    VarType varType;
    if (strchr(varName, '$') != NULL) {
        varType = VAR_TYPE_STRING;
    } else {
        varType = VAR_TYPE_NUMERIC;
    }

    if (varType == VAR_TYPE_NUMERIC) {
        double value = evaluateExpression(&tokens[assignmentOpIndex + 1], numTokens - assignmentOpIndex - 1);
        addOrUpdateVariable(varName, varType, value, "");
    } else {
        if (tokens[assignmentOpIndex + 1].type == TOKEN_STRING) {
            addOrUpdateVariable(varName, varType, 0, tokens[assignmentOpIndex + 1].value);
        } else if (tokens[assignmentOpIndex + 1].type == TOKEN_IDENTIFIER) {
            char *strValue = getStringValue(&tokens[assignmentOpIndex + 1]);
            addOrUpdateVariable(varName, varType, 0, strValue);
        } else {
            printf("Invalid string expression in LET\n");
        }
    }
}

// Execute IF command
void executeIf(Token *tokens, int numTokens) {
    int thenIndex = -1;
    for (int i = 0; i < numTokens; i++) {
        if (tokens[i].keyword == KW_THEN) {
            thenIndex = i;
            break;
        }
    }

    if (thenIndex == -1) {
        printf("Invalid IF statement: THEN not found\n");
        return;
    }

    double conditionResult = evaluateExpression(tokens + 1, thenIndex - 1);

    if (conditionResult != 0) {
        Line thenStatement;
        thenStatement.lineNumber = 0;
        thenStatement.numTokens = 0;
        for (int i = thenIndex + 1; i < numTokens; i++) {
            thenStatement.tokens[thenStatement.numTokens++] = tokens[i];
        }
        executeLine(&thenStatement);
    }
}

// Execute FOR command
void executeFor(Token *tokens, int numTokens) {
    if (numTokens < 7 || tokens[2].type != TOKEN_OPERATOR || strcmp(tokens[2].value, "=") != 0 || tokens[4].keyword != KW_TO) {
        printf("Invalid FOR statement\n");
        return;
    }

    char *varName = tokens[1].value;
    double startValue = evaluateExpression(&tokens[3], 1);
    double endValue = evaluateExpression(&tokens[5], 1);
    double stepValue = 1; // Default step

    int stepIndex = 6;
    if (stepIndex < numTokens && tokens[stepIndex].keyword == KW_STEP) {
        if (stepIndex + 1 < numTokens) {
            stepValue = evaluateExpression(&tokens[stepIndex + 1], 1);
            stepIndex += 2;
        } else {
            printf("Missing value after STEP\n");
            return;
        }
    }

    Variable *loopVar = findVariable(varName);
    if (!loopVar) {
        addOrUpdateVariable(varName, VAR_TYPE_NUMERIC, startValue, "");
        loopVar = findVariable(varName);
    } else {
        loopVar->numValue = startValue;
    }

    int nextLineIndex = -1;
    for (int i = currentLine + 1; i < numLines; i++) {
        if (program[i].numTokens > 0 && program[i].tokens[0].keyword == KW_NEXT) {
            if (program[i].numTokens > 1 && program[i].tokens[1].type == TOKEN_IDENTIFIER) {
                if (strcmp(program[i].tokens[1].value, varName) == 0) {
                    nextLineIndex = i;
                    break;
                }
            } else {
                nextLineIndex = i;
                break;
            }
        }
    }

    if (nextLineIndex == -1) {
        printf("FOR without NEXT\n");
        return;
    }

    loopVar->forNextLine = nextLineIndex;
    loopVar->forStep = stepValue;
    loopVar->forEnd = endValue;
    loopVar->forStartLine = currentLine;

    nextLine = currentLine + 1;
}

// Execute NEXT command
void executeNext(Token *tokens, int numTokens) {
    char *varName = NULL;
    if (numTokens > 1 && tokens[1].type == TOKEN_IDENTIFIER) {
        varName = tokens[1].value;
    }

    int forLineIndex = -1;
    Variable *loopVar = NULL;
    if (varName) {
        loopVar = findVariable(varName);
        if (loopVar) {
            forLineIndex = loopVar->forStartLine;
        }
    } else {
        for (int i = currentLine - 1; i >= 0; i--) {
            if (program[i].numTokens > 0 && program[i].tokens[0].keyword == KW_FOR) {
                loopVar = findVariable(program[i].tokens[1].value);
                if (loopVar) {
                    forLineIndex = i;
                    break;
                }
            }
        }
    }

    if (forLineIndex == -1 || !loopVar) {
        printf("NEXT without FOR\n");
        return;
    }

    loopVar->numValue += loopVar->forStep;

    if ((loopVar->forStep > 0 && loopVar->numValue > loopVar->forEnd) ||
        (loopVar->forStep < 0 && loopVar->numValue < loopVar->forEnd)) {
        nextLine = currentLine + 1;
    } else {
        nextLine = loopVar->forStartLine + 1;
    }
}

// Execute DATA command
void executeData(Token *tokens, int numTokens) {
    for (int i = 1; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_NUMBER) {
            if (numDataValues < MAX_DATA_VALUES) {
                dataValues[numDataValues++] = atof(tokens[i].value);
            } else {
                printf("Too many DATA values\n");
                return;
            }
        } else if (tokens[i].type == TOKEN_STRING) {
            printf("String DATA not yet implemented\n");
            return;
        }
    }
}

// Execute READ command
void executeRead(Token *tokens, int numTokens) {
    if (numTokens < 2) {
        printf("Invalid READ statement\n");
        return;
    }

    for (int i = 1; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_IDENTIFIER) {
            Variable *var = findVariable(tokens[i].value);
            if (!var) {
                addOrUpdateVariable(tokens[i].value, VAR_TYPE_NUMERIC, 0, "");
                var = findVariable(tokens[i].value);
            }

            if (dataReadPtr < numDataValues) {
                var->numValue = dataValues[dataReadPtr++];
            } else {
                printf("Out of DATA\n");
                return;
            }
        }
    }
}

// Execute RESTORE command
void executeRestore() {
    dataReadPtr = 0;
}

// Execute GOTO command
void executeGoto(Token *tokens, int numTokens) {
    if (numTokens < 2 || tokens[1].type != TOKEN_NUMBER) {
        printf("Invalid GOTO statement\n");
        return;
    }

    int targetLine = atoi(tokens[1].value);
    int targetIndex = findLineIndex(targetLine);
    if (targetIndex != -1) {
        nextLine = targetIndex;
    } else {
        printf("Undefined line %d\n", targetLine);
    }
}

// Execute GOSUB command
void executeGosub(Token *tokens, int numTokens) {
    if (numTokens < 2 || tokens[1].type != TOKEN_NUMBER) {
        printf("Invalid GOSUB statement\n");
        return;
    }

    int targetLine = atoi(tokens[1].value);
    int targetIndex = findLineIndex(targetLine);

    if (targetIndex != -1) {
        if (gosubStackPtr < MAX_GOSUB_STACK) {
            gosubStack[gosubStackPtr++] = currentLine + 1;
            nextLine = targetIndex;
        } else {
            printf("GOSUB stack overflow\n");
        }
    } else {
        printf("Undefined line %d\n", targetLine);
    }
}

// Execute RETURN command
void executeReturn() {
    if (gosubStackPtr > 0) {
        nextLine = gosubStack[--gosubStackPtr];
    } else {
        printf("RETURN without GOSUB\n");
    }
}

// Execute END command
void executeEnd() {
    running = false;
    nextLine = 0;
}

// Execute SET command
void executeSet(Token *tokens, int numTokens) {
    if (numTokens < 3 || tokens[1].type != TOKEN_IDENTIFIER || tokens[2].type != TOKEN_OPERATOR || strcmp(tokens[2].value, "=") != 0) {
        printf("Invalid SET statement\n");
        return;
    }

    if (strcasecmp(tokens[1].value, "emu_amiga_m68k") == 0) {
        if (numTokens > 3 && tokens[3].type == TOKEN_IDENTIFIER) {
            if (strcasecmp(tokens[3].value, "TRUE") == 0) {
                emu_amiga_m68k = true;
                printf("emu_amiga_m68k set to TRUE\n");
            } else if (strcasecmp(tokens[3].value, "FALSE") == 0) {
                emu_amiga_m68k = false;
                printf("emu_amiga_m68k set to FALSE\n");
            } else {
                printf("Invalid value for emu_amiga_m68k\n");
            }
        } else {
            printf("Invalid value for emu_amiga_m68k\n");
        }
    } else {
        printf("Unknown variable in SET statement\n");
    }
}

// Execute a line of BASIC code
void executeLine(Line *line) {
    if (line->numTokens == 0) {
        return;
    }

    switch (line->tokens[0].keyword) {
        case KW_REM:
            // Comment, do nothing
            break;
        case KW_LET:
            executeLet(line->tokens, line->numTokens);
            break;
        case KW_PRINT:
            executePrint(line->tokens, line->numTokens);
            break;
        case KW_LOAD: // Add this case
            executeLoad(line->tokens, line->numTokens);
            break;
        case KW_DIR: // Add this case
            executeDir();
            break;
        case KW_INPUT:
            executeInput(line->tokens, line->numTokens);
            break;
        case KW_IF:
            executeIf(line->tokens, line->numTokens);
            break;
        case KW_FOR:
            executeFor(line->tokens, line->numTokens);
            break;
        case KW_NEXT:
            executeNext(line->tokens, line->numTokens);
            break;
        case KW_GOTO:
            executeGoto(line->tokens, line->numTokens);
            break;
        case KW_GOSUB:
            executeGosub(line->tokens, line->numTokens);
            break;
        case KW_RETURN:
            executeReturn();
            break;
        case KW_DATA:
            executeData(line->tokens, line->numTokens);
            break;
        case KW_READ:
            executeRead(line->tokens, line->numTokens);
            break;
        case KW_RESTORE:
            executeRestore();
            break;
        case KW_END:
            executeEnd();
            break;
        case KW_SET:
            executeSet(line->tokens, line->numTokens);
            break;
        case KW_NONE:
            if (line->tokens[0].type == TOKEN_IDENTIFIER) {
                // Implicit LET
                executeLet(line->tokens, line->numTokens);
            } else if (line->tokens[0].type == TOKEN_NUMBER) {
                printf("Syntax error\n");
            } else {
                printf("Unimplemented command: %s\n", line->tokens[0].value);
            }
            break;
        default:
            printf("Unimplemented command: %s\n", line->tokens[0].value);
            break;
    }
}
