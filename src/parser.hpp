#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<vector>

#include "ops.hpp"
#include "lexer.hpp"

class Parser {
private:
	struct Macro {
		int line;
		int col;
		std::string name;
		token_list _tokens;
	};
	std::vector<Macro> macroses;
public:
	void ParsingError(Token tok, const char* err) {
		std::cout << "Parsing Error at " << tok.line;
		std::cout << "." << tok.col << ": " << err;
		exit(1);
	}
	ops_list* parse(token_list toks) {
		token_list& m_tokens = toks;
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
				case TokenType::cast_ptr:
					opsl->push_back(new OP(OP_TYPE::CAST_PTR, m_tokens[i]));
					break;
				case TokenType::malloc:
					opsl->push_back(new OP(OP_TYPE::OP_MALLOC, m_tokens[i]));
					break;
				case TokenType::free:
					opsl->push_back(new OP(OP_TYPE::OP_FREE, m_tokens[i]));
					break;
				case TokenType::load8:
					opsl->push_back(new OP(OP_TYPE::OP_LOAD8, m_tokens[i]));
					break;
				case TokenType::store8:
					opsl->push_back(new OP(OP_TYPE::OP_STORE8, m_tokens[i]));
					break;
				case TokenType::dup2:
					opsl->push_back(new OP(OP_TYPE::OP_2DUP, m_tokens[i]));
					break;
				case TokenType::band:
					opsl->push_back(new OP(OP_TYPE::OP_BAND, m_tokens[i]));
					break;
				case TokenType::bor:
					opsl->push_back(new OP(OP_TYPE::OP_BOR, m_tokens[i]));
					break;
				case TokenType::shr:
					opsl->push_back(new OP(OP_TYPE::OP_SHR, m_tokens[i]));
					break;
				case TokenType::shl:
					opsl->push_back(new OP(OP_TYPE::OP_SHL, m_tokens[i]));
					break;
				case TokenType::over:
					opsl->push_back(new OP(OP_TYPE::OP_OVER, m_tokens[i]));
					break;
				case TokenType::swap:
					opsl->push_back(new OP(OP_TYPE::OP_SWAP, m_tokens[i]));
					break;
				case TokenType::dump:
					opsl->push_back(new OP(OP_TYPE::OP_DUMP, m_tokens[i]));
					break;
				case TokenType::string_lit:
					opsl->push_back(new OP(OP_TYPE::PUSH_STR, m_tokens[i]));
					break;
				case TokenType::write:
					opsl->push_back(new OP(OP_TYPE::OP_WRITE, m_tokens[i]));
					break;
				case TokenType::macro:
				{
					if(i + 1 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at macro definition except macro name but got nothing"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at macro definition except macro name but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					Macro macro = { .line = m_tokens[i].line, .col = m_tokens[i].col , .name = m_tokens[i+1].value.value() };
					std::vector<Token> bstack;
					i += 2;
					while(m_tokens[i].type != TokenType::end || bstack.size() != 0) {
						switch(m_tokens[i].type) {
						case TokenType::_if: case TokenType::wwhile:
							bstack.push_back(m_tokens[i]);
							break;
						case TokenType::end:
							if(bstack.size() == 0) {
								ParsingError(m_tokens[i], "invalid end");
							}
							bstack.pop_back();
						}
						macro._tokens.push_back(m_tokens[i++]);
					}
					macroses.push_back(macro);
					break;
				}
				case TokenType::ident: 
				{
					Macro macro;
					bool finded = false;
					std::string cname = m_tokens[i].value.value();
					for(int i = 0;i < macroses.size();++i) {
						if(cname == macroses[i].name) {
							finded = true;
							macro = macroses[i];
							break;
						}
					}
					if(!finded) {
						ParsingError(m_tokens[i], ("unkown word `" + cname + "`").c_str());
					}
					ops_list* compiled_macro = parse(macro._tokens);
					opsl->insert(opsl->end(), compiled_macro->begin(), compiled_macro->end() - 1);
					break;
				}
				case TokenType::iinclude:
				{
					if(i + 1 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at include except string file name but got nothing"); 
					}
					if(m_tokens[i + 1].type != TokenType::string_lit) {
						ParsingError(m_tokens[i+1], ("at include except string file name but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					std::string fname = m_tokens[++i].value.value();
					std::string contents;
    				{
       	 				std::stringstream contents_stream;
       					std::fstream input(fname, std::ios::in);
    	   				contents_stream << input.rdbuf();
    			    	contents = contents_stream.str();
    	    			input.close();
    				}
    				Lexer nlexer(contents);
    				token_list ntokens = nlexer.lex();
    				ops_list* compiled_macro = parse(ntokens);
					opsl->insert(opsl->end(), compiled_macro->begin(), compiled_macro->end() - 1);
					break;
				}
				default:
					ParsingError(m_tokens[i], "Invalid token type");
			}

		}
		Token ltok = { .type = TokenType::exit, .line = 0, .col = 0 };
		opsl->push_back(new OP(OP_TYPE::OP_EXIT, ltok));
		return opsl;
	}
};