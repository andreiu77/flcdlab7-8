#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ===================== TOKENS ===================== */

typedef enum {
    TOK_EOF,
    TOK_INT, TOK_IF, TOK_ELSE, TOK_WHILE,
    TOK_READ, TOK_WRITE,
    TOK_IDENTIFIER,
    TOK_INT_CONST, TOK_STRING_CONST,
    TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_ASSIGN,
    TOK_LT, TOK_GT,
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    TOK_COMMA, TOK_SEMICOLON
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[256];
} Token;

Token current_token;
FILE *source;

/* ===================== UTIL ===================== */

void print_prod(const char *p) {
    printf("Production: %s\n", p);
}

void syntax_error(const char *msg) {
    printf("Syntax error: %s (near '%s')\n", msg, current_token.lexeme);
    exit(1);
}

/* ===================== LEXER ===================== */

void skip_ws() {
    char c;
    while ((c = fgetc(source)) != EOF) {
        if (!isspace(c)) {
            ungetc(c, source);
            return;
        }
    }
}

void next_token() {
    skip_ws();
    char c = fgetc(source);

    if (c == EOF) {
        current_token.type = TOK_EOF;
        strcpy(current_token.lexeme, "EOF");
        return;
    }

    if (c == '"') {
        int i = 0;
        while ((c = fgetc(source)) != '"' && c != EOF)
            current_token.lexeme[i++] = c;
        current_token.lexeme[i] = '\0';
        current_token.type = TOK_STRING_CONST;
        return;
    }

    if (isalpha(c)) {
        int i = 0;
        current_token.lexeme[i++] = c;
        while (isalnum(c = fgetc(source)))
            current_token.lexeme[i++] = c;
        current_token.lexeme[i] = '\0';
        ungetc(c, source);

        if (!strcmp(current_token.lexeme, "int")) current_token.type = TOK_INT;
        else if (!strcmp(current_token.lexeme, "if")) current_token.type = TOK_IF;
        else if (!strcmp(current_token.lexeme, "else")) current_token.type = TOK_ELSE;
        else if (!strcmp(current_token.lexeme, "while")) current_token.type = TOK_WHILE;
        else if (!strcmp(current_token.lexeme, "read")) current_token.type = TOK_READ;
        else if (!strcmp(current_token.lexeme, "write")) current_token.type = TOK_WRITE;
        else current_token.type = TOK_IDENTIFIER;
        return;
    }

    if (isdigit(c)) {
        int i = 0;
        current_token.lexeme[i++] = c;
        while (isdigit(c = fgetc(source)))
            current_token.lexeme[i++] = c;
        current_token.lexeme[i] = '\0';
        ungetc(c, source);
        current_token.type = TOK_INT_CONST;
        return;
    }

    current_token.lexeme[0] = c;
    current_token.lexeme[1] = '\0';

    switch (c) {
        case '+': current_token.type = TOK_PLUS; break;
        case '-': current_token.type = TOK_MINUS; break;
        case '*': current_token.type = TOK_MUL; break;
        case '/': current_token.type = TOK_DIV; break;
        case '=': current_token.type = TOK_ASSIGN; break;
        case '<': current_token.type = TOK_LT; break;
        case '>': current_token.type = TOK_GT; break;
        case '(': current_token.type = TOK_LPAREN; break;
        case ')': current_token.type = TOK_RPAREN; break;
        case '{': current_token.type = TOK_LBRACE; break;
        case '}': current_token.type = TOK_RBRACE; break;
        case ',': current_token.type = TOK_COMMA; break;
        case ';': current_token.type = TOK_SEMICOLON; break;
        default: syntax_error("Unknown character");
    }
}

void match(TokenType t) {
    if (current_token.type == t)
        next_token();
    else
        syntax_error("Unexpected token");
}

/* ===================== PARSER ===================== */

void expression();
void statement_list();
void statement();
void condition();

/* ---------- Expressions ---------- */

void factor() {
    if (current_token.type == TOK_IDENTIFIER) {
        match(TOK_IDENTIFIER);
        print_prod("factor -> identifier");
    } else if (current_token.type == TOK_INT_CONST) {
        match(TOK_INT_CONST);
        print_prod("factor -> integer_const");
    } else if (current_token.type == TOK_STRING_CONST) {
        match(TOK_STRING_CONST);
        print_prod("string_factor -> string_const");
    } else {
        syntax_error("Invalid factor");
    }
}

void term() {
    factor();
    print_prod("term -> factor");
}

void arith_expr() {
    term();
    if (current_token.type == TOK_PLUS) {
        match(TOK_PLUS);
        arith_expr();
        print_prod("arith_expr -> term + arith_expr");
    } else {
        print_prod("arith_expr -> term");
    }
}

void expression() {
    if (current_token.type == TOK_STRING_CONST) {
        factor();
        print_prod("expression -> string_expr");
    } else {
        arith_expr();
        print_prod("expression -> arith_expr");
    }
}

/* ---------- Condition ---------- */

void condition() {
    expression();
    if (current_token.type == TOK_LT || current_token.type == TOK_GT) {
        if (current_token.type == TOK_LT)
            print_prod("relational_operator -> <");
        else
            print_prod("relational_operator -> >");
        next_token();
    } else {
        syntax_error("Expected relational operator");
    }
    expression();
    print_prod("condition -> expression relational_operator expression");
}

/* ---------- Statements ---------- */

void io_stmt() {
    if (current_token.type == TOK_READ) {
        match(TOK_READ);
        match(TOK_LPAREN);
        match(TOK_IDENTIFIER);
        match(TOK_RPAREN);
        print_prod("io_stmt -> read ( identifier )");
    } else {
        match(TOK_WRITE);
        match(TOK_LPAREN);
        expression();
        match(TOK_RPAREN);
        print_prod("io_stmt -> write ( expression )");
    }
}

void assignment_stmt() {
    match(TOK_IDENTIFIER);
    match(TOK_ASSIGN);
    expression();
    print_prod("assignment_stmt -> identifier = expression");
}

void conditional_stmt() {
    match(TOK_IF);
    match(TOK_LPAREN);
    condition();
    match(TOK_RPAREN);
    match(TOK_LBRACE);
    statement_list();
    match(TOK_RBRACE);

    if (current_token.type == TOK_ELSE) {
        match(TOK_ELSE);
        match(TOK_LBRACE);
        statement_list();
        match(TOK_RBRACE);
        print_prod("conditional_stmt -> if ( condition ) { statement_list } else { statement_list }");
    } else {
        print_prod("conditional_stmt -> if ( condition ) { statement_list }");
    }
}

void while_stmt() {
    match(TOK_WHILE);
    match(TOK_LPAREN);
    condition();
    match(TOK_RPAREN);
    match(TOK_LBRACE);
    statement_list();
    match(TOK_RBRACE);
    print_prod("while_stmt -> while ( condition ) { statement_list }");
}

void statement() {
    if (current_token.type == TOK_READ || current_token.type == TOK_WRITE) {
        io_stmt();
        print_prod("statement -> io_stmt");
    } else if (current_token.type == TOK_IF) {
        conditional_stmt();
        print_prod("statement -> conditional_stmt");
    } else if (current_token.type == TOK_WHILE) {
        while_stmt();
        print_prod("statement -> while_stmt");
    } else {
        assignment_stmt();
        print_prod("statement -> assignment_stmt");
    }
}

void statement_list() {
    statement();
    match(TOK_SEMICOLON);
    print_prod("statement_list -> statement ;");
    if (current_token.type == TOK_IDENTIFIER ||
        current_token.type == TOK_READ ||
        current_token.type == TOK_WRITE ||
        current_token.type == TOK_IF ||
        current_token.type == TOK_WHILE) {
        statement_list();
        print_prod("statement_list -> statement ; statement_list");
    }
}

/* ---------- Program ---------- */

void declaration_list() {
    match(TOK_INT);
    match(TOK_IDENTIFIER);
    print_prod("id_list -> identifier");
    if (current_token.type == TOK_COMMA) {
        match(TOK_COMMA);
        match(TOK_IDENTIFIER);
        print_prod("id_list -> identifier , id_list");
    }
    match(TOK_SEMICOLON);
    print_prod("declaration -> int id_list");
    print_prod("declaration_list -> declaration ;");
}

void au_program() {
    match(TOK_IDENTIFIER); /* AU */
    match(TOK_IDENTIFIER); /* Language */
    declaration_list();
    statement_list();
    print_prod("au_program -> AU Language declaration_list statement_list");
}

/* ===================== MAIN ===================== */

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <program.txt>\n", argv[0]);
        return 1;
    }

    source = fopen(argv[1], "r");
    if (!source) {
        perror("File error");
        return 1;
    }

    printf("Starting parsing...\n");
    next_token();
    au_program();

    if (current_token.type != TOK_EOF)
        syntax_error("Extra tokens after program end");

    printf("Parsing completed successfully.\n");
    return 0;
}
