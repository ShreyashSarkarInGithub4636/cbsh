#include "cbsh.h"

// Run the program from a specific line number
void runProgram(int startLine) {
    currentLine = 0;
    nextLine = startLine;
    running = true;

    // Find the starting line index
    if (startLine != 0) {
        int found = 0;
        for (int i = 0; i < numLines; i++) {
            if (program[i].lineNumber == startLine) {
                currentLine = i;
                nextLine = i;
                found = 1;
                break;
            }
        }
        if (!found) {
            printf("Undefined line %d\n", startLine);
            running = false;
            return;
        }
    }

    while (running) {
        if (nextLine < numLines) {
            currentLine = nextLine;
            nextLine++;
            executeLine(&program[currentLine]);
        } else {
            running = false; // End of program
        }
    }
}

// Add a new line to the program or replace an existing line
void addLine(Line *newLine) {
    // Check if the line number already exists
    for (int i = 0; i < numLines; i++) {
        if (program[i].lineNumber == newLine->lineNumber) {
            // Replace the existing line
            program[i] = *newLine;
            return;
        }
    }

    // Add the new line
    if (numLines < MAX_NUM_LINES) {
        program[numLines++] = *newLine;

        // Sort lines by line number (simple bubble sort for now)
        for (int i = 0; i < numLines - 1; i++) {
            for (int j = 0; j < numLines - i - 1; j++) {
                if (program[j].lineNumber > program[j + 1].lineNumber) {
                    Line temp = program[j];
                    program[j] = program[j + 1];
                    program[j + 1] = temp;
                }
            }
        }
    } else {
        printf("Program too large\n");
    }
}
