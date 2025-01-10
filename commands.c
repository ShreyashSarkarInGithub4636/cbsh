#include "cbsh.h"

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
    numLines = 0;      // Clear the program
    numVariables = 0;  // Clear variables
    numDataValues = 0; // Clear DATA
    dataReadPtr = 0;
    currentLine = 0;
    running = false;
}

// Execute PRINT command
void executePrint(Token *tokens, int numTokens) {
    for (int i = 1; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_STRING) {
            printf("%s", tokens[i].value);
        } else if (tokens[i].type == TOKEN_IDENTIFIER) {
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
        } else if (tokens[i].type == TOKEN_NUMBER) {
            printf("%g", atof(tokens[i].value));
        } else if (tokens[i].type == TOKEN_OPERATOR && strcmp(tokens[i].value, ",") == 0) {
            printf("\t"); // Basic comma spacing
        } else if (tokens[i].keyword == KW_TAB) {
            if (i + 1 < numTokens && tokens[i + 1].type == TOKEN_NUMBER) {
                int spaces = atoi(tokens[i + 1].value);
                for (int j = 0; j < spaces; j++) {
                    printf(" ");
                }
                i++; // Skip the next token (number of spaces)
            }
        } else if (tokens[i].type == TOKEN_OPERATOR && strcmp(tokens[i].value, ";") == 0) {
            // No newline, just continue printing
        } else {
            printf(" "); // Default space
        }
    }
    if (numTokens > 1 && tokens[numTokens - 1].type == TOKEN_OPERATOR && strcmp(tokens[numTokens - 1].value, ";") == 0) {
        // Don't print a newline if the last token is a semicolon
    } else {
        printf("\n");
    }
}

// Execute INPUT command
void executeInput(Token *tokens, int numTokens) {
    if (numTokens < 2) {
        printf("Invalid INPUT statement\n");
        return;
    }

    // Check for a prompt string
    int varIndex = 1; // Index of the variable to store input
    if (tokens[1].type == TOKEN_STRING) {
        printf("%s", tokens[1].value); // Print the prompt
        varIndex = 2;
        if (varIndex >= numTokens || tokens[varIndex].type != TOKEN_IDENTIFIER) {
            printf("Missing variable in INPUT statement\n");
            return;
        }
    } else if (tokens[1].type == TOKEN_IDENTIFIER) {
        printf("? "); // Default prompt if no string is provided
    } else {
        printf("Invalid INPUT statement\n");
        return;
    }

    // Get the variable to store input
    Variable *var = findVariable(tokens[varIndex].value);
    if (!var) {
        // Create a new variable (determine type based on name or later input)
        if (strchr(tokens[varIndex].value, '$') != NULL) {
            // String variable (if it ends with $)
            addOrUpdateVariable(tokens[varIndex].value, VAR_TYPE_STRING, 0, "");
            var = findVariable(tokens[varIndex].value);
        } else {
            // Assume numeric by default
            addOrUpdateVariable(tokens[varIndex].value, VAR_TYPE_NUMERIC, 0, "");
            var = findVariable(tokens[varIndex].value);
        }
    }

    // Read input from user
    char inputBuffer[MAX_LINE_LENGTH];
    if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == NULL) {
        printf("Error reading input\n");
        return; // Handle input error
    }

    // Remove trailing newline from input
    inputBuffer[strcspn(inputBuffer, "\n")] = 0;

    // Store input in the variable
    if (var->type == VAR_TYPE_NUMERIC) {
        // Try to convert to a number
        char *endptr;
        double numValue = strtod(inputBuffer, &endptr);
        if (*endptr != '\0') {
            // Conversion failed, not a valid number
            printf("Invalid number input\n");
            var->numValue = 0; // Reset to 0 (or handle error differently)
        } else {
            var->numValue = numValue;
        }
    } else {
        // Store as string
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

    // Extract variable name
    char varName[MAX_LINE_LENGTH];
    strncpy(varName, tokens[0].value, sizeof(varName) - 1);
    varName[sizeof(varName) - 1] = '\0'; // Ensure null-termination

    // Determine variable type
    VarType varType;
    if (strchr(varName, '$') != NULL) {
        varType = VAR_TYPE_STRING;
    } else {
        varType = VAR_TYPE_NUMERIC;
    }

    // Evaluate the expression on the right-hand side
    if (varType == VAR_TYPE_NUMERIC) {
        double value = evaluateExpression(&tokens[assignmentOpIndex + 1], numTokens - assignmentOpIndex - 1);
        addOrUpdateVariable(varName, varType, value, "");
    } else {
        if (tokens[assignmentOpIndex + 1].type == TOKEN_STRING) {
            addOrUpdateVariable(varName, varType, 0, tokens[assignmentOpIndex + 1].value);
        } else if (tokens[assignmentOpIndex + 1].type == TOKEN_IDENTIFIER) {
            // Handle string variable assignment here (e.g., A$ = B$)
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

    // Evaluate the condition
    double conditionResult = evaluateExpression(tokens + 1, thenIndex - 1);

    // If the condition is true, execute the THEN part
    if (conditionResult != 0) {
        // Execute tokens after THEN as a new statement
        Line thenStatement;
        thenStatement.lineNumber = 0; // Treat it as an immediate mode line
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

    // Check for optional STEP
    int stepIndex = 6;
    if (stepIndex < numTokens && tokens[stepIndex].keyword == KW_STEP) {
        if (stepIndex + 1 < numTokens) {
            stepValue = evaluateExpression(&tokens[stepIndex + 1], 1);
            stepIndex += 2; // Skip STEP and its value
        } else {
            printf("Missing value after STEP\n");
            return;
        }
    }

    // Find or create the loop variable
    Variable *loopVar = findVariable(varName);
    if (!loopVar) {
        addOrUpdateVariable(varName, VAR_TYPE_NUMERIC, startValue, "");
        loopVar = findVariable(varName);
    } else {
        loopVar->numValue = startValue;
    }

    // Find the matching NEXT statement
    int nextLineIndex = -1;
    for (int i = currentLine + 1; i < numLines; i++) {
        if (program[i].numTokens > 0 && program[i].tokens[0].keyword == KW_NEXT) {
            // Check if NEXT refers to the correct variable (optional in some BASICs)
            if (program[i].numTokens > 1 && program[i].tokens[1].type == TOKEN_IDENTIFIER) {
                if (strcmp(program[i].tokens[1].value, varName) == 0) {
                    nextLineIndex = i;
                    break;
                }
            } else {
                // Unnamed NEXT matches any FOR
                nextLineIndex = i;
                break;
            }
        }
    }

    if (nextLineIndex == -1) {
        printf("FOR without NEXT\n");
        return;
    }

    // Store the loop information for later execution in executeNext
    loopVar->forNextLine = nextLineIndex;
    loopVar->forStep = stepValue;
    loopVar->forEnd = endValue;
    loopVar->forStartLine = currentLine; // Store the line number of FOR

    // Continue execution to the next line after FOR
    nextLine = currentLine + 1;
}

// Execute NEXT command
void executeNext(Token *tokens, int numTokens) {
    char *varName = NULL;
    if (numTokens > 1 && tokens[1].type == TOKEN_IDENTIFIER) {
        varName = tokens[1].value;
    }

    // Find the matching FOR loop
    int forLineIndex = -1;
    Variable *loopVar = NULL;
    if (varName) {
        loopVar = findVariable(varName);
        if (loopVar) {
            forLineIndex = loopVar->forStartLine;
        }
    } else {
        // Find the most recent FOR (in case of nested loops)
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

    // Update loop variable
    loopVar->numValue += loopVar->forStep;

    // Check if loop should terminate
    if ((loopVar->forStep > 0 && loopVar->numValue > loopVar->forEnd) ||
        (loopVar->forStep < 0 && loopVar->numValue < loopVar->forEnd)) {
        // Loop finished, continue to the line after NEXT
        nextLine = currentLine + 1;
    } else {
        // Loop again, go back to the line after FOR
        nextLine = forLineIndex + 1;
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
            // Handle string data (you might need a separate array for string data)
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
                // Create a new numeric variable (string DATA not handled in this example)
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
    dataReadPtr = 0; // Reset the data pointer
}

// Execute GOTO command
void executeGoto(Token *tokens, int numTokens) {
    if (numTokens < 2 || tokens[1].type != TOKEN_NUMBER) {
        printf("Invalid GOTO statement\n");
        return;
    }

    int targetLine = atoi(tokens[1].value);

    // Find the target line
    int targetIndex = -1;
    for (int i = 0; i < numLines; i++) {
        if (program[i].lineNumber == targetLine) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex != -1) {
        nextLine = targetIndex; // Jump to the target line
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

    // Find the target line
    int targetIndex = -1;
    for (int i = 0; i < numLines; i++) {
        if (program[i].lineNumber == targetLine) {
            targetIndex = i;
            break;
        }
    }

    if (targetIndex != -1) {
        // Push the next line number onto the GOSUB stack
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
        nextLine = gosubStack[--gosubStackPtr]; // Pop the return line number
    } else {
        printf("RETURN without GOSUB\n");
    }
}

// Execute END command
void executeEnd() {
    running = false;
    nextLine = 0; // Reset to the beginning of the program
}

// Execute a line of BASIC code
void executeLine(Line *line) {
    if (line->numTokens == 0) {
        return; // Empty line
    }

    if (line->tokens[0].keyword == KW_REM) {
        // It's a comment, do nothing
        return;
    } else if (line->tokens[0].keyword == KW_LET) {
        executeLet(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_PRINT) {
        executePrint(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_INPUT) {
        executeInput(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_IF) {
        executeIf(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_FOR) {
        executeFor(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_NEXT) {
        executeNext(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_GOTO) {
        executeGoto(line->tokens, line->numTokens);
        } else if (line->tokens[0].keyword == KW_GOSUB) {
        executeGosub(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_RETURN) {
        executeReturn();
    } else if (line->tokens[0].keyword == KW_DATA) {
        executeData(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_READ) {
        executeRead(line->tokens, line->numTokens);
    } else if (line->tokens[0].keyword == KW_RESTORE) {
        executeRestore();
    } else if (line->tokens[0].keyword == KW_END) {
        executeEnd();
    } else if (line->tokens[0].keyword == KW_SET) {
        executeSet(line->tokens, line->numTokens);
    } else if (line->tokens[0].type == TOKEN_IDENTIFIER) {
        // Implicit LET (assignment without LET keyword)
        executeLet(line->tokens, line->numTokens);
    } else if (line->tokens[0].type == TOKEN_NUMBER) {
        // Line number without a command (shouldn't happen during execution)
        printf("Syntax error\n");
    } else {
        printf("Unimplemented command: %s\n", line->tokens[0].value);
    }
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
