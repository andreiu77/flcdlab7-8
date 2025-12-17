<au_program> ::= AU Language <declaration_list> <statement_list> <declaration_list> ::= <declaration> ; | <declaration> ; <declaration_list> <declaration> ::= int <id_list> | char <id_list> | string <id_list> | <list_declaration> | <struct_declaration> | <struct_var_declaration> <list_declaration> ::= int [ <integer_const> ] <identifier> | char [ <integer_const> ] <identifier> | string [ <integer_const> ] <identifier> | <identifier> [ <integer_const> ] <identifier> <struct_declaration> ::= struct <identifier> { <declaration_list> } <struct_var_declaration> ::= <identifier> <id_list> <id_list> ::= <identifier> | <identifier> , <id_list> <statement_list> ::= <statement> ; | <statement> ; <statement_list> <statement> ::= <assignment_stmt> | <conditional_stmt> | <while_stmt> | <io_stmt> | <comment_stmt> <comment_stmt> ::= # (<letter> | <digit> | <space> | <symbol>)* <assignment_stmt> ::= <identifier> = <expression> | <identifier> [ <expression> ] = <expression> | <identifier> . <identifier> = <expression> | <identifier> [ <expression> ] . <identifier> = <expression> <conditional_stmt> ::= if ( <condition> ) { <statement_list> } | if ( <condition> ) { <statement_list> } else { <statement_list> } <while_stmt> ::= while ( <condition> ) { <statement_list> } <io_stmt> ::= read ( <identifier> ) | read ( <identifier> [ <expression> ] ) | read ( <identifier> . <identifier> ) | read ( <identifier> [ <expression> ] . <identifier> ) | write ( <expression> ) <expression> ::= <arith_expr> | <string_expr> <arith_expr> ::= <term> | <term> <add_operator> <arith_expr> <term> ::= <factor> | <factor> <mul_operator> <term> <factor> ::= <identifier> | <identifier> [ <expression> ] | <identifier> . <identifier> | <identifier> [ <expression> ] . <identifier> | <integer_const> | ( <expression> ) <add_operator> ::= + | - <mul_operator> ::= * | / | % <string_expr> ::= <string_factor> | <string_factor> + <string_expr> <string_factor> ::= <identifier> | <identifier> [ <expression> ] | <identifier> . <identifier> | <identifier> [ <expression> ] . <identifier> | <string_const> <condition> ::= <expression> <relational_operator> <expression> <relational_operator> ::= == | <> | < | > | <= | >= <constant> ::= <integer_const> | <character_const> | <string_const> <integer_const> ::= 0 | <nonzerodigit> <digit>* <character_const> ::= ' (<letter> | <digit> | <symbol> | <space>) ' <string_const> ::= " (<letter> | <digit> | <space> | <symbol>)* " <identifier> ::= <letter> (<letter> | <digit> | _ )* <letter> ::= A | ... | Z | a | ... | z <digit> ::= 0 | 1 | ... | 9 <nonzerodigit> ::= 1 | 2 | ... | 9 <space> ::= ' ' <symbol> ::= ! | ? | . | , | : | ; | _ | + | - | * | / | % | = | < | > | 
for this bnf code generate a raw C parser (no yacc allowed). i also have a parser for this bnf in yacc if it is easier for you to write code

output: 
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ===================== TOKENS ===================== */

typedef enum {
    TOK_EOF,
    TOK_INT, TOK_CHAR, TOK_STRING, TOK_STRUCT,
    TOK_IF, TOK_ELSE, TOK_WHILE,
    TOK_READ, TOK_WRITE,
    TOK_IDENTIFIER,
    TOK_INT_CONST, TOK_CHAR_CONST, TOK_STRING_CONST,
    TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV, TOK_MOD,
    TOK_ASSIGN,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_COMMA, TOK_SEMICOLON, TOK_DOT,
    TOK_HASH
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[256];
} Token;

Token current_token;
FILE *source;

/* ===================== ERROR ===================== */

void syntax_error(const char *msg) {
    printf("Syntax error: %s (near '%s')\n", msg, current_token.lexeme);
    exit(1);
}

/* ===================== LEXER ===================== */

void skip_whitespace() {
    char c;
    while ((c = fgetc(source)) != EOF) {
        if (!isspace(c)) {
            ungetc(c, source);
            return;
        }
    }
}

int is_identifier_start(char c) {
    return isalpha(c);
}

int is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

void next_token() {
    skip_whitespace();
    char c = fgetc(source);

    if (c == EOF) {
        current_token.type = TOK_EOF;
        strcpy(current_token.lexeme, "EOF");
        return;
    }

    /* Identifiers & Keywords */
    if (is_identifier_start(c)) {
        int i = 0;
        current_token.lexeme[i++] = c;
        while (is_identifier_char(c = fgetc(source)))
            current_token.lexeme[i++] = c;
        current_token.lexeme[i] = '\0';
        ungetc(c, source);

        if (!strcmp(current_token.lexeme, "int")) current_token.type = TOK_INT;
        else if (!strcmp(current_token.lexeme, "char")) current_token.type = TOK_CHAR;
        else if (!strcmp(current_token.lexeme, "string")) current_token.type = TOK_STRING;
        else if (!strcmp(current_token.lexeme, "struct")) current_token.type = TOK_STRUCT;
        else if (!strcmp(current_token.lexeme, "if")) current_token.type = TOK_IF;
        else if (!strcmp(current_token.lexeme, "else")) current_token.type = TOK_ELSE;
        else if (!strcmp(current_token.lexeme, "while")) current_token.type = TOK_WHILE;
        else if (!strcmp(current_token.lexeme, "read")) current_token.type = TOK_READ;
        else if (!strcmp(current_token.lexeme, "write")) current_token.type = TOK_WRITE;
        else current_token.type = TOK_IDENTIFIER;
        return;
    }

    /* Numbers */
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

    /* Symbols */
    current_token.lexeme[0] = c;
    current_token.lexeme[1] = '\0';

    switch (c) {
        case '+': current_token.type = TOK_PLUS; break;
        case '-': current_token.type = TOK_MINUS; break;
        case '*': current_token.type = TOK_MUL; break;
        case '/': current_token.type = TOK_DIV; break;
        case '%': current_token.type = TOK_MOD; break;
        case '=':
            if ((c = fgetc(source)) == '=') current_token.type = TOK_EQ;
            else { current_token.type = TOK_ASSIGN; ungetc(c, source); }
            break;
        case '<':
            if ((c = fgetc(source)) == '=') current_token.type = TOK_LE;
            else if (c == '>') current_token.type = TOK_NEQ;
            else { current_token.type = TOK_LT; ungetc(c, source); }
            break;
        case '>':
            if ((c = fgetc(source)) == '=') current_token.type = TOK_GE;
            else { current_token.type = TOK_GT; ungetc(c, source); }
            break;
        case '(': current_token.type = TOK_LPAREN; break;
        case ')': current_token.type = TOK_RPAREN; break;
        case '{': current_token.type = TOK_LBRACE; break;
        case '}': current_token.type = TOK_RBRACE; break;
        case '[': current_token.type = TOK_LBRACKET; break;
        case ']': current_token.type = TOK_RBRACKET; break;
        case ',': current_token.type = TOK_COMMA; break;
        case ';': current_token.type = TOK_SEMICOLON; break;
        case '.': current_token.type = TOK_DOT; break;
        case '#': current_token.type = TOK_HASH; break;
        default:
            syntax_error("Unknown character");
    }
}

/* ===================== PARSER ===================== */

void match(TokenType t) {
    if (current_token.type == t)
        next_token();
    else
        syntax_error("Unexpected token");
}

/* Forward declarations */
void au_program();
void declaration_list();
void declaration();
void id_list();
void statement_list();
void statement();
void assignment_stmt();
void conditional_stmt();
void while_stmt();
void io_stmt();
void expression();
void arith_expr();
void term();
void factor();
void condition();

/* Grammar */

void au_program() {
    match(TOK_IDENTIFIER); // AU
    match(TOK_IDENTIFIER); // Language
    declaration_list();
    statement_list();
}

void declaration_list() {
    declaration();
    match(TOK_SEMICOLON);
    if (current_token.type == TOK_INT || current_token.type == TOK_CHAR ||
        current_token.type == TOK_STRING || current_token.type == TOK_STRUCT ||
        current_token.type == TOK_IDENTIFIER)
        declaration_list();
}

void declaration() {
    if (current_token.type == TOK_INT || current_token.type == TOK_CHAR || current_token.type == TOK_STRING) {
        next_token();
        id_list();
    } else if (current_token.type == TOK_STRUCT) {
        match(TOK_STRUCT);
        match(TOK_IDENTIFIER);
        match(TOK_LBRACE);
        declaration_list();
        match(TOK_RBRACE);
    } else if (current_token.type == TOK_IDENTIFIER) {
        match(TOK_IDENTIFIER);
        id_list();
    } else syntax_error("Invalid declaration");
}

void id_list() {
    match(TOK_IDENTIFIER);
    if (current_token.type == TOK_COMMA) {
        match(TOK_COMMA);
        id_list();
    }
}

void statement_list() {
    statement();
    match(TOK_SEMICOLON);
    if (current_token.type == TOK_IDENTIFIER || current_token.type == TOK_IF ||
        current_token.type == TOK_WHILE || current_token.type == TOK_READ ||
        current_token.type == TOK_WRITE || current_token.type == TOK_HASH)
        statement_list();
}

void statement() {
    if (current_token.type == TOK_IDENTIFIER) assignment_stmt();
    else if (current_token.type == TOK_IF) conditional_stmt();
    else if (current_token.type == TOK_WHILE) while_stmt();
    else if (current_token.type == TOK_READ || current_token.type == TOK_WRITE) io_stmt();
    else if (current_token.type == TOK_HASH) next_token();
    else syntax_error("Invalid statement");
}

void assignment_stmt() {
    match(TOK_IDENTIFIER);
    match(TOK_ASSIGN);
    expression();
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
}

void io_stmt() {
    if (current_token.type == TOK_READ) {
        match(TOK_READ);
        match(TOK_LPAREN);
        match(TOK_IDENTIFIER);
        match(TOK_RPAREN);
    } else {
        match(TOK_WRITE);
        match(TOK_LPAREN);
        expression();
        match(TOK_RPAREN);
    }
}

void expression() {
    arith_expr();
}

void arith_expr() {
    term();
    if (current_token.type == TOK_PLUS || current_token.type == TOK_MINUS) {
        next_token();
        arith_expr();
    }
}

void term() {
    factor();
    if (current_token.type == TOK_MUL || current_token.type == TOK_DIV || current_token.type == TOK_MOD) {
        next_token();
        term();
    }
}

void factor() {
    if (current_token.type == TOK_IDENTIFIER)
        match(TOK_IDENTIFIER);
    else if (current_token.type == TOK_INT_CONST)
        match(TOK_INT_CONST);
    else if (current_token.type == TOK_LPAREN) {
        match(TOK_LPAREN);
        expression();
        match(TOK_RPAREN);
    } else syntax_error("Invalid factor");
}

void condition() {
    expression();
    if (current_token.type == TOK_EQ || current_token.type == TOK_NEQ ||
        current_token.type == TOK_LT || current_token.type == TOK_GT ||
        current_token.type == TOK_LE || current_token.type == TOK_GE)
        next_token();
    else syntax_error("Expected relational operator");
    expression();
}

/* ===================== MAIN ===================== */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <source_file.au>\n", argv[0]);
        return 1;
    }

    source = fopen(argv[1], "r");
    if (!source) {
        perror("Cannot open file");
        return 1;
    }

    next_token();
    au_program();

    if (current_token.type != TOK_EOF)
        syntax_error("Extra input after program end");

    printf("✔ Parsing successful!\n");
    fclose(source);
    return 0;
}

```
AU Language int n, i; read(n); i = 1; while (i < n) { write(i); if (i > 5) { write("Big"); } else { write("Small"); }; i = i + 1; }; write("Done"); for the program above i got Syntax error: Unknown character (near '"')
I need it to look something like this (this output is from yacc): Starting parsing... Production: id_list -> identifier Production: id_list -> identifier , id_list Production: declaration -> int id_list Production: declaration_list -> declaration ; Production: io_stmt -> read ( identifier ) Production: statement -> io_stmt Production: factor -> integer_const Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: assignment_stmt -> identifier = expression Production: statement -> assignment_stmt Production: factor -> identifier Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: relational_operator -> < Production: factor -> identifier Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: condition -> expression relational_operator expression Production: factor -> identifier Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: io_stmt -> write ( expression ) Production: statement -> io_stmt Production: factor -> identifier Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: relational_operator -> > Production: factor -> integer_const Production: term -> factor Production: arith_expr -> term Production: expression -> arith_expr Production: condition -> expression relational_operator expression Production: string_factor -> string_const Production: string_expr -> string_factor Production: expression -> string_expr Production: io_stmt -> write ( expression ) Production: statement -> io_stmt Production: statement_list -> statement ; Production: string_factor -> string_const Production: string_expr -> string_factor Production: expression -> string_expr Production: io_stmt -> write ( expression ) Production: statement -> io_stmt Production: statement_list -> statement ; Production: conditional_stmt -> if ( condition ) { statement_list } else { statement_list } Production: statement -> conditional_stmt Production: factor -> identifier Production: term -> factor Production: add_operator -> + Production: factor -> integer_const Production: term -> factor Production: arith_expr -> term Production: arith_expr -> term add_operator arith_expr Production: expression -> arith_expr Production: assignment_stmt -> identifier = expression Production: statement -> assignment_stmt Production: statement_list -> statement ; Production: statement_list -> statement ; statement_list Production: statement_list -> statement ; statement_list Production: while_stmt -> while ( condition ) { statement_list } Production: statement -> while_stmt Production: string_factor -> string_const Production: string_expr -> string_factor Production: expression -> string_expr Production: io_stmt -> write ( expression ) Production: statement -> io_stmt Production: statement_list -> statement ; Production: statement_list -> statement ; statement_list Production: statement_list -> statement ; statement_list Production: statement_list -> statement ; statement_list Production: au_program -> AU Language declaration_list statement_list Parsing completed successfully.

Starting parsing...
Syntax error: Unexpected token (near 'AU')

Syntax error: Extra tokens after program end (near 'while') why this syntax error at the end?

🧠 Root cause (important)

Your grammar says:

statement_list ::= statement ;
                | statement ; statement_list


But your implementation only supports:

statement_list -> statement ;


Same problem for:

while_stmt

conditional_stmt

relational operators <, >

block { statement_list }

string expressions inside write

So the parser stops too early.


