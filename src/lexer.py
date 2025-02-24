import re

# PL/0关键字和操作符
KEYWORDS = {'begin', 'end', 'if', 'then', 'while', 'do', 'write', 'read'}
OPERATORS = {'+', '-', '*', '/', ':=', '=', '<>', '<', '>', ':', ';', ',', '(', ')'}
DELIMITERS = {'(', ')', ',', ';'}

# Token类型定义
class Token:
    def __init__(self, type_, value):
        self.type = type_
        self.value = value

    def __repr__(self):
        return f"Token({self.type}, {repr(self.value)})"

# 词法分析器
class Lexer:
    def __init__(self, code):
        self.code = code
        self.position = 0

    def get_char(self):
        if self.position < len(self.code):
            return self.code[self.position]
        return None

    def consume(self):
        self.position += 1

    def peek(self):
        if self.position < len(self.code):
            return self.code[self.position]
        return None

    def is_digit(self, char):
        return char.isdigit()

    def is_alpha(self, char):
        return char.isalpha()

    def skip_whitespace(self):
        while self.get_char() is not None and self.get_char().isspace():
            self.consume()

    def next_token(self):
        self.skip_whitespace()
        char = self.get_char()

        if char is None:
            return None  # End of input

        # Handle numbers
        if self.is_digit(char):
            value = ''
            while self.is_digit(self.get_char()):
                value += self.get_char()
                self.consume()
            return Token('NUMBER', value)

        # Handle identifiers and keywords
        if self.is_alpha(char):
            value = ''
            while self.is_alpha(self.get_char()) or self.is_digit(self.get_char()):
                value += self.get_char()
                self.consume()
            if value in KEYWORDS:
                return Token('KEYWORD', value)
            return Token('IDENTIFIER', value)

        # Handle operators and delimiters
        if char in OPERATORS:
            value = char
            self.consume()
            return Token('OPERATOR', value)

        if char in DELIMITERS:
            value = char
            self.consume()
            return Token('DELIMITER', value)

        # If we reach here, something is wrong
        raise ValueError(f"Unexpected character: {char}")

    def tokenize(self):
        tokens = []
        while (token := self.next_token()) is not None:
            tokens.append(token)
        return tokens
