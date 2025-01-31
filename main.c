#include "cbsh.h"


// Global data structures (unchanged)
Line program[MAX_NUM_LINES];
int numLines = 0;
Variable variables[MAX_VARIABLES];
int numVariables = 0;
double dataValues[MAX_DATA_VALUES];
int numDataValues = 0;
int dataReadPtr = 0; // Pointer for READ statement
int currentLine = 0; // Current line being executed
int nextLine = 0; // Next line to be executed
bool running = false;

// for gosub
int gosubStack[MAX_GOSUB_STACK];
int gosubStackPtr = 0;

// Environment variables
bool emu_amiga_m68k = false;


// Function to complete filenames
char **filename_completion(const char *text, int start, int end) {
    return rl_completion_matches(text, rl_filename_completion_function);
}

int main(int argc, char *argv[]) {
    char *lineBuffer;

    // Install filename completion
    rl_attempted_completion_function = filename_completion;

    if (argc > 1) {
        // Script mode (unchanged)
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error opening file: %s\n", argv[1]);
            return 1;
        }

        // Check for shebang (unchanged)
        if (fgets(lineBuffer, sizeof(lineBuffer), file) != NULL) {
            if (strncmp(lineBuffer, "#!/usr/bin/env cbsh", 19) != 0 && strncmp(lineBuffer, "#!/usr/bin/CBSH", 15) != 0) {
                rewind(file);
            }
        }

        while (fgets(lineBuffer, MAX_LINE_LENGTH, file) != NULL) {
            lineBuffer[strcspn(lineBuffer, "\n")] = 0;

            Line newLine;
            tokenizeLine(lineBuffer, &newLine);

            if (newLine.lineNumber == 0) {
                executeLine(&newLine);
            } else {
                addLine(&newLine);
            }
        }
        fclose(file);

        runProgram(0);
    } else {
        // Interactive mode
        printf("CBSH - Commodore BASIC Shell, version 1.0\n \n");
        printf("READY.\n");

        while (1) {
            lineBuffer = readline("> ");
            if (!lineBuffer) {
                break; // Exit on EOF (Ctrl+D)
            }

            // Add the line to history
            add_history(lineBuffer);

            Line newLine;
            tokenizeLine(lineBuffer, &newLine);

            if (newLine.lineNumber == 0) {
                if (newLine.numTokens > 0) {
                    if (newLine.tokens[0].keyword == KW_LIST) {
                        executeList(0, -1);
                    } else if (newLine.tokens[0].keyword == KW_NEW) {
                        executeNew();
                    } else if (newLine.tokens[0].keyword == KW_RUN) {
                        runProgram(0);
                    } else {
                        executeLine(&newLine);
                    }
                }
            } else {
                addLine(&newLine);
            }
            free(lineBuffer);
        }
    }

    return 0;
}
