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
    cast_bool,
    cast_int,
    malloc,
    store8,
    load8,
    free,
    cast_ptr,
    dup2,
    bor,
    band,
    shl,
    shr,
    swap,
    over,
    dump,
    string_lit,
    write,
    macro,
    iinclude,
    store16,
    load16,
    store32,
    load32,
    mod,
    proc,
    in,
    bake,
    memory,
};

std::string tok_to_string(const TokenType type)
{
    switch (type) {
    case TokenType::int_lit:
        return "`number`";
    case TokenType::string_lit:
        return "`string`";
    case TokenType::ident:
        return "`identifier`";
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
    case TokenType::cast_int:
        return "`cast(int)`";
    case TokenType::cast_bool:
        return "`cast(bool)`";
    case TokenType::cast_ptr:
        return "`cast(ptr)`";
    case TokenType::malloc:
        return "`malloc`";
    case TokenType::store8:
        return "`!8`";
    case TokenType::load8:
        return "`@8`";
    case TokenType::store16:
        return "`!16`";
    case TokenType::load16:
        return "`@16`";
    case TokenType::store32:
        return "`!32`";
    case TokenType::load32:
        return "`@32`";
    case TokenType::free:
        return "`free`";
    case TokenType::dup2:
        return "`free`";
    case TokenType::bor:
        return "`|`";
    case TokenType::band:
        return "`&`";
    case TokenType::shl:
        return "`<<`";
    case TokenType::shr:
        return "`>>`";
    case TokenType::swap:
        return "`swap`";
    case TokenType::over:
        return "`over`";
    case TokenType::write:
        return "`write`";
    case TokenType::macro:
        return "`macro`";
    case TokenType::iinclude:
        return "`include`";
    case TokenType::mod:
        return "`%`";
    case TokenType::proc:
        return "`proc`";
    case TokenType::in:
        return "`in`";
    case TokenType::bake:
        return "`--`";
    case TokenType::memory:
        return "`memory`";
    }
    assert(false);
}

struct Token {
    TokenType type;
    int line;
    int col;
    std::optional<std::string> value {};
    friend std::ostream& operator<<(std::ostream& out, const Token& tok) {
        out << "Token(.type = " << tok_to_string(tok.type);
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

bool is_valid_id(char c) {
    switch(c) {
    case '(': case ')':
    case '!': case '@':
    case '?': case '_':
    case '.':
        return true;
    }
    return false;
}

bool is_valid_id_ns(char c) {
    switch(c) {
    case '(': case ')':
    case '!': case '@':
    case '?': case '_':
    case '.': case '-':
    case '$': case '=':
        return true;
    }
    return false;
}

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
            if (std::isalpha(peek().value()) || is_valid_id(peek().value())) {
                buf.push_back(consume());
                while (peek().has_value() && std::isalnum(peek().value()) || ( peek().has_value() && (is_valid_id(peek().value()) || is_valid_id_ns(peek().value())))) {
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
                else if(buf == "false") {
                    tokens.push_back({ .type = TokenType::ffalse, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "cast(bool)") {
                    tokens.push_back({ .type = TokenType::cast_bool, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "cast(int)") {
                    tokens.push_back({ .type = TokenType::cast_int, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "cast(ptr)") {
                    tokens.push_back({ .type = TokenType::cast_ptr, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "malloc") {
                    tokens.push_back({ .type = TokenType::malloc, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "free") {
                    tokens.push_back({ .type = TokenType::free, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "!8") {
                    tokens.push_back({ .type = TokenType::store8, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "@8") {
                    tokens.push_back({ .type = TokenType::load8, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "!16") {
                    tokens.push_back({ .type = TokenType::store16, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "@16") {
                    tokens.push_back({ .type = TokenType::load16, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "!32") {
                    tokens.push_back({ .type = TokenType::store32, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "@32") {
                    tokens.push_back({ .type = TokenType::load32, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "dup2") {
                    tokens.push_back({ .type = TokenType::dup2, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "swap") {
                    tokens.push_back({ .type = TokenType::swap, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "over") {
                    tokens.push_back({ .type = TokenType::over, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "???") {
                    tokens.push_back({ .type = TokenType::dump, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "write") {
                    tokens.push_back({ .type = TokenType::write, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "macro") {
                    tokens.push_back({ .type = TokenType::macro, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "include") {
                    tokens.push_back({ .type = TokenType::iinclude, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "proc") {
                    tokens.push_back({ .type = TokenType::proc, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "in") {
                    tokens.push_back({ .type = TokenType::in, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else if(buf == "memory") {
                    tokens.push_back({ .type = TokenType::memory, .line = line_count, .col = m_col - (int)buf.size() });
                    buf.clear();
                }
                else {
                    tokens.push_back({ .type = TokenType::ident, .line = line_count, .col = m_col - (int)buf.size(), .value = buf });
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
            else if (peek().value() == '%') {
                consume();
                tokens.push_back({ .type = TokenType::mod, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '+') {
                consume();
                tokens.push_back({ .type = TokenType::plus, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '*') {
                consume();
                tokens.push_back({ .type = TokenType::star, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '<' && peek(1).has_value()
                    && peek(1).value() == '<') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::shl, .line = line_count , .col = m_col -2 });
            }
            else if (peek().value() == '>' && peek(1).has_value()
                    && peek(1).value() == '>') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::shr, .line = line_count , .col = m_col -2 });
            }
            else if (peek().value() == '<') {
                consume();
                tokens.push_back({ .type = TokenType::left_arrow, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '>') {
                consume();
                tokens.push_back({ .type = TokenType::right_arrow, .line = line_count , .col = m_col -1 });
            }
            else if(peek().value() == '"') {
                consume();
                buf.clear();
                while(peek().has_value() && peek().value() != '"') {
                    buf.push_back(consume());
                }
                consume();
                for(int i = 0;i < (int)buf.size();++i) {
                    if(buf[i] == '\\') {
                        if(buf[i+1] == 'n') {
                            buf.erase(buf.begin()+i);
                            buf[i] = '\n';
                        }
                    }
                }
                tokens.push_back({ .type = TokenType::string_lit, .line = line_count , .col = m_col - (int)buf.size(), .value = buf });
                buf.clear();
            }
            else if(peek().value() == '\'') {
                consume();
                if(!peek().has_value()) {
                    std::cout << "unclosed '\n";
                }
                char c = consume();
                if(c == '\'') {
                    if(peek().value() == 'n') {
                        c = (char)10;
                    }
                }
                if(!peek().has_value() || peek().value() != '\'') {
                    std::cout << "unclosed '\n";
                }
                tokens.push_back({ .type = TokenType::int_lit, .line = line_count , .col = m_col - (int)buf.size(), .value = std::to_string((int)c) });
                buf.clear();
            }
            else if (peek().value() == '-' && peek(1).has_value()
                    && peek(1).value() == '-') {
                consume();
                consume();
                tokens.push_back({ .type = TokenType::bake, .line = line_count , .col = m_col -2 });
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
            else if (peek().value() == '|') {
                consume();
                tokens.push_back({ .type = TokenType::bor, .line = line_count , .col = m_col -1 });
            }
            else if (peek().value() == '&') {
                consume();
                tokens.push_back({ .type = TokenType::band, .line = line_count , .col = m_col -1 });
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
                std::cerr << "Invalid token at " << m_index << std::endl;
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