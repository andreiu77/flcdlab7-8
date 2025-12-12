#ifndef LL1_PARSER_H
#define LL1_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SYMBOLS 100
#define MAX_RULES 100
#define MAX_RHS 10
#define STR_LEN 50
#define EPSILON "ε"
#define END_MARKER "$"

// --- Data Structures ---

// Tree Node for the Parse Tree
typedef struct TreeNode {
    int index;
    char symbol[STR_LEN];
    int father;
    int sibling;
} TreeNode;

// Grammar Rule
typedef struct Rule {
    int id;
    char lhs[STR_LEN];
    char rhs[MAX_RHS][STR_LEN];
    int rhs_count;
} Rule;

// A generic Set of strings (for First/Follow sets)
typedef struct StringSet {
    char items[MAX_SYMBOLS][STR_LEN];
    int count;
} StringSet;

// Entry for the Parsing Table
typedef struct TableEntry {
    char lhs[STR_LEN];
    char term[STR_LEN];
    int rule_id;
} TableEntry;

// Main Parser Struct
typedef struct LL1Parser {
    char terminals[MAX_SYMBOLS][STR_LEN];
    int term_count;
    
    char non_terminals[MAX_SYMBOLS][STR_LEN];
    int non_term_count;

    Rule rules[MAX_RULES];
    int rule_count;

    // Maps non-terminal index to a StringSet
    StringSet first[MAX_SYMBOLS];
    StringSet follow[MAX_SYMBOLS];

    // Parsing table (Linear list for simplicity)
    TableEntry table[MAX_RULES * MAX_SYMBOLS];
    int table_count;

    // Tree Data
    TreeNode* nodes[1000];
    int node_count;
} LL1Parser;

// --- Function Prototypes ---

// Set Helpers
void set_add(StringSet* set, const char* val);
bool set_contains(StringSet* set, const char* val);
int set_union(StringSet* dest, StringSet* src);
int set_union_minus_epsilon(StringSet* dest, StringSet* src);

// Grammar Logic
void init_parser(LL1Parser* p);
bool is_terminal(LL1Parser* p, const char* s);
bool is_non_terminal(LL1Parser* p, const char* s);
int get_nt_index(LL1Parser* p, const char* s);
void get_first_of_sequence(LL1Parser* p, char sequence[][STR_LEN], int count, StringSet* result);

// Construction
void compute_first_sets(LL1Parser* p);
void compute_follow_sets(LL1Parser* p);
void build_parsing_table(LL1Parser* p);

// Parsing
TreeNode* create_node(LL1Parser* p, const char* symbol, int father_index);
void load_and_parse(LL1Parser* p, const char* filename);
void print_tree_table(LL1Parser* p);
void print_visual_tree(LL1Parser* p);

#endif