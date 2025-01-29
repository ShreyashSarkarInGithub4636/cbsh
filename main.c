#include "cbsh.h"

// Global data structures
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

int main(int argc, char *argv[]) {
    char lineBuffer[MAX_LINE_LENGTH];

    if (argc > 1) {
        // Script mode
        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
            printf("Error opening file: %s\n", argv[1]);
            return 1;
        }

        // Check for shebang
        if (fgets(lineBuffer, sizeof(lineBuffer), file) != NULL) {
            if (strncmp(lineBuffer, "#!/usr/bin/env cbsh", 19) != 0 && strncmp(lineBuffer, "#!/usr/bin/CBSH", 15) != 0) {
                // Rewind to the beginning if no shebang is found
                rewind(file);
            }
        }

        while (fgets(lineBuffer, sizeof(lineBuffer), file) != NULL) {
            lineBuffer[strcspn(lineBuffer, "\n")] = 0; // Remove trailing newline

            Line newLine;
            tokenizeLine(lineBuffer, &newLine);

            // Handle immediate mode commands in the script
            if (newLine.lineNumber == 0) {
                executeLine(&newLine);
            } else {
                addLine(&newLine);
            }
        }
        fclose(file);

        // Run the program after loading
        runProgram(0);
    } else {
        // Interactive mode
        printf("CBSH - Commodore BASIC Shell, version 1.0\n \n");
        printf("READY.\n");

        while (1) {
            printf("> ");
            if (fgets(lineBuffer, sizeof(lineBuffer), stdin) == NULL) {
                break; // Exit on EOF (Ctrl+D)
            }

            Line newLine;
            tokenizeLine(lineBuffer, &newLine);

            // If line number is 0, it's an immediate command
            if (newLine.lineNumber == 0) {
                if (newLine.numTokens > 0) {
                    if (newLine.tokens[0].keyword == KW_LIST) {
                        executeList(0, -1); // LIST with optional line range
                    } else if (newLine.tokens[0].keyword == KW_NEW) {
                        executeNew();
                    } else if (newLine.tokens[0].keyword == KW_RUN) {
                        runProgram(0);
                    } else {
                        executeLine(&newLine);
                    }
                }
            } else {
                // If it has a line number, add it to the program
                addLine(&newLine);
            }
        }
    }

    return 0;
}
