#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

enum class Lexeme {
    ARRAY,
    BEGIN,
    ELSE,
    END,
    IF,
    OF,
    OR,
    PROGRAM,
    PROCEDURE,
    THEN,
    TYPE,
    VAR,
    MULTIPLICATION,
    PLUS,
    MINUS,
    DIVIDE,
    SEMICOLON,
    COMMA,
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    EQ,
    GREATER,
    LESS,
    LESS_EQ,
    GREATER_EQ,
    NOT_EQ,
    COLON,
    ASSIGN,
    DOT,
    IDENTIFIER,
    STRING,
    INTEGER,
    FLOAT,
    LINE_COMMENT,
    BLOCK_COMMENT,
    BAD,
    END_OF_FILE // Переименован из EOF
};


struct Position {
    int line;
    int column;
};

struct Token {
    Lexeme type;
    std::string lexeme;
    Position position;
};

class Lexer {
private:
    std::string text;
    size_t pos = 0;
    char currentChar = '\0';
    int line = 1;
    int column = 0;

    void advance() {
        if (currentChar == '\n') {
            line++;
            column = 0;
        } else {
            column++;
        }
        pos++;
        currentChar = (pos < text.size()) ? text[pos] : '\0';
    }

    char peek() const {
        return (pos + 1 < text.size()) ? text[pos + 1] : '\0';
    }

    void skipLineComment() {
        while (pos < text.size() && text[pos] != '\n') {
            advance();
        }
        if (pos < text.size()) {
            advance(); // Пропустить '\n'
        }
    }

    bool skipBlockComment() {
        advance();
        while (pos < text.size()) {
            if (text[pos] == '}') {
                advance();
                return true;
            }
            advance();
        }
        return false;
    }


    void skipWhitespace() {
        while (std::isspace(currentChar)) {
            advance();
        }
    }

    Token number() {
        Position start = {line, column};
        std::string result;

        while (std::isdigit(currentChar)) {
            result += currentChar;
            advance();
        }

        if (currentChar == '.') {
            result += currentChar;
            advance();
            while (std::isdigit(currentChar)) {
                result += currentChar;
                advance();
            }
            return {Lexeme::FLOAT, result, start};
        }

        return {Lexeme::INTEGER, result, start};
    }

    Token identifier() {
        Position start = {line, column};
        std::string result;

        while (std::isalnum(currentChar) || currentChar == '_') {
            result += currentChar;
            advance();
        }

        static const std::unordered_map<std::string, Lexeme> keywords = {
                {"ARRAY", Lexeme::ARRAY}, {"BEGIN", Lexeme::BEGIN}, {"ELSE", Lexeme::ELSE},
                {"END", Lexeme::END}, {"IF", Lexeme::IF}, {"OF", Lexeme::OF},
                {"OR", Lexeme::OR}, {"PROGRAM", Lexeme::PROGRAM}, {"PROCEDURE", Lexeme::PROCEDURE},
                {"THEN", Lexeme::THEN}, {"TYPE", Lexeme::TYPE}, {"VAR", Lexeme::VAR}
        };

        auto it = keywords.find(result);
        if (it != keywords.end()) {
            return {it->second, result, start};
        }

        return {Lexeme::IDENTIFIER, result, start};
    }

    Token stringLiteral() {
        Position start = {line, column};
        std::string result;

        advance();
        while (currentChar != '\'' && currentChar != '\n' && currentChar != '\0') {
            result += currentChar;
            advance();
        }

        if (currentChar == '\'') {
            advance();
            return {Lexeme::STRING, result, start};
        }

        return {Lexeme::BAD, result, start};
    }

    Token operatorOrPunctuation() {
        Position start = {line, column};
        char ch = currentChar;
        advance();

        switch (ch) {
            case '*': return {Lexeme::MULTIPLICATION, "*", start};
            case '+': return {Lexeme::PLUS, "+", start};
            case '-': return {Lexeme::MINUS, "-", start};
            case '/': return {Lexeme::DIVIDE, "/", start};
            case ';': return {Lexeme::SEMICOLON, ";", start};
            case ',': return {Lexeme::COMMA, ",", start};
            case '(': return {Lexeme::LEFT_PAREN, "(", start};
            case ')': return {Lexeme::RIGHT_PAREN, ")", start};
            case '[': return {Lexeme::LEFT_BRACKET, "[", start};
            case ']': return {Lexeme::RIGHT_BRACKET, "]", start};
            case '=': return {Lexeme::EQ, "=", start};
            case '>':
                if (currentChar == '=') {
                    advance();
                    return {Lexeme::GREATER_EQ, ">=", start};
                }
                return {Lexeme::GREATER, ">", start};
            case '<':
                if (currentChar == '=') {
                    advance();
                    return {Lexeme::LESS_EQ, "<=", start};
                } else if (currentChar == '>') {
                    advance();
                    return {Lexeme::NOT_EQ, "<>", start};
                }
                return {Lexeme::LESS, "<", start};
            case ':':
                if (currentChar == '=') {
                    advance();
                    return {Lexeme::ASSIGN, ":=", start};
                }
                return {Lexeme::COLON, ":", start};
            case '.': return {Lexeme::DOT, ".", start};
            default: return {Lexeme::BAD, std::string(1, ch), start};
        }
    }

public:
    explicit Lexer(const std::string& input) : text(input) {
        currentChar = !text.empty() ? text[0] : '\0';
    }

    Token nextToken() {
        while (currentChar != '\0') {
            if (std::isspace(currentChar)) {
                skipWhitespace();
                continue;
            }

            if (std::isdigit(currentChar)) {
                return number();
            }

            if (std::isalpha(currentChar) || currentChar == '_') {
                return identifier();
            }

            if (currentChar == '\'') {
                return stringLiteral();
            }

            if (currentChar == '/' && peek() == '/') {
                skipLineComment();
                return nextToken();
            }

            // Пропуск многострочных комментариев
            if (currentChar == '{') {
                if (skipBlockComment()) {
                    return nextToken();
                }
                return {Lexeme::BAD, "{", {line, column}};
            }

            return operatorOrPunctuation();
        }

        return {Lexeme::END_OF_FILE, "", {line, column}};
    }

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        Token token;
        do {
            token = nextToken();
            tokens.push_back(token);
        } while (token.type != Lexeme::END_OF_FILE);
        return tokens;
    }
};

std::string lexemeToString(Lexeme type) {
    switch (type) {
        case Lexeme::BAD: return "BAD";
        case Lexeme::IDENTIFIER: return "IDENTIFIER";
        case Lexeme::STRING: return "STRING";
        case Lexeme::INTEGER: return "INTEGER";
        case Lexeme::FLOAT: return "FLOAT";
        case Lexeme::LINE_COMMENT: return "LINE_COMMENT";
        case Lexeme::BLOCK_COMMENT: return "BLOCK_COMMENT";
        case Lexeme::ARRAY: return "ARRAY";
        case Lexeme::BEGIN: return "BEGIN";
        case Lexeme::ELSE: return "ELSE";
        case Lexeme::END: return "END";
        case Lexeme::IF: return "IF";
        case Lexeme::OF: return "OF";
        case Lexeme::OR: return "OR";
        case Lexeme::PROGRAM: return "PROGRAM";
        case Lexeme::PROCEDURE: return "PROCEDURE";
        case Lexeme::THEN: return "THEN";
        case Lexeme::TYPE: return "TYPE";
        case Lexeme::VAR: return "VAR";
        case Lexeme::MULTIPLICATION: return "MULTIPLICATION";
        case Lexeme::PLUS: return "PLUS";
        case Lexeme::MINUS: return "MINUS";
        case Lexeme::DIVIDE: return "DIVIDE";
        case Lexeme::SEMICOLON: return "SEMICOLON";
        case Lexeme::COMMA: return "COMMA";
        case Lexeme::LEFT_PAREN: return "LEFT_PAREN";
        case Lexeme::RIGHT_PAREN: return "RIGHT_PAREN";
        case Lexeme::LEFT_BRACKET: return "LEFT_BRACKET";
        case Lexeme::RIGHT_BRACKET: return "RIGHT_BRACKET";
        case Lexeme::EQ: return "EQ";
        case Lexeme::GREATER: return "GREATER";
        case Lexeme::LESS: return "LESS";
        case Lexeme::LESS_EQ: return "LESS_EQ";
        case Lexeme::GREATER_EQ: return "GREATER_EQ";
        case Lexeme::NOT_EQ: return "NOT_EQ";
        case Lexeme::COLON: return "COLON";
        case Lexeme::ASSIGN: return "ASSIGN";
        case Lexeme::DOT: return "DOT";
        case Lexeme::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

void printTokens(const std::vector<Token>& tokens) {
    for (const auto& token : tokens) {
        std::cout <<  lexemeToString(token.type) << " (" << token.position.line << ", " << token.position.column << ") \""<< token.lexeme << "\"" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: lexer <input_file>" << std::endl;
        return 1;
    }

    std::ifstream inputFile(argv[1]);
    if (!inputFile) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
    Lexer lexer(text);
    auto tokens = lexer.tokenize();
    printTokens(tokens);

    return 0;
}
