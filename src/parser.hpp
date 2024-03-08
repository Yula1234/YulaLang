#pragma once

#include<iostream>
#include<string>
#include<vector>

#include "ops.hpp"
#include "lexer.hpp"

class Parser {
private:
	token_list m_tokens;
public:
	explicit Parser(const token_list _tokens) {
		m_tokens = _tokens;
	}
	void ParsingError(Token tok, const char* err) {
		std::cout << "Parsing Error at " << tok.line;
		std::cout << "." << tok.col << ": " << err;
		exit(1);
	}
	ops_list* parse() {
		ops_list* opsl = new std::vector<OP*>();
		int size = m_tokens.size();
		for(int i = 0;i < size;++i) {
			switch(m_tokens[i].type) {
				case TokenType::int_lit:
					opsl->push_back(new OP(OP_TYPE::PUSH_INT, m_tokens[i], std::stoi(m_tokens[i].value.value())));
					break;
				case TokenType::print:
					opsl->push_back(new OP(OP_TYPE::INTR_PRINT, m_tokens[i]));
					break;
				case TokenType::plus:
					opsl->push_back(new OP(OP_TYPE::OPER_ADD, m_tokens[i]));
					break;
				case TokenType::minus:
					opsl->push_back(new OP(OP_TYPE::OPER_SUB, m_tokens[i]));
					break;
				case TokenType::star:
					opsl->push_back(new OP(OP_TYPE::OPER_MUL, m_tokens[i]));
					break;
				case TokenType::fslash:
					opsl->push_back(new OP(OP_TYPE::OPER_DIV, m_tokens[i]));
					break;
				case TokenType::_if:
					opsl->push_back(new OP(OP_TYPE::OP_IF, m_tokens[i]));
					break;
				case TokenType::end:
					opsl->push_back(new OP(OP_TYPE::OP_END, m_tokens[i]));
					break;
				case TokenType::_else:
					opsl->push_back(new OP(OP_TYPE::OP_ELSE, m_tokens[i]));
					break;
				case TokenType::exit:
					opsl->push_back(new OP(OP_TYPE::OP_EXIT, m_tokens[i]));
					break;
				case TokenType::eq:
					opsl->push_back(new OP(OP_TYPE::OP_EQ, m_tokens[i]));
					break;
				case TokenType::left_arrow:
					opsl->push_back(new OP(OP_TYPE::OP_LESS, m_tokens[i]));
					break;
				case TokenType::right_arrow:
					opsl->push_back(new OP(OP_TYPE::OP_ABOVE, m_tokens[i]));
					break;
				case TokenType::dup:
					opsl->push_back(new OP(OP_TYPE::OP_DUP, m_tokens[i]));
					break;
				case TokenType::ddo:
					opsl->push_back(new OP(OP_TYPE::OP_DO, m_tokens[i]));
					break;
				case TokenType::wwhile:
					opsl->push_back(new OP(OP_TYPE::OP_WHILE, m_tokens[i]));
					break;
				case TokenType::drop:
					opsl->push_back(new OP(OP_TYPE::OP_DROP, m_tokens[i]));
					break;
				case TokenType::land:
					opsl->push_back(new OP(OP_TYPE::OP_AND, m_tokens[i]));
					break;
				case TokenType::lor:
					opsl->push_back(new OP(OP_TYPE::OP_OR, m_tokens[i]));
					break;
				case TokenType::lnot:
					opsl->push_back(new OP(OP_TYPE::OP_NOT, m_tokens[i]));
					break;
				case TokenType::ttrue:
					opsl->push_back(new OP(OP_TYPE::OP_TRUE, m_tokens[i]));
					break;
				case TokenType::ffalse:
					opsl->push_back(new OP(OP_TYPE::OP_FALSE, m_tokens[i]));
					break;
				case TokenType::cast_int:
					opsl->push_back(new OP(OP_TYPE::CAST_INT, m_tokens[i]));
					break;
				case TokenType::cast_bool:
					opsl->push_back(new OP(OP_TYPE::CAST_BOOL, m_tokens[i]));
					break;
				default:
					ParsingError(m_tokens[i], "Invalid token type");
			}

		}
		Token ltok = { .type = TokenType::exit, .line = 0, .col = 0 };
		opsl->push_back(new OP(OP_TYPE::OP_EXIT, ltok));
		return opsl;
	}
};