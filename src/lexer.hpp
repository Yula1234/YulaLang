#pragma once

#include<string>
#include<vector>
#include<cassert>
#include<optional>

enum class TokenType {
    int_lit,
    ident,
    plus,
    star,
    minus,
    fslash,
    print,
    _if,
    end,
    exit,
    _else,
    eq,
    dup,
    left_arrow,
    right_arrow,
    ddo,
    wwhile,
    drop,
    lnot,
    lor,
    land,
    ttrue,
    ffalse,
};

inline std::string to_string(const TokenType type)
{
    switch (type) {
    case TokenType::int_lit:
        return "int literal";
    case TokenType::ident:
        return "identifier";
    case TokenType::plus:
        return "`+`";
    case TokenType::star:
        return "`*`";
    case TokenType::minus:
        return "`-`";
    case TokenType::fslash:
        return "`/`";
    case TokenType::print:
        return "`print`";
    case TokenType::_if:
        return "`if`";
    case TokenType::end:
        return "`end`";
    case TokenType::exit:
        return "`exit`";
    case TokenType::_else:
        return "`else`";
    case TokenType::eq:
        return "`=`";
    case TokenType::dup:
        return "`dup`";
    case TokenType::left_arrow:
        return "`<`";
    case TokenType::right_arrow:
        return "`>`";
    case TokenType::ddo:
        return "`do`";
    case TokenType::wwhile:
        return "`while`";
    case TokenType::drop:
        return "`drop`";
    case TokenType::lor:
        return "`or`";
    case TokenType::land:
        return "`and`";
    case TokenType::lnot:
        return "`not`";
    case TokenType::ffalse:
        return "`false`";
    case TokenType::ttrue:
        return "`true`";
    }
    assert(false);
}

struct Token {
    TokenType type;
    int line;
    int col;
    std::optional<std::string> value {};
    friend std::ostream& operator<<(std::ostream& out, const Token& tok) {
        out << "Token(.type = " << to_string(tok.type);
        out << ", .line = " << tok.line;
        out << ", .col = " << tok.col;
        if(tok.value.has_value()) {
            out << ", .value = " << tok.value.value();
        }
        out << ")";
        return out;
    }
};

#define token_list std::vector<Token>

#define START_COL 1

class Lexer {
private:
    int m_col = START_COL;
public:
    explicit Lexer(std::string src)
        : m_src(std::move(src))
    {
    }

    std::vector<Token> lex()
    {
        std::vector<Token> tokens;
        std::string buf;
        int line_count = 1;
        while (peek().has_value()) {
            if (std::isalpha(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value())) {
                    buf.push_back(consume());
                }
                if(buf == "print") {
                    tokens.push_back({ .type = TokenType::print, .line = line_count, .col = m_col - (int)buf.size()});
                    buf.clear();
                }
                else if(buf == "if") {
                    tokens.push_back({ .type = TokenType::_if, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "else") {
                    tokens.push_back({ .type = TokenType::_else, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "dup") {
                    tokens.push_back({ .type = TokenType::dup, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "drop") {
                    tokens.push_back({ .type = TokenType::drop, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "exit") {
                    tokens.push_back({ .type = TokenType::exit, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "end") {
                    tokens.push_back({ .type = TokenType::end, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "do") {
                    tokens.push_back({ .type = TokenType::ddo, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "while") {
                    tokens.push_back({ .type = TokenType::wwhile, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "not") {
                    tokens.push_back({ .type = TokenType::lnot, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "or") {
                    tokens.push_back({ .type = TokenType::lor, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "and") {
                    tokens.push_back({ .type = TokenType::land, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "true") {
                    tokens.push_back({ .type = TokenType::ttrue, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "false") {
                    tokens.push_back({ .type = TokenType::ffalse, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else {
                    tokens.push_back({ .type = TokenType::ident, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
            }
            else if (std::isdigit(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isdigit(peek().value())) {
                    buf.push_back(consume());
                }
                tokens.push_back({ .type = TokenType::int_lit, .line = line_count, .col = m_col - (int)buf.size() , .value = buf });
                buf.clear();
            }
            else if (peek().value() == '/' && peek(1).has_value() && peek(1).value() == '/') {
                consume();
                consume();
                while (peek().has_value() && peek().value() != '\n') {
                    consume();
                }
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::plus, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::star, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '<') {
                consume();
                tokens.push_back({ .type = TokenType::left_arrow, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '>') {
                consume();
                tokens.push_back({ .type = TokenType::right_arrow, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '-') {
                consume();
                tokens.push_back({ .type = TokenType::minus, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '=') {
                consume();
                tokens.push_back({ .type = TokenType::eq, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '/') {
                consume();
                tokens.push_back({ .type = TokenType::fslash, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '\n') {
                consume();
                m_col = START_COL;
                line_count++;
            }
            else if (std::isspace(peek().value())) {
                consume();
            }
            else {
                std::cerr << "Invalid token" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;
    }

private:
    [[nodiscard]] std::optional<char> peek(const size_t offset = 0) const
    {
        if (m_index + offset >= m_src.length()) {
            return {};
        }
        return m_src.at(m_index + offset);
    }

    char consume()
    {
        m_col++;
        return m_src.at(m_index++);
    }

    const std::string m_src;
    size_t m_index = 0;
};