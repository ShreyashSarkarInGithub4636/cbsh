# CBSH - Commodore BASIC Shell

CBSH is a simple BASIC interpreter implemented as a shell for Linux, Unix, and other similar platforms. It aims to provide a nostalgic BASIC programming experience within a familiar shell environment.

**What is CBSH?**

Essentially, CBSH is basically  a command-line interactive shell that lets you write and execute BASIC programs. Instead of running in a dedicated BASIC environment (like on a Commodore 64), it integrates with your existing terminal, allowing you to use it alongside other shell commands.

**Currently Supported BASIC Commands:**

While CBSH is still under development, it currently supports the following subset of standard BASIC commands:

*   **`LIST`:** Displays the program listing. You can optionally specify a line number or a range of line numbers (e.g., `LIST 10`, `LIST 20-50`).
*   **`NEW`:** Clears the current program from memory, allowing you to start a new one.
*   **`PRINT`:** Outputs text, numbers, or variable values to the console.
    *   Example: `PRINT "Hello, world!"`, `PRINT X`, `PRINT 10 + 5`
*   **`INPUT`:** Prompts the user to enter a value and assigns it to a variable.
    *   Example: `INPUT "Enter your name: ", A$`
*   **`IF...THEN`:** Provides conditional execution. If the condition is true, the statement after `THEN` is executed.
    *   Example: `IF X > 10 THEN PRINT "X is greater than 10"`
*   **`FOR...NEXT`:** Creates loops that repeat a block of code a specified number of times. It supports the optional `STEP` keyword to control the loop increment.
    *   Example: `FOR I = 1 TO 10: PRINT I: NEXT I`
    *   Example with `STEP`: `FOR J = 10 TO 1 STEP -1: PRINT J: NEXT J`
*   **`LET`:** Assigns a value to a variable (optional in most cases, as you can often assign directly, e.g., `X = 5`).
    *   Example: `LET A = 10`, `LET B$ = "Hello"`
*   **`REM`:**  Indicates a comment in the code (remarks). These lines are ignored during execution.
    *   Example: `10 REM This is a comment`
*   **`GOTO`:** Unconditionally jumps to a specified line number.
    *   Example: `GOTO 100`
*   **`GOSUB`:** Jumps to a subroutine (a block of code starting at a specific line number).
    *   Example: `GOSUB 200`
*   **`RETURN`:** Used within a subroutine to return execution to the line after the `GOSUB` call.
*   **`DATA`:** Stores data values within the program that can be accessed using the `READ` command.
    *   Example: `100 DATA 1, 2, 3, "Hello"`
*   **`READ`:** Reads values from `DATA` statements and assigns them to variables.
    *   Example: `READ A, B, C, D$`
*   **`RESTORE`:** Resets the `DATA` pointer, allowing you to read `DATA` values from the beginning again.
*   **`END`:** Stops program execution.
*  **`SET`:** Used to set environment variables within the shell. (Currently only supports `emu_amiga_m68k` set to either `TRUE` or `FALSE`).
*   **`TAB`:** Used within a `PRINT` statement to move the cursor to a specific column.

**Commands Still Under Development:**

The following commands are planned but not yet fully implemented:

*   `SQR`
*   `RND`
*   `SIN`
*   `USR`
*   `CLEAR`
*   `STOP`
*   `ABS`
*   `INT`
*   `STEP`

**How to Compile:**

1. **Prerequisites:** Make sure you have the Autotools installed on your system (Autoconf, Automake, and their dependencies). You can typically install them using your distribution's package manager. For example, on Debian/Ubuntu:

    ```bash
    sudo apt-get install autoconf automake
    ```

2. **Generate Build Files:** Run the provided `autogen.sh` script in the project's root directory:

    ```bash
    ./autogen.sh
    ./configure
    ```

4. **Compile:** Use `make` to compile the code:

    ```bash
    make
    ```

5. **Run:** The executable `cbsh` will be created in the current directory. You can run it from here:

    ```bash
    ./cbsh
    ```

**Example Usage:**

```basic
> 10 PRINT "HELLO, WORLD!"
> 20 GOTO 10
> RUN
HELLO, WORLD!
HELLO, WORLD!
HELLO, WORLD!
... (infinite loop, press Ctrl+C to stop)
> NEW
> 10 FOR I = 1 TO 5
> 20 PRINT "THIS IS LINE"; I
> 30 NEXT I
> RUN
THIS IS LINE 1
THIS IS LINE 2
THIS IS LINE 3
THIS IS LINE 4
THIS IS LINE 5
>```
