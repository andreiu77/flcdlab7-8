class LL1Parser:
    def __init__(self):
        # 1. Grammar Definition
        self.terminals = {'+', '*', 'a', '(', ')'}
        self.non_terminals = {'S', 'A', 'B', 'C', 'D'}
        self.epsilon = 'ε'
        self.start_symbol = 'S'
        self.end_marker = '$'

        # Rules with IDs as specified in the assignment
        self.rules = {
            1: ('S', ['B', 'A']),
            2: ('A', ['+', 'B', 'A']),
            3: ('A', [self.epsilon]),
            4: ('B', ['D', 'C']),
            5: ('C', ['*', 'D', 'C']),
            6: ('C', [self.epsilon]),
            7: ('D', ['(', 'S', ')']),
            8: ('D', ['a'])
        }

        # 2. Data Structures for Analysis
        self.first = {nt: set() for nt in self.non_terminals}
        self.follow = {nt: set() for nt in self.non_terminals}
        self.parsing_table = {} # Key: (Non-Terminal, Terminal), Value: Rule ID

        # 3. Build the Parser
        print(">>> Computing FIRST sets...")
        self.compute_first_sets()
        self._print_sets(self.first, "FIRST")

        print("\n>>> Computing FOLLOW sets...")
        self.compute_follow_sets()
        self._print_sets(self.follow, "FOLLOW")

        print("\n>>> Building Parsing Table...")
        self.build_parsing_table()
        self._print_table()

    def _is_terminal(self, symbol):
        return symbol in self.terminals

    def compute_first_sets(self):
        """Iteratively computes FIRST sets for all non-terminals."""
        changed = True
        while changed:
            changed = False
            for _, (lhs, rhs) in self.rules.items():
                # Calculate FIRST(RHS)
                rhs_first = self._get_first_of_sequence(rhs)
                
                # Update FIRST(LHS)
                before_len = len(self.first[lhs])
                self.first[lhs].update(rhs_first)
                if len(self.first[lhs]) > before_len:
                    changed = True

    def _get_first_of_sequence(self, sequence):
        """Helper: returns FIRST set for a sequence of symbols."""
        result = set()
        nullable = True
        
        for symbol in sequence:
            if symbol == self.epsilon:
                continue
            
            if self._is_terminal(symbol):
                result.add(symbol)
                nullable = False
                break
            elif symbol in self.non_terminals:
                # Add FIRST(NonTerminal) - {ε}
                result.update(self.first[symbol] - {self.epsilon})
                # If this NonTerminal cannot produce ε, stop
                if self.epsilon not in self.first[symbol]:
                    nullable = False
                    break
            else:
                # Should not happen in this specific grammar context
                nullable = False
                break
        
        if nullable:
            result.add(self.epsilon)
        return result

    def compute_follow_sets(self):
        """Iteratively computes FOLLOW sets."""
        self.follow[self.start_symbol].add(self.end_marker)
        
        changed = True
        while changed:
            changed = False
            for _, (lhs, rhs) in self.rules.items():
                # Loop through the RHS to find non-terminals
                for i, symbol in enumerate(rhs):
                    if symbol in self.non_terminals:
                        # CASE 1: FOLLOW(symbol) gets FIRST(rest_of_rhs) - {ε}
                        rest_of_rhs = rhs[i+1:]
                        first_of_rest = self._get_first_of_sequence(rest_of_rhs)
                        
                        before_len = len(self.follow[symbol])
                        self.follow[symbol].update(first_of_rest - {self.epsilon})
                        
                        # CASE 2: If rest_of_rhs is nullable (or empty), add FOLLOW(LHS)
                        if self.epsilon in first_of_rest or not rest_of_rhs:
                            self.follow[symbol].update(self.follow[lhs])
                            
                        if len(self.follow[symbol]) > before_len:
                            changed = True

    def build_parsing_table(self):
        """Constructs the LL(1) parsing table using FIRST and FOLLOW sets."""
        for rule_id, (lhs, rhs) in self.rules.items():
            first_alpha = self._get_first_of_sequence(rhs)
            
            # Rule 1: For each terminal 'a' in FIRST(alpha), add A->alpha to M[A, a]
            for terminal in first_alpha:
                if terminal != self.epsilon:
                    self._add_to_table(lhs, terminal, rule_id)
            
            # Rule 2: If ε is in FIRST(alpha), add A->alpha to M[A, b] for each b in FOLLOW(A)
            if self.epsilon in first_alpha:
                for b in self.follow[lhs]:
                    self._add_to_table(lhs, b, rule_id)

    def _add_to_table(self, non_terminal, terminal, rule_id):
        key = (non_terminal, terminal)
        if key in self.parsing_table:
            if self.parsing_table[key] != rule_id:
                raise ValueError(f"Grammar is not LL(1)! Conflict at M[{non_terminal}, {terminal}]")
        self.parsing_table[key] = rule_id

    def parse(self, input_string):
        # 1. Preprocess input
        tokens = [char for char in input_string if char != ' ']
        tokens.append(self.end_marker)

        # 2. Initialize Stack
        stack = [self.end_marker, self.start_symbol]
        production_sequence = []
        cursor = 0

        print(f"\n{'STACK':<20} {'INPUT':<20} {'ACTION'}")
        print("-" * 60)

        while stack:
            top = stack[-1]
            current_token = tokens[cursor]
            
            stack_str = "".join(stack)
            input_str = "".join(tokens[cursor:])

            if top == current_token:
                # Match Terminal
                print(f"{stack_str:<20} {input_str:<20} Match '{current_token}'")
                stack.pop()
                cursor += 1
                if top == self.end_marker:
                    break
            
            elif top in self.terminals:
                raise ValueError(f"Syntax Error: Expected '{top}', found '{current_token}'")
                
            elif top in self.non_terminals:
                # Look up table
                rule_id = self.parsing_table.get((top, current_token))
                
                if rule_id is None:
                    raise ValueError(f"Syntax Error: No rule for [{top}, {current_token}]")
                
                lhs, rhs = self.rules[rule_id]
                production_sequence.append(rule_id)
                
                # Format RHS for display
                rhs_display = "".join(rhs)
                print(f"{stack_str:<20} {input_str:<20} Output Rule {rule_id} ({lhs}->{rhs_display})")
                
                stack.pop()
                
                # Push RHS in reverse order (exclude epsilon)
                if rhs != [self.epsilon]:
                    for symbol in reversed(rhs):
                        stack.append(symbol)
            else:
                raise ValueError("Unknown Error")

        return production_sequence

    def _print_sets(self, sets, name):
        for k, v in sorted(sets.items()):
            # Sort for deterministic output, filter out Epsilon for cleaner look if desired
            formatted = ", ".join(sorted(list(v)))
            print(f"{name}({k}) = {{ {formatted} }}")

    def _print_table(self):
        print(f"{'NT':<5} {'Term':<5} {'Rule'}")
        print("-" * 20)
        for (nt, term), rule in sorted(self.parsing_table.items()):
            print(f"{nt:<5} {term:<5} {rule}")


if __name__ == "__main__":
    parser = LL1Parser()
    input_seq = "a + a"
    
    print(f"\nParsing Sequence: {input_seq}")
    try:
        result = parser.parse(input_seq)
        print("\nParsing Successful!")
        print("Sequence:", "".join(str(r) for r in result))
    except ValueError as e:
        print(f"\nParsing Failed: {e}")