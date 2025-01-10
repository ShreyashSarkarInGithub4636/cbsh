#include "cbsh.h"

// Evaluate an expression (handle only basic arithmetic for now)
double evaluateExpression(Token *tokens, int numTokens) {
    if (numTokens == 1) {
        return getNumericValue(&tokens[0]);
    } else if (numTokens == 3) {
        double val1 = getNumericValue(&tokens[0]);
        double val2 = getNumericValue(&tokens[2]);
        if (strcmp(tokens[1].value, "+") == 0) return val1 + val2;
        if (strcmp(tokens[1].value, "-") == 0) return val1 - val2;
        if (strcmp(tokens[1].value, "*") == 0) return val1 * val2;
        if (strcmp(tokens[1].value, "/") == 0) {
            if (val2 == 0) {
                printf("Division by zero\n");
                return 0;
            }
            return val1 / val2;
        }
        // Handle more operators here (e.g., <, >, =)
    }
    printf("Invalid expression\n");
    return 0;
}
