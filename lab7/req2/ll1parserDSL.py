import sys

class TreeNode:
    def __init__(self, index, symbol, father_index=0):
        self.index = index
        self.symbol = symbol
        self.father = father_index
        self.sibling = 0

class LL1Parser:
    def __init__(self):
        self.epsilon = 'ε'
        self.start_symbol = '<au_program>'
        self.end_marker = '$'

        self.terminals = {
            'AU', 'Language', 'int', 'char', 'string', 'struct', 
            'if', 'else', 'while', 'read', 'write',
            'identifier', 'integer_const', 'string_const', 'character_const',
            ';', ',', '[', ']', '{', '}', '(', ')', '.',
            '=', '+', '-', '*', '/', '%', 
            '==', '<>', '<', '>', '<=', '>=', '#'
        }

        self.non_terminals = {
            '<au_program>', '<declaration_list>', '<declaration>', 
            '<declaration_tail>', '<type_decl_tail>', '<list_decl_tail>',
            '<statement_list>', '<statement>', '<assignment_stmt>', '<assignment_tail>',
            '<conditional_stmt>', '<else_part>', '<while_stmt>', '<io_stmt>', 
            '<io_tail>', '<io_struct_suffix>', '<expression>', '<arith_expr>', '<arith_tail>', 
            '<term>', '<term_tail>', '<factor>', '<factor_tail>',
            '<add_operator>', '<mul_operator>', '<condition>', '<relational_operator>',
            '<id_list>', '<id_list_tail>', '<struct_tail>'
        }

        # Rule Format: ID: (LHS, [RHS list])
        self.rules = {
            1:  ('<au_program>', ['AU', 'Language', '<declaration_list>', '<statement_list>']),
            
            # Declaration List
            2:  ('<declaration_list>', ['<declaration>', ';', '<declaration_list>']),
            3:  ('<declaration_list>', [self.epsilon]), 

            # Declaration
            4:  ('<declaration>', ['int', '<type_decl_tail>']),
            5:  ('<declaration>', ['char', '<type_decl_tail>']),
            6:  ('<declaration>', ['string', '<type_decl_tail>']),
            7:  ('<declaration>', ['struct', 'identifier', '{', '<declaration_list>', '}']),
            
            8:  ('<type_decl_tail>', ['<id_list>']), 
            9:  ('<type_decl_tail>', ['[', 'integer_const', ']', 'identifier']),

            # Statement List
            10: ('<statement_list>', ['<statement>', ';', '<statement_list>']),
            11: ('<statement_list>', [self.epsilon]),

            # Statements
            12: ('<statement>', ['<assignment_stmt>']),
            13: ('<statement>', ['<conditional_stmt>']),
            14: ('<statement>', ['<while_stmt>']),
            15: ('<statement>', ['<io_stmt>']),

            # Assignment
            16: ('<assignment_stmt>', ['identifier', '<assignment_tail>']),
            17: ('<assignment_tail>', ['=', '<expression>']),
            18: ('<assignment_tail>', ['[', '<expression>', ']', '<factor_tail>', '=', '<expression>']),
            19: ('<assignment_tail>', ['.', 'identifier', '=', '<expression>']),

            # Conditional
            20: ('<conditional_stmt>', ['if', '(', '<condition>', ')', '{', '<statement_list>', '}', '<else_part>']),
            21: ('<else_part>', ['else', '{', '<statement_list>', '}']),
            22: ('<else_part>', [self.epsilon]),

            # While
            23: ('<while_stmt>', ['while', '(', '<condition>', ')', '{', '<statement_list>', '}']),

            # IO
            24: ('<io_stmt>', ['read', '(', 'identifier', '<io_tail>', ')']),
            25: ('<io_stmt>', ['write', '(', '<expression>', ')']),
            26: ('<io_tail>', [self.epsilon]),
            27: ('<io_tail>', ['[', '<expression>', ']', '<io_struct_suffix>']),
            
            # Expressions
            28: ('<expression>', ['<term>', '<arith_tail>']),
            29: ('<arith_tail>', ['<add_operator>', '<term>', '<arith_tail>']),
            30: ('<arith_tail>', [self.epsilon]),

            31: ('<term>', ['<factor>', '<term_tail>']),
            32: ('<term_tail>', ['<mul_operator>', '<factor>', '<term_tail>']),
            33: ('<term_tail>', [self.epsilon]),

            # Factor
            34: ('<factor>', ['identifier', '<factor_tail>']),
            35: ('<factor>', ['integer_const']),
            36: ('<factor>', ['(', '<expression>', ')']),
            37: ('<factor_tail>', [self.epsilon]),
            38: ('<factor_tail>', ['[', '<expression>', ']', '<struct_tail>']),
            39: ('<factor_tail>', ['.', 'identifier']),
            
            # ID List
            40: ('<id_list>', ['identifier', '<id_list_tail>']),
            41: ('<id_list_tail>', [',', '<id_list>']),
            42: ('<id_list_tail>', [self.epsilon]),

            # Operators
            43: ('<add_operator>', ['+']), 44: ('<add_operator>', ['-']),
            45: ('<mul_operator>', ['*']), 46: ('<mul_operator>', ['/']),
            47: ('<condition>', ['<expression>', '<relational_operator>', '<expression>']),
            48: ('<relational_operator>', ['==']), 49: ('<relational_operator>', ['<']),
            50: ('<relational_operator>', ['>']),
            
            51: ('<factor>', ['string_const']),

            # Misc Rules
            52: ('<declaration>', ['identifier', '<type_decl_tail>']),
            53: ('<io_tail>', ['.', 'identifier']),
            54: ('<io_struct_suffix>', ['.', 'identifier']),
            55: ('<io_struct_suffix>', [self.epsilon]),
            
            56: ('<struct_tail>', ['.', 'identifier']),
            57: ('<struct_tail>', [self.epsilon])
        }

        # Data Structures
        self.first = {nt: set() for nt in self.non_terminals}
        self.follow = {nt: set() for nt in self.non_terminals}
        self.parsing_table = {} 
        self.nodes = []
        self.node_counter = 0

        # Run construction
        self.compute_first_sets()
        self.compute_follow_sets()
        self.build_parsing_table()

    
    def _is_terminal(self, symbol):
        return symbol in self.terminals

    def _get_first_of_sequence(self, sequence):
        result = set()
        nullable = True
        for symbol in sequence:
            if symbol == self.epsilon: continue
            
            if self._is_terminal(symbol):
                result.add(symbol)
                nullable = False
                break
            elif symbol in self.non_terminals:
                result.update(self.first[symbol] - {self.epsilon})
                if self.epsilon not in self.first[symbol]:
                    nullable = False
                    break
        if nullable: result.add(self.epsilon)
        return result

    def compute_first_sets(self):
        changed = True
        while changed:
            changed = False
            for _, (lhs, rhs) in self.rules.items():
                rhs_first = self._get_first_of_sequence(rhs)
                before = len(self.first[lhs])
                self.first[lhs].update(rhs_first)
                if len(self.first[lhs]) > before: changed = True

    def compute_follow_sets(self):
        self.follow[self.start_symbol].add(self.end_marker)
        changed = True
        while changed:
            changed = False
            for _, (lhs, rhs) in self.rules.items():
                for i, symbol in enumerate(rhs):
                    if symbol in self.non_terminals:
                        rest = rhs[i+1:]
                        first_rest = self._get_first_of_sequence(rest)
                        before = len(self.follow[symbol])
                        self.follow[symbol].update(first_rest - {self.epsilon})
                        if self.epsilon in first_rest or not rest:
                            self.follow[symbol].update(self.follow[lhs])
                        if len(self.follow[symbol]) > before: changed = True

    def build_parsing_table(self):
        for rule_id, (lhs, rhs) in self.rules.items():
            first_alpha = self._get_first_of_sequence(rhs)
            for terminal in first_alpha:
                if terminal != self.epsilon:
                    key = (lhs, terminal)
                    if key not in self.parsing_table:
                        self.parsing_table[key] = rule_id
            if self.epsilon in first_alpha:
                for b in self.follow[lhs]:
                    key = (lhs, b)
                    if key not in self.parsing_table:
                        self.parsing_table[key] = rule_id
        

    def create_node(self, symbol, father_index=0):
        self.node_counter += 1
        node = TreeNode(self.node_counter, symbol, father_index)
        self.nodes.append(node)
        return node

    def load_pif(self, filename):
        """
        Reads a PIF file. 
        Expected format: ClassCode Line ST_ID TokenString
        Example: I 3 0 Student
        """
        tokens = []
        try:
            with open(filename, 'r') as f:
                for line in f:
                    parts = line.strip().split(' ', 3)
                    if len(parts) < 4: continue
                    
                    code = parts[0]
                    token_str = parts[3]
                    
                    grammar_terminal = token_str
                    
                    if code == 'I':
                        grammar_terminal = 'identifier'
                    elif code == 'U':
                        grammar_terminal = 'integer_const'
                    elif code == 'L':
                        grammar_terminal = 'string_const'
                    elif code == 'C':
                        grammar_terminal = 'character_const'
                    
                    tokens.append((grammar_terminal, token_str))
                    
        except FileNotFoundError:
            print(f"Error: File {filename} not found.")
            sys.exit(1)
            
        tokens.append((self.end_marker, self.end_marker))
        return tokens

    def parse(self, pif_filename):
        tokens = self.load_pif(pif_filename)
        self.nodes = []
        self.node_counter = 0
        
        root = self.create_node(self.start_symbol, 0)
        stack = [root]
        cursor = 0

        print(f"Parsing {len(tokens)} tokens...")
        
        while stack:
            node = stack[-1]
            symbol = node.symbol
            
            lookahead_term, lookahead_val = tokens[cursor]

            if symbol == self.epsilon:
                stack.pop()
                continue

            if symbol in self.terminals or symbol == self.end_marker:
                if symbol == lookahead_term:
                    node.symbol = lookahead_val
                    stack.pop()
                    cursor += 1
                else:
                    self.error(symbol, lookahead_term, cursor, lookahead_val)
            
            elif symbol in self.non_terminals:
                rule_id = self.parsing_table.get((symbol, lookahead_term))
                if rule_id is None:
                    self.error(symbol, lookahead_term, cursor, lookahead_val)
                
                stack.pop()
                _, rhs = self.rules[rule_id]
                
                children = []
                for s in rhs:
                    children.append(self.create_node(s, node.index))
                
                for i in range(len(children)-1):
                    children[i].sibling = children[i+1].index
                
                for child in reversed(children):
                    stack.append(child)
            else:
                print(f"Unknown Symbol Error: {symbol}")
                return

        print("\nParsing Successful!")
        self.print_tree_table()
        self.print_visual_tree()

    def error(self, expected, found_term, cursor, found_val):
        print(f"\nSyntax Error at token #{cursor+1} (Value: '{found_val}').")
        print(f"Expected: {expected}, Found Terminal: {found_term}")
        sys.exit(1)

    def print_tree_table(self):
        print("\nParsing Tree Table")
        print("=" * 60)
        print(f"{'Index':<10} {'Symbol':<30} {'Father':<10} {'Sibling':<10}")
        print("-" * 60)
        for node in sorted(self.nodes, key=lambda x: x.index):
            print(f"{node.index:<10} {node.symbol:<30} {node.father:<10} {node.sibling:<10}")
        print("=" * 60)

    def print_visual_tree(self):
        print("\nVisual Parsing Tree:")
        if not self.nodes: return
        root = self.nodes[0]
        self._print_node_recursive(root, "", True)

    def _print_node_recursive(self, node, prefix, is_last):
        connector = "└── " if is_last else "├── "
        print(f"{prefix}{connector}{node.symbol} ({node.index})")
        child_prefix = prefix + ("    " if is_last else "│   ")
        children = sorted([n for n in self.nodes if n.father == node.index], key=lambda x: x.index)
        for i, child in enumerate(children):
            self._print_node_recursive(child, child_prefix, (i == len(children) - 1))

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python mini_lang_parser.py <pif_file>")
        sys.exit(1)

    pif_filename = sys.argv[1]
    parser = LL1Parser()
    parser.parse(pif_filename)

