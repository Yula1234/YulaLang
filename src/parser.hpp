#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<filesystem>
#include<algorithm>

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
	struct Memory {
		std::string name;
		int offset;
	};
	std::vector<std::string> m_includes;
	std::vector<Macro> m_macroses;
	std::vector<Memory> m_memories;
	int m_mem_offset = 0;
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
				case TokenType::load16:
					opsl->push_back(new OP(OP_TYPE::OP_LOAD16, m_tokens[i]));
					break;
				case TokenType::store16:
					opsl->push_back(new OP(OP_TYPE::OP_STORE16, m_tokens[i]));
					break;
				case TokenType::load32:
					opsl->push_back(new OP(OP_TYPE::OP_LOAD32, m_tokens[i]));
					break;
				case TokenType::store32:
					opsl->push_back(new OP(OP_TYPE::OP_STORE32, m_tokens[i]));
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
					m_macroses.push_back(macro);
					break;
				}
				case TokenType::ident: 
				{
					Macro macro;
					bool finded = false;
					std::string cname = m_tokens[i].value.value();
					for(int i = 0;i < m_macroses.size();++i) {
						if(cname == m_macroses[i].name) {
							finded = true;
							macro = m_macroses[i];
							break;
						}
					}
					if(!finded) {
						Memory mem;
						bool mfinded = false;
						std::string cname = m_tokens[i].value.value();
						for(int i = 0;i < m_memories.size();++i) {
							if(cname == m_memories[i].name) {
								mfinded = true;
								mem = m_memories[i];
								break;
							}
						}
						if(!mfinded) {
							ParsingError(m_tokens[i], ("unkown word `" + cname + "`").c_str());
						}
						opsl->push_back(new OP(OP_TYPE::OP_MEM, m_tokens[i], mem.offset));
						break;
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
					std::string fname = "./lib/" + m_tokens[++i].value.value() + ".yula";
					std::string path = std::filesystem::canonical(fname).string();
					if(std::find(m_includes.begin(), m_includes.end(), path) != m_includes.end()) {
						// lib already included
						break;
					}
					std::string contents;
    				{
       	 				std::stringstream contents_stream;
       					std::fstream input(path, std::ios::in);
    	   				contents_stream << input.rdbuf();
    			    	contents = contents_stream.str();
    	    			input.close();
    				}
    				Lexer nlexer(contents);
    				token_list ntokens = nlexer.lex();
    				ops_list* compiled_macro = parse(ntokens);
					opsl->insert(opsl->end(), compiled_macro->begin(), compiled_macro->end() - 1);
					m_includes.push_back(path);
					break;
				}
				case TokenType::mod:
					opsl->push_back(new OP(OP_TYPE::OP_MOD, m_tokens[i]));
					break;
				case TokenType::memory:
				{
					if(i + 2 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at memory definition except memory name and size"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at memory definition except memory name but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					if(m_tokens[i + 2].type != TokenType::int_lit) {
						ParsingError(m_tokens[i+1], ("at memory definition except memory size but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					std::string mname = m_tokens[i + 1].value.value();
					int size = std::stoi(m_tokens[i + 2].value.value());
					i += 2;
					Memory mem = { .name = mname , .offset = m_mem_offset };
					m_mem_offset += size;
					m_memories.push_back(mem);
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