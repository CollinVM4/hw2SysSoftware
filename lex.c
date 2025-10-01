#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID_LEN 11
#define MAX_NUM_LEN 5
#define MAX_SOURCE_SIZE 10000
#define MAX_LEXEMES 500

enum token_type {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, oddsym_dup, eqlsym, neqsym,
    lessym, leqsym, gtrsym, geqsym, lparentsym,
    rparentsym, commasym, semicolonsym, periodsym, becomessym,
    beginsym, endsym, ifsym, thensym, whilesym,
    dosym, callsym, constsym, varsym, procsym,
    writesym, readsym, elsesym
};

typedef struct {
    char lexeme[MAX_ID_LEN + 1];
    int token;
    int value;
} lexeme;

const char *reserved[] = {
    "const","var","procedure","call","begin","end","if","then",
    "else","while","do","read","write","odd"
};
const int reservedTokens[] = {
    constsym, varsym, procsym, callsym, beginsym, endsym,
    ifsym, thensym, elsesym, whilesym, dosym,
    readsym, writesym, oddsym
};
const int numReserved = 14;

char source[MAX_SOURCE_SIZE];
lexeme table[MAX_LEXEMES];
int tableIndex = 0;

int isReserved(const char *word) {
    for (int i = 0; i < numReserved; i++) {
        if (strcmp(word, reserved[i]) == 0)
            return reservedTokens[i];
    }
    return 0;
}

void addLexeme(const char *word, int token, int value) {
    if (tableIndex >= MAX_LEXEMES) return;
    strncpy(table[tableIndex].lexeme, word, MAX_ID_LEN);
    table[tableIndex].token = token;
    table[tableIndex].value = value;
    tableIndex++;
}

void error(const char *msg, const char *context) {
    printf("ERROR: %s -> %s\n", msg, context);
}

void lexer(const char *input) {
    int i = 0;
    while (input[i] != '\0') {
        if (isspace(input[i])) { i++; continue; }

        if (input[i] == '/' && input[i+1] == '*') {
            i += 2;
            while (input[i] != '\0' && !(input[i] == '*' && input[i+1] == '/')) i++;
            if (input[i] == '\0') { error("Unterminated comment", ""); return; }
            i += 2;
            continue;
        }
        if (isalpha(input[i])) {
            char buffer[MAX_ID_LEN+5]; int j=0;
            while (isalnum(input[i]) && j < MAX_ID_LEN) buffer[j++] = input[i++];
            buffer[j] = '\0';
            if (isalnum(input[i])) {
                while (isalnum(input[i])) i++;
                error("Identifier too long", buffer);
                continue;
            }
            int res = isReserved(buffer);
            if (res) addLexeme(buffer, res, 0);
            else addLexeme(buffer, identsym, 0);
            continue;
        }

        if (isdigit(input[i])) {
            char buffer[MAX_NUM_LEN+5]; int j=0;
            while (isdigit(input[i]) && j < MAX_NUM_LEN) buffer[j++] = input[i++];
            buffer[j] = '\0';
            if (isdigit(input[i])) {
                while (isdigit(input[i])) i++;
                error("Number too long", buffer);
                continue;
            }
            addLexeme(buffer, numbersym, atoi(buffer));
            continue;
        }

        switch (input[i]) {
            case '+': addLexeme("+", plussym, 0); i++; break;
            case '-': addLexeme("-", minussym, 0); i++; break;
            case '*': addLexeme("*", multsym, 0); i++; break;
            case '/': addLexeme("/", slashsym, 0); i++; break;
            case '=': addLexeme("=", eqlsym, 0); i++; break;
            case '<':
                if (input[i+1] == '=') { addLexeme("<=", leqsym, 0); i+=2; }
                else if (input[i+1] == '>') { addLexeme("<>", neqsym, 0); i+=2; }
                else { addLexeme("<", lessym, 0); i++; }
                break;
            case '>':
                if (input[i+1] == '=') { addLexeme(">=", geqsym, 0); i+=2; }
                else { addLexeme(">", gtrsym, 0); i++; }
                break;
            case ':':
                if (input[i+1] == '=') { addLexeme(":=", becomessym, 0); i+=2; }
                else { error("Invalid symbol", ":"); i++; }
                break;
            case '(': addLexeme("(", lparentsym, 0); i++; break;
            case ')': addLexeme(")", rparentsym, 0); i++; break;
            case ',': addLexeme(",", commasym, 0); i++; break;
            case ';': addLexeme(";", semicolonsym, 0); i++; break;
            case '.': addLexeme(".", periodsym, 0); i++; break;
            default:
                {
                    char bad[2] = {input[i], '\0'};
                    error("Invalid symbol", bad);
                    i++;
                }
                break;
        }
    }
}

void printSource(const char *input) {
    printf("Source Program:\n%s\n\n", input);
}

void printLexemeTable() {
    printf("Lexeme Table:\n");
    printf("lexeme\t\ttoken type\n");
    for (int i=0; i<tableIndex; i++) {
        printf("%-12s %d\n", table[i].lexeme, table[i].token);
    }
    printf("\n");
}

void printTokenList() {
    printf("Token List:\n");
    for (int i=0; i<tableIndex; i++) {
        printf("%d ", table[i].token);
        if (table[i].token == identsym || table[i].token == numbersym) {
            printf("%s ", table[i].lexeme);
        }
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <sourcefile>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        perror("File open error");
        return 1;
    }

    fread(source, 1, MAX_SOURCE_SIZE-1, fp);
    fclose(fp);

    printSource(source);
    lexer(source);
    printLexemeTable();
    printTokenList();

    return 0;
}
