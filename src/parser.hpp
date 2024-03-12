#pragma once

#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<filesystem>
#include<algorithm>

#include "ops.hpp"
#include "lexer.hpp"

enum class DataType {
	_int,
	_bool,
	ptr,
};

std::string dt_tostr(DataType dt) {
	switch(dt) {
		case DataType::_int:
			return "INT";
		case DataType::_bool:
			return "BOOL";
		case DataType::ptr:
			return "PTR";
	}
	assert(false);
	return "";
}

struct eval_result {
	int value;
	int diff;
	DataType type;
	std::vector<int> stack;
};

struct DataElement {
	DataType type;
	friend std::ostream& operator<<(std::ostream& out, DataElement& de) {
        out << "<" << dt_tostr(de.type) << ">";
        return out;
    }
    friend bool operator==(const DataElement one, const DataElement two) {
        return one.type == two.type;
    }
};

DataElement type_to_dt(TokenType tp) {
	switch(tp) {
		case TokenType::type_int:
			return { .type = DataType::_int };
		case TokenType::type_bool:
			return { .type = DataType::_bool };
		case TokenType::type_ptr:
			return { .type = DataType::ptr };
	}
	std::cout << "AssertionError at TokenType to DataType\n";
	std::cout << "except Token Type but got " << tok_to_string(tp) << "\n";
	exit(1);
}

#define DataStack std::vector<DataElement>

void show_sdata(DataStack& ds) {
	int size = ds.size();
	std::cout << "[";
	for(int i = 0;i < size;++i) {
		std::cout << ds[i];
		if(i != (size - 1)) {
			std::cout << ", ";
		}
	}
	std::cout << "]\n";
}

struct Memory {
	std::string name;
	int offset;
	bool local;
};

struct Procedure {
	std::string name;
	DataStack ins;
	DataStack outs;
	Token def;
	int ip;
	int local_mem_cap;
	std::vector<Memory> local_mems;
};

std::optional<Procedure> proc_lookup(std::vector<Procedure>& m_procs, std::string& name) {
	for(int i = 0;i < m_procs.size();++i) {
		if(name == m_procs[i].name) {
			return m_procs[i];
		}
	}
	return std::nullopt;
}

Procedure* proc_lookup_rv(std::vector<Procedure>& m_procs, std::string& name) {
	for(int i = 0;i < m_procs.size();++i) {
		if(name == m_procs[i].name) {
			return &(m_procs[i]);
		}
	}
	assert(false); // at rvalue unreacheable
	return nullptr;
}

class Parser {
private:
	struct Macro {
		int line;
		int col;
		std::string name;
		token_list _tokens;
	};
	std::vector<std::string> m_includes;
	std::vector<Macro> m_macroses;
	std::vector<Memory> m_memories;
	std::vector<Procedure> m_procs;
	int m_mem_offset = 0;
public:
	int get_memsize() {
		return m_mem_offset;
	}
	std::vector<Procedure> get_procs() {
		return m_procs;
	}
	void ParsingError(Token tok, const char* err) {
		std::cout << "Parsing Error at " << tok.line;
		std::cout << "." << tok.col << ": " << err;
		exit(1);
	}
	eval_result eval_const_value(token_list& m_tokens, int s_ip, bool s0) {
		eval_result res = {0, {}};
		for(int ip = s_ip;m_tokens[ip].type != TokenType::end && ip < m_tokens.size();++ip) {
			res.diff = ip - s_ip;
			switch(m_tokens[ip].type) {
				case TokenType::int_lit:
				{
					res.stack.push_back(std::stoi(m_tokens[ip].value.value()));
					break;
				}
				case TokenType::plus:
				{
					if(res.stack.size() < 2) {
						ParsingError(m_tokens[ip], "not enought arguments for +\n");
					}
					int one = res.stack[res.stack.size() - 1];
					res.stack.pop_back();
					int two = res.stack[res.stack.size() - 1];
					res.stack.pop_back();
					res.stack.push_back(two + one);
					break;
				}
				case TokenType::star:
				{
					if(res.stack.size() < 2) {
						ParsingError(m_tokens[ip], "not enought arguments for *\n");
					}
					int one = res.stack[res.stack.size() - 1];
					res.stack.pop_back();
					int two = res.stack[res.stack.size() - 1];
					res.stack.pop_back();
					res.stack.push_back(two * one);
					break;
				}
				case TokenType::ident:
				{
					Macro macro;
					bool finded = false;
					std::string cname = m_tokens[ip].value.value();
					for(int i = 0;i < m_macroses.size();++i) {
						if(cname == m_macroses[i].name) {
							finded = true;
							macro = m_macroses[i];
							break;
						}
					}
					if(finded) {
						eval_result m_er = eval_const_value(macro._tokens, 0, false);
						res.stack.insert(res.stack.end(), m_er.stack.begin(), m_er.stack.end());
						break;
					}
					ParsingError(m_tokens[ip], ("at constant unkown word `" + m_tokens[ip].value.value() + "`\n").c_str());
					break;
				}
				default:
				{
					Token tok = m_tokens[ip];
					ParsingError(tok, ("constant evaluating of " + tok_to_string(tok.type) + " its not allowed!\n").c_str());
				}
			}
		}
		if(res.stack.size() != 1 && s0) {
			std::cout << res.stack.size() << std::endl;
			ParsingError(m_tokens[s_ip], "invalid constant");
		}
		res.value = res.stack[0];
		res.type = DataType::_int;
		return res;
	}
	ops_list* parse(token_list& toks) {
		token_list& m_tokens = toks;
		ops_list* opsl = new std::vector<OP*>();
		int size = m_tokens.size();
		int real_ip = 0;
		bool is_sproc = false;
		std::string sproc_name = "";
		for(int i = 0;i < size;++i, ++real_ip) {
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
					is_sproc = false;
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
					real_ip -= 2;
					break;
				case TokenType::cast_bool:
					opsl->push_back(new OP(OP_TYPE::CAST_BOOL, m_tokens[i]));
					real_ip -= 2;
					break;
				case TokenType::cast_ptr:
					opsl->push_back(new OP(OP_TYPE::CAST_PTR, m_tokens[i]));
					real_ip -= 2;
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
				case TokenType::macro:
				{
					Token MacroDef = m_tokens[i];
					if(i + 1 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at macro definition except macro name but got nothing"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at macro definition except macro name but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					std::string mcname = m_tokens[i+1].value.value();
					for(int i = 0;i < m_macroses.size();++i) {
						if(mcname == m_macroses[i].name) {
							std::cout << "at " << MacroDef.line << "." << MacroDef.col;
							std::cout << " redefinition of macro `" << mcname << "`";
							std::cout << "NOTE: first defenition " << m_macroses[i].line << "." << m_macroses[i].col << "\n";
							exit(1);
						}
					}
					Macro macro = { .line = m_tokens[i].line, .col = m_tokens[i].col , .name = mcname };
					std::vector<Token> bstack;
					i += 2;
					while(m_tokens[i].type != TokenType::end || bstack.size() != 0) {
						switch(m_tokens[i].type) {
						case TokenType::_if: case TokenType::wwhile:
							bstack.push_back(m_tokens[i]);
							break;
						case TokenType::macro:
							ParsingError(MacroDef, ("Unclosed macro `" + mcname + "`").c_str());
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
					if(finded) {
						ops_list* compiled_macro = parse(macro._tokens);
						opsl->insert(opsl->end(), compiled_macro->begin(), compiled_macro->end() - 1);
						break;
					}

					Memory mem;
					bool mfinded = false;
					for(int i = 0;i < m_memories.size();++i) {
						if(cname == m_memories[i].name) {
							mfinded = true;
							mem = m_memories[i];
							break;
						}
					}
					if(mfinded) {
						opsl->push_back(new OP(OP_TYPE::OP_MEM, m_tokens[i], mem.offset, (int)mem.local));
						break;
					} else {
						if(is_sproc) {
							Procedure* cp = proc_lookup_rv(m_procs, sproc_name);
							Memory mem;
							bool mfinded = false;
							for(int i = 0;i < cp->local_mems.size();++i) {
								if(cname == cp->local_mems[i].name) {
									mfinded = true;
									mem = cp->local_mems[i];
									break;
								}
							}
							if(mfinded) {
								opsl->push_back(new OP(OP_TYPE::OP_MEM, m_tokens[i], mem.offset, (int)mem.local));
								break;
							}
						}
					}

					std::optional<Procedure> proc = proc_lookup(m_procs, cname);
					if(proc.has_value()) {
						opsl->push_back(new OP(OP_TYPE::OP_CALL, m_tokens[i], proc.value().ip));
						break;
					}

					ParsingError(m_tokens[i], ("unkown word `" + cname + "`").c_str());
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
				case TokenType::rot:
					opsl->push_back(new OP(OP_TYPE::OP_ROT, m_tokens[i]));
					break;
				case TokenType::memory:
				{
					if(i + 3 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at memory definition except memory name and size, and end"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at memory definition except memory name but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					if(m_tokens[i + 2].type != TokenType::int_lit) {
						ParsingError(m_tokens[i+1], ("at memory definition except memory size but got " + tok_to_string(m_tokens[i + 1].type)).c_str());
					}
					std::string mname = m_tokens[i + 1].value.value();
					eval_result er = eval_const_value(m_tokens, i + 2, true);
					i += er.diff + 3;
					int size = er.value;
					Memory mem = { .name = mname , .offset = m_mem_offset, .local = is_sproc };
					if(!is_sproc) {
						m_mem_offset += size;
						m_memories.push_back(mem);
					} else {
						Procedure* cp = proc_lookup_rv(m_procs, sproc_name);
						mem.offset = cp->local_mem_cap;
						cp->local_mems.push_back(mem);
						cp->local_mem_cap += size;
					}
					break;
				}
				case TokenType::proc:
				{
					if(i + 3 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at procedure definition except procedure name and ins, outs, and in\n"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at procedure definition except procedure name but got " + tok_to_string(m_tokens[i + 1].type) + "\n").c_str());
					}
					Token ProcDef = m_tokens[i];
					is_sproc = true;
					i += 1; // skip `proc` token
					std::string pname = m_tokens[i++].value.value();
					sproc_name = pname;
					std::optional<Procedure> prc = proc_lookup(m_procs, pname);
					if(prc.has_value()) {
						std::cout << "at " << ProcDef.line << "." << ProcDef.col;
						std::cout << " procedure `" << pname << "` redefinition.\n";
						std::cout << "NOTE: first defenition at " << prc.value().def.line << prc.value().def.col << "\n";
						exit(1);
					}
					DataStack _outs;
					DataStack _ins;
					while(m_tokens[i].type != TokenType::bake) {
						if(m_tokens[i].type == TokenType::end || m_tokens[i].type == TokenType::in) {
							ParsingError(ProcDef, "except -- after proc definition with types\n");
						}
						_ins.push_back(type_to_dt(m_tokens[i].type));
						i += 1;
					}
					i += 1;
					while(m_tokens[i].type != TokenType::in) {
						if(m_tokens[i].type == TokenType::end) {
							ParsingError(ProcDef, "except `in` after proc definition\n");
						}
						_outs.push_back(type_to_dt(m_tokens[i].type));
						i += 1;
					}
					assert(m_tokens[i].type == TokenType::in);
					if(_outs.size() > 3) {
						ParsingError(m_tokens[i], "procedures outputs length > 3 (after --).\n");
					}
					if(_ins.size() > 24) {
						ParsingError(m_tokens[i], "procedures inputs length > 24 (after proc).\n");
					}
					m_tokens[i].value = pname;
					opsl->push_back(new OP(OP_TYPE::OP_SKIP_PROC, m_tokens[i]));
					opsl->push_back(new OP(OP_TYPE::OP_PROC, m_tokens[i]));
					Procedure proc = {pname, _ins, _outs, ProcDef, real_ip, 0};
					m_procs.push_back(proc);
					break;
				}
				case TokenType::c_call1:
				case TokenType::c_call2:
				case TokenType::c_call3:
				{
					if(i + 1 > m_tokens.size()) {
						ParsingError(m_tokens[i], "at c_call(1) except c_function name but got nothing\n"); 
					}
					if(m_tokens[i + 1].type != TokenType::ident) {
						ParsingError(m_tokens[i+1], ("at c_call(1) except c_function name but got " + tok_to_string(m_tokens[i + 1].type) + "\n").c_str());
					}
					std::string cfname = m_tokens[i + 1].value.value();
					int argc;
					switch(m_tokens[i].type) {
					case TokenType::c_call1:
						argc = 1;
						break;
					case TokenType::c_call2:
						argc = 2;
						break;
					case TokenType::c_call3:
						argc = 3;
						break;
					}
					opsl->push_back(new OP(OP_TYPE::OP_C_CALL, m_tokens[++i], argc));
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