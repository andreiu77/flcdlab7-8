#include "ll1_parser.h"

// --- Set Operations ---

void set_add(StringSet* set, const char* val) {
    if (set_contains(set, val)) return;
    strcpy(set->items[set->count], val);
    set->count++;
}

bool set_contains(StringSet* set, const char* val) {
    for (int i = 0; i < set->count; i++) {
        if (strcmp(set->items[i], val) == 0) return true;
    }
    return false;
}

// Returns 1 if destination changed, 0 otherwise
int set_union(StringSet* dest, StringSet* src) {
    int changed = 0;
    for (int i = 0; i < src->count; i++) {
        if (!set_contains(dest, src->items[i])) {
            set_add(dest, src->items[i]);
            changed = 1;
        }
    }
    return changed;
}

// Union but exclude Epsilon
int set_union_minus_epsilon(StringSet* dest, StringSet* src) {
    int changed = 0;
    for (int i = 0; i < src->count; i++) {
        if (strcmp(src->items[i], EPSILON) != 0) {
            if (!set_contains(dest, src->items[i])) {
                set_add(dest, src->items[i]);
                changed = 1;
            }
        }
    }
    return changed;
}

// --- Helper Functions ---

bool is_terminal(LL1Parser* p, const char* s) {
    for (int i = 0; i < p->term_count; i++) {
        if (strcmp(p->terminals[i], s) == 0) return true;
    }
    return false;
}

bool is_non_terminal(LL1Parser* p, const char* s) {
    for (int i = 0; i < p->non_term_count; i++) {
        if (strcmp(p->non_terminals[i], s) == 0) return true;
    }
    return false;
}

int get_nt_index(LL1Parser* p, const char* s) {
    for (int i = 0; i < p->non_term_count; i++) {
        if (strcmp(p->non_terminals[i], s) == 0) return i;
    }
    return -1;
}

void add_rule(LL1Parser* p, int id, const char* lhs, int count, const char* r1, const char* r2, const char* r3, const char* r4, const char* r5, const char* r6) {
    Rule* r = &p->rules[p->rule_count++];
    r->id = id;
    strcpy(r->lhs, lhs);
    r->rhs_count = count;
    if(count > 0) strcpy(r->rhs[0], r1);
    if(count > 1) strcpy(r->rhs[1], r2);
    if(count > 2) strcpy(r->rhs[2], r3);
    if(count > 3) strcpy(r->rhs[3], r4);
    if(count > 4) strcpy(r->rhs[4], r5);
    if(count > 5) strcpy(r->rhs[5], r6);
}

// --- Initialization ---

void init_parser(LL1Parser* p) {
    p->rule_count = 0;
    p->term_count = 0;
    p->non_term_count = 0;
    p->table_count = 0;
    p->node_count = 0;

    // Define Terminals
    const char* terms[] = {
        "AU", "Language", "int", "char", "string", "struct", "if", "else", "while",
        "read", "write", "identifier", "integer_const", "string_const", "character_const",
        ";", ",", "[", "]", "{", "}", "(", ")", ".", "=", "+", "-", "*", "/", "%",
        "==", "<>", "<", ">", "<=", ">=", "#", EPSILON
    };
    for(int i=0; i<38; i++) strcpy(p->terminals[p->term_count++], terms[i]);

    // Define Non-Terminals
    const char* non_terms[] = {
        "<au_program>", "<declaration_list>", "<declaration>", "<declaration_tail>",
        "<type_decl_tail>", "<list_decl_tail>", "<statement_list>", "<statement>",
        "<assignment_stmt>", "<assignment_tail>", "<conditional_stmt>", "<else_part>",
        "<while_stmt>", "<io_stmt>", "<io_tail>", "<io_struct_suffix>", "<expression>",
        "<arith_expr>", "<arith_tail>", "<term>", "<term_tail>", "<factor>",
        "<factor_tail>", "<add_operator>", "<mul_operator>", "<condition>",
        "<relational_operator>", "<id_list>", "<id_list_tail>", "<struct_tail>"
    };
    for(int i=0; i<30; i++) strcpy(p->non_terminals[p->non_term_count++], non_terms[i]);

    // Define Rules (Hardcoded based on Python source)
    // Note: Using a helper `add_rule` to keep code concise.
    // 0 arguments are ignored by the helper if count is low.
    add_rule(p, 1, "<au_program>", 4, "AU", "Language", "<declaration_list>", "<statement_list>", "", "");
    add_rule(p, 2, "<declaration_list>", 3, "<declaration>", ";", "<declaration_list>", "", "", "");
    add_rule(p, 3, "<declaration_list>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 4, "<declaration>", 2, "int", "<type_decl_tail>", "", "", "", "");
    add_rule(p, 5, "<declaration>", 2, "char", "<type_decl_tail>", "", "", "", "");
    add_rule(p, 6, "<declaration>", 2, "string", "<type_decl_tail>", "", "", "", "");
    add_rule(p, 7, "<declaration>", 5, "struct", "identifier", "{", "<declaration_list>", "}", "");
    add_rule(p, 8, "<type_decl_tail>", 1, "<id_list>", "", "", "", "", "");
    add_rule(p, 9, "<type_decl_tail>", 4, "[", "integer_const", "]", "identifier", "", "");
    add_rule(p, 10, "<statement_list>", 3, "<statement>", ";", "<statement_list>", "", "", "");
    add_rule(p, 11, "<statement_list>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 12, "<statement>", 1, "<assignment_stmt>", "", "", "", "", "");
    add_rule(p, 13, "<statement>", 1, "<conditional_stmt>", "", "", "", "", "");
    add_rule(p, 14, "<statement>", 1, "<while_stmt>", "", "", "", "", "");
    add_rule(p, 15, "<statement>", 1, "<io_stmt>", "", "", "", "", "");
    add_rule(p, 16, "<assignment_stmt>", 2, "identifier", "<assignment_tail>", "", "", "", "");
    add_rule(p, 17, "<assignment_tail>", 2, "=", "<expression>", "", "", "", "");
    add_rule(p, 18, "<assignment_tail>", 6, "[", "<expression>", "]", "<factor_tail>", "=", "<expression>");
    add_rule(p, 19, "<assignment_tail>", 4, ".", "identifier", "=", "<expression>", "", "");
    add_rule(p, 20, "<conditional_stmt>", 8, "if", "(", "<condition>", ")", "{", "<statement_list>"); // Split logic not needed for logic, just storage
       // Wait, C doesn't support varargs nicely in simple translation. 
       // Rule 20 has 8 tokens. `add_rule` handles up to 6. Extending manually below for specific rule:
       p->rules[p->rule_count-1].rhs[6][0] = '}'; p->rules[p->rule_count-1].rhs[7][0] = '\0'; strcpy(p->rules[p->rule_count-1].rhs[7], "<else_part>");
       p->rules[p->rule_count-1].rhs_count = 8;

    add_rule(p, 21, "<else_part>", 4, "else", "{", "<statement_list>", "}", "", "");
    add_rule(p, 22, "<else_part>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 23, "<while_stmt>", 7, "while", "(", "<condition>", ")", "{", "<statement_list>");
       strcpy(p->rules[p->rule_count-1].rhs[6], "}"); p->rules[p->rule_count-1].rhs_count = 7;

    add_rule(p, 24, "<io_stmt>", 5, "read", "(", "identifier", "<io_tail>", ")", "");
    add_rule(p, 25, "<io_stmt>", 4, "write", "(", "<expression>", ")", "", "");
    add_rule(p, 26, "<io_tail>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 27, "<io_tail>", 4, "[", "<expression>", "]", "<io_struct_suffix>", "", "");
    add_rule(p, 28, "<expression>", 2, "<term>", "<arith_tail>", "", "", "", "");
    add_rule(p, 29, "<arith_tail>", 3, "<add_operator>", "<term>", "<arith_tail>", "", "", "");
    add_rule(p, 30, "<arith_tail>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 31, "<term>", 2, "<factor>", "<term_tail>", "", "", "", "");
    add_rule(p, 32, "<term_tail>", 3, "<mul_operator>", "<factor>", "<term_tail>", "", "", "");
    add_rule(p, 33, "<term_tail>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 34, "<factor>", 2, "identifier", "<factor_tail>", "", "", "", "");
    add_rule(p, 35, "<factor>", 1, "integer_const", "", "", "", "", "");
    add_rule(p, 36, "<factor>", 3, "(", "<expression>", ")", "", "", "");
    add_rule(p, 37, "<factor_tail>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 38, "<factor_tail>", 4, "[", "<expression>", "]", "<struct_tail>", "", "");
    add_rule(p, 39, "<factor_tail>", 2, ".", "identifier", "", "", "", "");
    add_rule(p, 40, "<id_list>", 2, "identifier", "<id_list_tail>", "", "", "", "");
    add_rule(p, 41, "<id_list_tail>", 2, ",", "<id_list>", "", "", "", "");
    add_rule(p, 42, "<id_list_tail>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 43, "<add_operator>", 1, "+", "", "", "", "", "");
    add_rule(p, 44, "<add_operator>", 1, "-", "", "", "", "", "");
    add_rule(p, 45, "<mul_operator>", 1, "*", "", "", "", "", "");
    add_rule(p, 46, "<mul_operator>", 1, "/", "", "", "", "", "");
    add_rule(p, 47, "<condition>", 3, "<expression>", "<relational_operator>", "<expression>", "", "", "");
    add_rule(p, 48, "<relational_operator>", 1, "==", "", "", "", "", "");
    add_rule(p, 49, "<relational_operator>", 1, "<", "", "", "", "", "");
    add_rule(p, 50, "<relational_operator>", 1, ">", "", "", "", "", "");
    add_rule(p, 51, "<factor>", 1, "string_const", "", "", "", "", "");
    add_rule(p, 52, "<declaration>", 2, "identifier", "<type_decl_tail>", "", "", "", "");
    add_rule(p, 53, "<io_tail>", 2, ".", "identifier", "", "", "", "");
    add_rule(p, 54, "<io_struct_suffix>", 2, ".", "identifier", "", "", "", "");
    add_rule(p, 55, "<io_struct_suffix>", 1, EPSILON, "", "", "", "", "");
    add_rule(p, 56, "<struct_tail>", 2, ".", "identifier", "", "", "", "");
    add_rule(p, 57, "<struct_tail>", 1, EPSILON, "", "", "", "", "");

    // Initialize Sets
    for(int i=0; i<MAX_SYMBOLS; i++) {
        p->first[i].count = 0;
        p->follow[i].count = 0;
    }

    compute_first_sets(p);
    compute_follow_sets(p);
    build_parsing_table(p);
}

// --- Computation Logic ---

void get_first_of_sequence(LL1Parser* p, char sequence[][STR_LEN], int count, StringSet* result) {
    result->count = 0;
    bool nullable = true;

    for (int i = 0; i < count; i++) {
        char* symbol = sequence[i];
        if (strcmp(symbol, EPSILON) == 0) continue;

        if (is_terminal(p, symbol)) {
            set_add(result, symbol);
            nullable = false;
            break;
        } else if (is_non_terminal(p, symbol)) {
            int idx = get_nt_index(p, symbol);
            set_union_minus_epsilon(result, &p->first[idx]);
            if (!set_contains(&p->first[idx], EPSILON)) {
                nullable = false;
                break;
            }
        }
    }
    if (nullable) set_add(result, EPSILON);
}

void compute_first_sets(LL1Parser* p) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < p->rule_count; i++) {
            Rule* r = &p->rules[i];
            int lhs_idx = get_nt_index(p, r->lhs);
            
            StringSet rhs_first;
            get_first_of_sequence(p, r->rhs, r->rhs_count, &rhs_first);

            int initial_count = p->first[lhs_idx].count;
            set_union(&p->first[lhs_idx], &rhs_first);
            
            if (p->first[lhs_idx].count > initial_count) changed = true;
        }
    }
}

void compute_follow_sets(LL1Parser* p) {
    int start_idx = get_nt_index(p, "<au_program>");
    set_add(&p->follow[start_idx], END_MARKER);

    bool changed = true;
    while (changed) {
        changed = false;
        for (int i = 0; i < p->rule_count; i++) {
            Rule* r = &p->rules[i];
            
            for (int j = 0; j < r->rhs_count; j++) {
                char* symbol = r->rhs[j];
                if (!is_non_terminal(p, symbol)) continue;

                int symbol_idx = get_nt_index(p, symbol);
                int initial_count = p->follow[symbol_idx].count;

                // Rest of sequence
                char rest[MAX_RHS][STR_LEN];
                int rest_count = 0;
                for(int k=j+1; k<r->rhs_count; k++) strcpy(rest[rest_count++], r->rhs[k]);

                StringSet first_rest;
                get_first_of_sequence(p, rest, rest_count, &first_rest);

                set_union_minus_epsilon(&p->follow[symbol_idx], &first_rest);

                if (set_contains(&first_rest, EPSILON) || rest_count == 0) {
                    int lhs_idx = get_nt_index(p, r->lhs);
                    set_union(&p->follow[symbol_idx], &p->follow[lhs_idx]);
                }

                if (p->follow[symbol_idx].count > initial_count) changed = true;
            }
        }
    }
}

void build_parsing_table(LL1Parser* p) {
    for (int i = 0; i < p->rule_count; i++) {
        Rule* r = &p->rules[i];
        
        StringSet first_alpha;
        get_first_of_sequence(p, r->rhs, r->rhs_count, &first_alpha);

        for (int j = 0; j < first_alpha.count; j++) {
            char* term = first_alpha.items[j];
            if (strcmp(term, EPSILON) != 0) {
                // Add to table: (LHS, term) = rule
                TableEntry* entry = &p->table[p->table_count++];
                strcpy(entry->lhs, r->lhs);
                strcpy(entry->term, term);
                entry->rule_id = r->id;
            } else {
                int lhs_idx = get_nt_index(p, r->lhs);
                for (int k = 0; k < p->follow[lhs_idx].count; k++) {
                    char* b = p->follow[lhs_idx].items[k];
                    TableEntry* entry = &p->table[p->table_count++];
                    strcpy(entry->lhs, r->lhs);
                    strcpy(entry->term, b);
                    entry->rule_id = r->id;
                }
            }
        }
    }
}

int get_rule_from_table(LL1Parser* p, const char* lhs, const char* term) {
    for (int i = 0; i < p->table_count; i++) {
        if (strcmp(p->table[i].lhs, lhs) == 0 && strcmp(p->table[i].term, term) == 0) {
            return p->table[i].rule_id;
        }
    }
    return -1;
}

// --- Parsing Execution ---

TreeNode* create_node(LL1Parser* p, const char* symbol, int father_index) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    p->node_count++;
    node->index = p->node_count;
    strcpy(node->symbol, symbol);
    node->father = father_index;
    node->sibling = 0;
    
    // Store pointer in array for linear access (printing)
    if(p->node_count < 1000) p->nodes[p->node_count-1] = node;
    
    return node;
}

void load_and_parse(LL1Parser* p, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: File %s not found.\n", filename);
        exit(1);
    }

    // Load Tokens
    typedef struct { char term[STR_LEN]; char val[STR_LEN]; } Token;
    Token tokens[1000];
    int token_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Simple manual parsing to handle PIF format: ClassCode Line ST_ID TokenString
        char code[10], line_num[10], st_id[10], val[50];
        // Note: This simple scan assumes no spaces in token values unless handled carefully.
        // For this mini language, we assume standard spacing.
        int count = sscanf(line, "%s %s %s %s", code, line_num, st_id, val);
        if (count < 4) continue;

        char grammar_term[STR_LEN];
        strcpy(grammar_term, val);

        if (strcmp(code, "I") == 0) strcpy(grammar_term, "identifier");
        else if (strcmp(code, "U") == 0) strcpy(grammar_term, "integer_const");
        else if (strcmp(code, "L") == 0) strcpy(grammar_term, "string_const");
        else if (strcmp(code, "C") == 0) strcpy(grammar_term, "character_const");

        strcpy(tokens[token_count].term, grammar_term);
        strcpy(tokens[token_count].val, val);
        token_count++;
    }
    fclose(file);

    // Add End Marker
    strcpy(tokens[token_count].term, END_MARKER);
    strcpy(tokens[token_count].val, END_MARKER);
    token_count++;

    // Algorithm
    printf("Parsing %d tokens...\n", token_count);
    
    TreeNode* root = create_node(p, "<au_program>", 0);
    
    TreeNode* stack[1000];
    int top = 0;
    stack[top] = root;

    int cursor = 0;

    while (top >= 0) {
        TreeNode* node = stack[top];
        char* symbol = node->symbol;
        char* lookahead_term = tokens[cursor].term;
        char* lookahead_val = tokens[cursor].val;

        if (strcmp(symbol, EPSILON) == 0) {
            top--;
            continue;
        }

        if (is_terminal(p, symbol) || strcmp(symbol, END_MARKER) == 0) {
            if (strcmp(symbol, lookahead_term) == 0) {
                // Match
                strcpy(node->symbol, lookahead_val); // Replace grammar term with actual value
                top--;
                cursor++;
            } else {
                printf("\nSyntax Error at token #%d (Value: '%s').\n", cursor+1, lookahead_val);
                printf("Expected: %s, Found Terminal: %s\n", symbol, lookahead_term);
                exit(1);
            }
        } else {
            // Non-terminal
            int rule_id = get_rule_from_table(p, symbol, lookahead_term);
            if (rule_id == -1) {
                printf("\nSyntax Error at token #%d (Value: '%s').\n", cursor+1, lookahead_val);
                printf("No rule for Non-Terminal '%s' with lookahead '%s'\n", symbol, lookahead_term);
                exit(1);
            }

            top--; // Pop non-terminal
            
            // Get rule RHS
            Rule* r = NULL;
            for(int i=0; i<p->rule_count; i++) if(p->rules[i].id == rule_id) r = &p->rules[i];

            // Create children
            TreeNode* children[MAX_RHS];
            for (int i = 0; i < r->rhs_count; i++) {
                children[i] = create_node(p, r->rhs[i], node->index);
            }

            // Link siblings
            for (int i = 0; i < r->rhs_count - 1; i++) {
                children[i]->sibling = children[i+1]->index;
            }

            // Push reversed to stack
            for (int i = r->rhs_count - 1; i >= 0; i--) {
                stack[++top] = children[i];
            }
        }
    }

    printf("\nParsing Successful!\n");
    print_tree_table(p);
}

// --- Printing ---

void print_tree_table(LL1Parser* p) {
    printf("\nParsing Tree Table\n");
    printf("============================================================\n");
    printf("%-10s %-30s %-10s %-10s\n", "Index", "Symbol", "Father", "Sibling");
    printf("------------------------------------------------------------\n");
    // Sort logic omitted for brevity (array is already roughly ordered by creation)
    for (int i = 0; i < p->node_count; i++) {
        TreeNode* n = p->nodes[i];
        printf("%-10d %-30s %-10d %-10d\n", n->index, n->symbol, n->father, n->sibling);
    }
    printf("============================================================\n");
}


// --- Main ---

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL); 

    if (argc < 2) {
        printf("Usage: ./parser PIF.txt\n");
        return 1;
    }

    printf("Initializing Parser...\n");
    LL1Parser* parser = (LL1Parser*)malloc(sizeof(LL1Parser));
    
    if (parser == NULL) {
        printf("Error: Memory allocation failed. Out of RAM.\n");
        return 1;
    }

    init_parser(parser);
    load_and_parse(parser, argv[1]);

    free(parser);
    
    printf("\nPress Enter to exit...");
    getchar(); 

    return 0;
}
