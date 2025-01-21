import sys
import re
from enum import Enum

class Lexeme(Enum):
    ARRAY = "ARRAY"
    BEGIN = "BEGIN"
    ELSE = "ELSE"
    END = "END"
    IF = "IF"
    OF = "OF"
    OR = "OR"
    PROGRAM = "PROGRAM"
    PROCEDURE = "PROCEDURE"
    THEN = "THEN"
    TYPE = "TYPE"
    VAR = "VAR"
    MULTIPLICATION = "MULTIPLICATION"
    PLUS = "PLUS"
    MINUS = "MINUS"
    DIVIDE = "DIVIDE"
    SEMICOLON = "SEMICOLON"
    COMMA = "COMMA"
    LEFT_PAREN = "LEFT_PAREN"
    RIGHT_PAREN = "RIGHT_PAREN"
    LEFT_BRACKET = "LEFT_BRACKET"
    RIGHT_BRACKET = "RIGHT_BRACKET"
    EQ = "EQ"
    GREATER = "GREATER"
    LESS = "LESS"
    LESS_EQ = "LESS_EQ"
    GREATER_EQ = "GREATER_EQ"
    NOT_EQ = "NOT_EQ"
    COLON = "COLON"
    ASSIGN = "ASSIGN"
    DOT = "DOT"
    IDENTIFIER = "IDENTIFIER"
    STRING = "STRING"
    INTEGER = "INTEGER"
    FLOAT = "FLOAT"
    LINE_COMMENT = "LINE_COMMENT"
    BLOCK_COMMENT = "BLOCK_COMMENT"
    BAD = "BAD"

class Position:
    def __init__(self, line, column):
        self.line = line
        self.column = column

class Token:
    def __init__(self, type, lexeme, position):
        self.type = type
        self.lexeme = lexeme
        self.position = position

class Lexer:
    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.current_char = text[0] if text else ""
        self.line = 1
        self.column = 1

    def advance(self):
        if self.current_char == "\n":
            self.line += 1
            self.column = 1
        else:
            self.column += 1

        self.pos += 1
        self.current_char = self.text[self.pos] if self.pos < len(self.text) else ""

    def peek(self):
        return self.text[self.pos + 1] if self.pos + 1 < len(self.text) else ""

    def skip_whitespace(self):
        while self.current_char.isspace():
            self.advance()

    def skip_line_comment(self):
        while self.current_char and self.current_char != "\n":
            self.advance()
        self.advance()

    def skip_block_comment(self):
        start_position = Position(self.line, self.column)
        self.advance()
        while self.current_char:
            if self.current_char == "}" and self.peek() == "":
                self.advance()
                return None
            elif self.current_char == "}" and self.peek() != "":
                self.advance()
                break
            self.advance()
        else:
            return Token(Lexeme.BAD, "Unclosed block comment", start_position)
        return None

    def number(self):
        start = Position(self.line, self.column)
        result = ""
        is_float = False
        is_invalid = False

        while self.current_char.isdigit():
            result += self.current_char
            self.advance()

        if self.current_char == ".":
            is_float = True
            result += self.current_char
            self.advance()

            if not self.current_char.isdigit():
                is_invalid = True

            while self.current_char.isdigit():
                result += self.current_char
                self.advance()

            if self.current_char == ".":
                is_invalid = True
                while self.current_char and not self.current_char.isspace() and self.current_char not in ";,:)]}":
                    result += self.current_char
                    self.advance()

        if self.current_char in "eE" and not is_invalid:
            is_float = True
            result += self.current_char
            self.advance()

            if self.current_char in "+-":
                result += self.current_char
                self.advance()

            if not self.current_char.isdigit():
                is_invalid = True

            while self.current_char.isdigit():
                result += self.current_char
                self.advance()

        if is_invalid:
            while not self.current_char.isspace() and self.current_char:
                result += self.current_char
                self.advance()
            return Token(Lexeme.BAD, result, start)

        return Token(Lexeme.FLOAT if is_float else Lexeme.INTEGER, result, start)

    def identifier(self):
        start = Position(self.line, self.column)
        result = ""

        while self.current_char.isalnum() or self.current_char == "_":
            result += self.current_char
            self.advance()

        keywords = {
            "ARRAY": Lexeme.ARRAY, "BEGIN": Lexeme.BEGIN, "ELSE": Lexeme.ELSE,
            "END": Lexeme.END, "IF": Lexeme.IF, "OF": Lexeme.OF,
            "OR": Lexeme.OR, "PROGRAM": Lexeme.PROGRAM, "PROCEDURE": Lexeme.PROCEDURE,
            "THEN": Lexeme.THEN, "TYPE": Lexeme.TYPE, "VAR": Lexeme.VAR
        }

        if len(result) > 255:  # Проверка длины строки
            return Token(Lexeme.BAD, result, start)

        return Token(keywords.get(result, Lexeme.IDENTIFIER), result, start)

    def string_literal(self):
        start = Position(self.line, self.column)
        result = ""
        result += self.current_char
        self.advance()

        while self.current_char != "'" and self.current_char != "\n" and self.current_char:
            result += self.current_char
            self.advance()

        if self.current_char == "'":
            result += self.current_char
            self.advance()
            return Token(Lexeme.STRING, result, start)

        return Token(Lexeme.BAD, result, start)

    def operator_or_punctuation(self):
        start = Position(self.line, self.column)
        ch = self.current_char
        self.advance()

        operators = {
           '*': Lexeme.MULTIPLICATION, '+': Lexeme.PLUS, '-': Lexeme.MINUS,
          '/': Lexeme.DIVIDE, ';': Lexeme.SEMICOLON, ',': Lexeme.COMMA,
          '(': Lexeme.LEFT_PAREN, ')': Lexeme.RIGHT_PAREN,
          '[': Lexeme.LEFT_BRACKET, ']': Lexeme.RIGHT_BRACKET,
          '=': Lexeme.EQ, '.': Lexeme.DOT,
        }

        if ch in operators:
            return Token(operators[ch], ch, start)

        if ch == '>':
             if self.current_char == '=':
                 self.advance()
                 return Token(Lexeme.GREATER_EQ, ">=", start)

             return Token(Lexeme.GREATER, ">", start)
        if ch == '<':
            if self.current_char == "=":
                self.advance()
                return Token(Lexeme.LESS_EQ, "<=", start)
            elif self.current_char == ">":
                self.advance()
                return Token(Lexeme.NOT_EQ, "<>", start)
            return Token(Lexeme.LESS, "<", start)

        if ch == ':':
            if self.current_char == "=":
                self.advance()
                return Token(Lexeme.ASSIGN, ":=", start)
            return Token(Lexeme.COLON, ":", start)

        return Token(Lexeme.BAD, ch, start)

    def next_token(self):
        while self.current_char:
            if self.current_char.isspace():
                self.skip_whitespace()
                continue

            if self.current_char.isdigit():
                return self.number()

            if self.current_char.isalpha() or self.current_char == "_":
                return self.identifier()

            if self.current_char == "'":
                return self.string_literal()

            if self.current_char == '/' and self.peek() == '/':
                self.skip_line_comment()
                continue

            if self.current_char == '{':
                comment_result = self.skip_block_comment()
                if isinstance(comment_result, Token):
                    return comment_result
                continue

            return self.operator_or_punctuation()

        return None

    def tokenize(self):
        tokens = []
        while True:
            token = self.next_token()
            if not token:
                break
            tokens.append(token)
        return tokens

def lexeme_to_string(lexeme):
    return lexeme.value

def print_token_to_file(tokens, output_file):
    with open(output_file, "w") as out:
        for token in tokens:
            out.write(f"{lexeme_to_string(token.type)} ({token.position.line}, {token.position.column}) \"{token.lexeme}\"\n")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: lexer <input_file> <output_file>")
        sys.exit(1)

    try:
        with open(sys.argv[1], "r") as input_file:
            text = input_file.read()
    except FileNotFoundError:
        print(f"Error: Cannot open file {sys.argv[1]}")
        sys.exit(1)

    lexer = Lexer(text)
    tokens = lexer.tokenize()

    try:
        print_token_to_file(tokens, sys.argv[2])
    except Exception as e:
        print(f"Error writing to file {sys.argv[2]}: {e}")
        sys.exit(1)
