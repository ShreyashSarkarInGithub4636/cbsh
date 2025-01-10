#include "cbsh.h"

// Find a variable by name (case-insensitive)
Variable *findVariable(const char *name) {
    for (int i = 0; i < numVariables; i++) {
        if (strcasecmp(variables[i].name, name) == 0) {
            return &variables[i];
        }
    }
    return NULL;
}

// Get the value of a variable (or literal)
double getNumericValue(Token *token) {
    if (token->type == TOKEN_NUMBER) {
        return atof(token->value);
    } else if (token->type == TOKEN_IDENTIFIER) {
        Variable *var = findVariable(token->value);
        if (var == NULL) {
            printf("Undefined variable: %s\n", token->value);
            return 0; // Or handle the error appropriately
        }
        if (var->type != VAR_TYPE_NUMERIC) {
            printf("Type mismatch: %s is not a numeric variable\n", token->value);
            return 0;
        }
        return var->numValue;
    }
    printf("Invalid numeric value\n");
    return 0;
}

// Get string value
char *getStringValue(Token *token) {
    if (token->type == TOKEN_STRING) {
        return token->value; // Directly return the string literal
    } else if (token->type == TOKEN_IDENTIFIER) {
        Variable *var = findVariable(token->value);
        if (var) {
            if (var->type == VAR_TYPE_STRING) {
                return var->strValue;
            } else {
                printf("Type mismatch: %s is not a string variable\n", token->value);
                return ""; // Handle error: not a string variable
            }
        } else {
            printf("Undefined variable: %s\n", token->value);
            return ""; // Handle error: variable not found
        }
    }
    printf("Invalid string value\n");
    return "";
}

// Function to check if a variable exists
bool variableExists(const char *name) {
    for (int i = 0; i < numVariables; i++) {
        if (strcasecmp(variables[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

// Function to add or update a variable
void addOrUpdateVariable(const char *name, VarType type, double numValue, const char *strValue) {
    Variable *var = findVariable(name);
    if (var) {
        // Update existing variable
        var->type = type;
        if (type == VAR_TYPE_NUMERIC) {
            var->numValue = numValue;
        } else {
            strcpy(var->strValue, strValue);
        }
    } else {
        // Add new variable
        if (numVariables < MAX_VARIABLES) {
            strcpy(variables[numVariables].name, name);
            variables[numVariables].type = type;
            if (type == VAR_TYPE_NUMERIC) {
                variables[numVariables].numValue = numValue;
            } else {
                strcpy(variables[numVariables].strValue, strValue);
            }
            numVariables++;
        } else {
            printf("Too many variables\n");
        }
    }
}
