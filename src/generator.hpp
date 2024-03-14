#pragma once

#include<iostream>
#include<string>
#include<sstream>
#include<vector>
#include<cassert>
#include<cstdarg>

#include "ops.hpp"
#include "lexer.hpp"
#include "lexer.hpp"

#define MSTACK_CAPACITY 8012

bool typecheck(DataStack& ds, int length, ...) {
	if(ds.size() < length) {
		return false;
	}
	va_list args;
	va_start(args, length);
	int ip = ds.size();
	std::vector<DataType> extypes;
	for(int i = 0;i < length;++i) {
		extypes.push_back(va_arg(args, DataType));
	}
	for(int i = extypes.size() - 1;i > -1;--i) {
		DataType cur = extypes[i];
		if(ds[--ip].type != cur) {
			return false;
		}
	}
	va_end(args);
	return true;
}

bool typecheckds(DataStack& ds, DataStack& extypes) {
	if(ds.size() < extypes.size()) {
		return false;
	}
	int ip = ds.size();
	for(int i = extypes.size() - 1;i > -1;--i) {
		DataElement cur = extypes[i];
		if(ds[--ip].type != cur.type) {
			return false;
		}
	}
	return true;
}


std::vector<std::string> std_externs = {
	"ExitProcess@4",
	"printf",
};


void crossref_check_blocks(ops_list* ops, std::vector<Procedure> m_procs) {
	std::vector<OP*> stack;
	std::vector<int> locs;
	for(int ip = 0;ip < ops->size();++ip) {
		switch((*(ops))[ip]->type) {
			case OP_TYPE::OP_IF:
				stack.push_back((*(ops))[ip]);
				break;
			case OP_TYPE::OP_SKIP_PROC:
				stack.push_back((*(ops))[ip]);
				break;
			case OP_TYPE::OP_WHILE:
				stack.push_back((*(ops))[ip]);
				locs.push_back(ip);
				break;
			case OP_TYPE::OP_DO:
				if(stack.size() == 0) {
					OP cur_end = *((*(ops))[ip]);
					Token etok = cur_end.tok;
					std::cout << "at " << etok.line << ":";
					std::cout << etok.col << " do without while\n";
					exit(1);
				}
				if(stack[stack.size() - 1]->type != OP_TYPE::OP_WHILE) {
					OP cur_end = *((*(ops))[ip]);
					Token etok = cur_end.tok;
					std::cout << "at " << etok.line << ":";
					std::cout << etok.col << " do without while\n";
					exit(1);
				}
				stack.push_back((*(ops))[ip]);
				break;
			case OP_TYPE::OP_END: {
				if(stack.size() == 0) {
					OP cur_end = *((*(ops))[ip]);
					Token etok = cur_end.tok;
					std::cout << "at " << etok.line << ":";
					std::cout << etok.col << " end without [if, else, do]\n";
					exit(1);
				}
				OP* cur = (*(ops))[ip];
				cur->operand1 = -1;
				int oldop1 = stack[stack.size() - 1]->operand1;
				stack[stack.size() - 1]->operand1 = ip;
				if(stack[stack.size() - 1]->type == OP_TYPE::OP_DO) {
					stack.pop_back();
					cur->operand1 = locs[locs.size() - 1];
					locs.pop_back();
					stack.pop_back();
					break;
				}
				if(stack[stack.size() - 1]->type == OP_TYPE::OP_SKIP_PROC) {
					cur->operand1 = -2;
					for(int i = 0;i < m_procs.size();++i) {
						if(m_procs[i].name == stack[stack.size() - 1]->tok.value.value()) {
							cur->operand2 = i;
							break;
						}
					}
					stack.pop_back();
					break;
				}
				stack.pop_back();
				break;
			}
			case OP_TYPE::OP_ELSE: {
				if(stack.size() == 0) {
					OP cur_else = *((*(ops))[ip]);
					Token etok = cur_else.tok;
					std::cout << "at " << etok.line << ":";
					std::cout << etok.col << " else without if\n";
					exit(1);
				}
				OP* cur = (*(ops))[ip];
				stack[stack.size() - 1]->operand1 = ip + 1;
				stack.pop_back();
				stack.push_back((*(ops))[ip]);
				break;
			}
		}
	}
	if(stack.size() != 0) {
		OP* enblock = stack[0];
		Token tok = enblock->tok;
		std::cout << "at " << tok.line << ":" << tok.col;
		std::cout << " unclosed block (" << op_to_string(enblock->type) << ")\n";
		exit(1);
	}
}

DataStack& last_scope(std::vector<DataStack*>& scopes) {
	return *(scopes[scopes.size() - 1]);
}

void err_unhandled_data(DataStack& ds) {
	std::cout << "unhandled data on the stack:\n";
	show_sdata(ds);
	exit(1);
}

void TypeError(DataStack& ds, int ip, ops_list* ops, const char* err) {
	std::cout << "TypeError at " << (*((*ops)[ip])).tok.line << "." << (*((*ops)[ip])).tok.col << ": " << err << std::endl;
	show_sdata(ds);
	exit(1);
}


void typecheck_program(ops_list* ops, std::vector<Procedure> m_procs) {
	std::vector<DataStack*> scopes;
	scopes.push_back(new DataStack);
	std::vector<OP*> bstack;
	std::vector<int> ifsizes;
	Procedure tcproc = {};
	bool is_proc = false;
	for(int ip = 0;ip < ops->size();++ip) {
		switch((*ops)[ip]->type) {
			case OP_TYPE::PUSH_INT:
				last_scope(scopes).push_back({ .type = DataType::_int });
				break;
			case OP_TYPE::OP_PROC:
			{
				scopes.push_back(new DataStack);
				is_proc = true;
				OP* cur = (*ops)[ip];
				DataStack& ds = last_scope(scopes);
				Procedure proc = proc_lookup(m_procs, cur->tok.value.value()).value();
				tcproc = proc;
				ds.insert(ds.end(), proc.ins.begin(), proc.ins.end());
				break;
			}
			case OP_TYPE::OP_CALL:
			{
				OP* cur = (*ops)[ip];
				DataStack& ds = last_scope(scopes);
				Procedure proc = proc_lookup(m_procs, cur->tok.value.value()).value();
				if(!typecheckds(ds, proc.ins)) {
					std::cout << "at " << cur->tok.line << "." << cur->tok.col;
					std::cout << " procedure `" << proc.name << "` except types: \n";
					show_sdata(proc.ins);
					std::cout << "but got:\n";
					show_sdata(ds);
					exit(1);
				}
				for(int i = 0;i < proc.ins.size();++i) {
					ds.pop_back();
				}
				ds.insert(ds.end(), proc.outs.begin(), proc.outs.end());
				break;
			}
			case OP_TYPE::OP_TRUE:
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::OP_FALSE:
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::INTR_PRINT:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::_int)) {
					TypeError(ds, ip, ops, "print excepts types [INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OPER_ADD:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int) && !typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "+ excepts types [INT, INT], but got: ");
				}
				if(typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					ds.pop_back();
					ds.pop_back();
					ds.push_back({ .type = DataType::ptr });
					break;
				}
				ds.pop_back();
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OPER_SUB:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int) && !typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "- excepts types [INT, INT], but got: ");
				}
				if(typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					ds.pop_back();
					ds.pop_back();
					ds.push_back({ .type = DataType::ptr });
					break;
				}
				ds.pop_back();
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OPER_MUL:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int) && !typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "* excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OPER_DIV:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "/ excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_MOD:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "% excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_BOR:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int) && !typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "| excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_BAND:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "& excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_SHR:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, ">> excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_SHL:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "<< excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_ELSE:
			{
				DataStack& ds = last_scope(scopes);
				if(ifsizes.size() == 0) {
					std::cout << "else without if\n";
					exit(1); // TODO: propper error message
				}
				while(ds.size() > ifsizes[ifsizes.size() - 1]) {
					ds.pop_back();
				}
				ifsizes.pop_back();
				break;
			}
			case OP_TYPE::OP_END:
			{
				if(is_proc && bstack.size() == 0) {
					DataStack& ds = last_scope(scopes);
					if((!typecheckds(ds, tcproc.outs)) || ds.size() != tcproc.outs.size()) {
						std::cout << "at " << tcproc.def.line << "." << tcproc.def.col;
						std::cout << " at end of the procedure except types: ";
						show_sdata(tcproc.outs);
						std::cout << "but got: ";
						show_sdata(ds);
						exit(1);
					}
					scopes.pop_back();
					is_proc = false;
				} else {
					bstack.pop_back();
				}
				break;
			}
			case OP_TYPE::OP_IF:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::_bool)) {
					TypeError(ds, ip, ops, "if excepts types [BOOL], but got: ");
				}
				ds.pop_back();
				bstack.push_back((*ops)[ip]);
				ifsizes.push_back(ds.size());
				break;
			}
			case OP_TYPE::OP_EXIT:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::_int)) {
					TypeError(ds, ip, ops, "at end of the program excepts types [INT], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_EQ:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "= excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				ds.push_back({ .type = DataType::_bool });
				break;
			}
			case OP_TYPE::OP_ABOVE:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "> excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				ds.push_back({ .type = DataType::_bool });
				break;
			}
			case OP_TYPE::OP_LESS:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_int, DataType::_int)) {
					TypeError(ds, ip, ops, "< excepts types [INT, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				ds.push_back({ .type = DataType::_bool });
				break;
			}
			case OP_TYPE::OP_NOT:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::_bool)) {
					TypeError(ds, ip, ops, "not excepts types [BOOL], but got: ");
				}
				break;
			}
			case OP_TYPE::OP_OR:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_bool, DataType::_bool)) {
					TypeError(ds, ip, ops, "or excepts types [BOOL, BOOL], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_AND:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::_bool, DataType::_bool)) {
					TypeError(ds, ip, ops, "and excepts types [BOOL, BOOL], but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_DUP:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 1) {
					TypeError(ds, ip, ops, "dup excepts 1 element, but got: ");
				}
				ds.push_back(ds[ds.size() - 1]);
				break;
			}
			case OP_TYPE::OP_DO:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::_bool)) {
					TypeError(ds, ip, ops, "do except types [BOOL], but got: ");
				}
				ds.pop_back();
				bstack.push_back((*ops)[ip]);
				break;
			}
			case OP_TYPE::OP_DROP:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 1) {
					TypeError(ds, ip, ops, "drop excepts 1 element, but got: ");
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::CAST_INT:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 1) {
					TypeError(ds, ip, ops, "cast(int) excepts 1 element, but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				ops->erase(ops->begin() + ip--);
				break;
			}
			case OP_TYPE::CAST_BOOL:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 1) {
					TypeError(ds, ip, ops, "cast(bool) excepts 1 element, but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::_bool });
				ops->erase(ops->begin() + ip--);
				break;
			}
			case OP_TYPE::CAST_PTR:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 1) {
					TypeError(ds, ip, ops, "cast(ptr) excepts 1 element, but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::ptr });
				ops->erase(ops->begin() + ip--);
				break;
			}
			case OP_TYPE::OP_STORE8:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "!8 excepts types [PTR, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_LOAD8:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::ptr)) {
					TypeError(ds, ip, ops, "@8 excepts types [PTR], but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OP_STORE16:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "!16 excepts types [PTR, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_LOAD16:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::ptr)) {
					TypeError(ds, ip, ops, "@16 excepts types [PTR], but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OP_STORE32:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 2, DataType::ptr, DataType::_int)) {
					TypeError(ds, ip, ops, "!32 excepts types [PTR, INT], but got: ");
				}
				ds.pop_back();
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_LOAD32:
			{
				DataStack& ds = last_scope(scopes);
				if(!typecheck(ds, 1, DataType::ptr)) {
					TypeError(ds, ip, ops, "@32 excepts types [PTR], but got: ");
				}
				ds.pop_back();
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OP_2DUP:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 2) {
					TypeError(ds, ip, ops, "2dup excepts 2 elements, but got: ");
				}
				DataElement cur = ds[ds.size() - 1];
				DataElement last = ds[ds.size() - 2];
				ds.push_back(last);
				ds.push_back(cur);
				break;
			}
			case OP_TYPE::OP_OVER:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 2) {
					TypeError(ds, ip, ops, "over excepts 2 elements, but got: ");
				}
				ds.push_back(ds[ds.size() - 2]);
				break;
			}
			case OP_TYPE::OP_ROT:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 3) {
					TypeError(ds, ip, ops, "rot excepts 3 elements, but got: ");
				}
				ds.push_back(ds[ds.size() - 3]);
				break;
			}
			case OP_TYPE::OP_SWAP:
			{
				DataStack& ds = last_scope(scopes);
				if(ds.size() < 2) {
					TypeError(ds, ip, ops, "swap excepts 2 elements, but got: ");
				}
				DataElement last = ds[ds.size() - 2];
				DataElement cur = ds[ds.size() - 1];
				ds.pop_back();
				ds.pop_back();
				ds.push_back(cur);
				ds.push_back(last);
				break;
			}
			case OP_TYPE::OP_DUMP:
			{
				OP* cur_dump = (*ops)[ip];
				Token tok = cur_dump->tok;
				std::cout << "??? dump at " << tok.line << "." << tok.col << ": ";
				show_sdata(last_scope(scopes));
				exit(1);
				break;
			}
			case OP_TYPE::PUSH_STR:
			{
				DataStack& ds = last_scope(scopes);
				ds.push_back({ .type = DataType::ptr });
				break;
			}
			case OP_TYPE::OP_MEM:
			{
				last_scope(scopes).push_back({ .type = DataType::ptr });
				break;
			}
			case OP_TYPE::OP_C_CALL:
			{
				DataStack& ds = last_scope(scopes);
				int argc = (*ops)[ip]->operand1;
				if(ds.size() < argc) {
					TypeError(ds, ip, ops, "not enought arguments for c_call(...)");
				}
				for(int i = 0;i < argc;++i) {
					ds.pop_back();
				}
				ds.push_back({ .type = DataType::_int });
				break;
			}
			case OP_TYPE::OP_BIND:
			{
				DataStack& ds = last_scope(scopes);
				OP* curbd = (*ops)[ip];
				DataType type = (DataType)curbd->operand1;
				if(!typecheck(ds, 1, type)) {
					TypeError(ds, ip, ops, ("let except type " + dt_tostr(type)).c_str());
				}
				ds.pop_back();
				break;
			}
			case OP_TYPE::OP_PUSH_BIND:
			{
				DataStack& ds = last_scope(scopes);
				OP* curpb = (*ops)[ip];
				ds.push_back({ .type = (DataType)curpb->operand2 });
				break;
			}
		}
	}
	DataStack& ds = last_scope(scopes);
	if(ds.size() > 0) {
		std::cout << "at end of the program except INT, but got unhandled data: ";
		show_sdata(ds);
		exit(1);
	}
}

class Generator {
private:
	struct String {
		int index;
		std::string data;
	};
	ops_list* m_ops;
	std::stringstream m_output;
	std::vector<String> m_strings;
	std::vector<Procedure> m_procs;
	int m_memsize = 96;
	int m_bind_nest = 0;
public:
	explicit Generator(ops_list* _ops) {
		m_ops = _ops;
	}
	void set_procs(std::vector<Procedure> _procs) {
		m_procs = _procs;
	} 
	void set_memsize(int nmemsize) {
		m_memsize = nmemsize;
	}
	OP m_at(int index) {
		return *((*m_ops)[index]);
	}
	void m_new_addr(int ip) {
		m_output << "addr_" << ip << ":";
		OP cur_op = m_at(ip);
		Token tok = cur_op.tok;
		m_output << "            ; " << tok.line << ":" << tok.col;
		m_output << " `" << op_to_string(cur_op.type) << "`";
		m_output << "\n";
	}
	void GeneratorError(std::string err, int ip) {
		std::cout << "Generator Error at " << m_at(ip).tok.line << "." << m_at(ip).tok.col << ": " << err << std::endl;
		exit(1);
	}
	void GeneratorErrorWSS(DataStack& ds, std::string err, int ip) {
		std::cout << "Generator Error at " << m_at(ip).tok.line << "." << m_at(ip).tok.col << ": " << err << std::endl;
		show_sdata(ds);
		exit(1);
	}
	void err_unhandled_data(DataStack& ds) {
		std::cout << "unhandled data on the stack:\n";
		show_sdata(ds);
		exit(1);
	}
	void m_gen_push_int(int ip) {
		m_new_addr(ip);
		m_output << "    push " << m_at(ip).operand1 << "\n";
	}
	void m_gen_print_intrinsic(int ip) {
		m_new_addr(ip);
		m_output << "    push numfmt\n";
		m_output << "    call printf\n";
		m_output << "    add esp, 8\n";
	}
	void m_gen_bor(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    or eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_band(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    and eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_add_oper(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    add eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_sub_oper(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    sub eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_mul_oper(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    imul eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_div_oper(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    div ebx\n";
		m_output << "    push eax\n";
		m_output << "    mov edx, ecx\n";
	}
	void m_gen_mod(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    div ebx\n";
		m_output << "    push edx\n";

	}
	void m_gen_eq(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    mov ecx, 1\n";
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    cmp eax, ebx\n";
		m_output << "    cmove edx, ecx\n";
		m_output << "    push edx\n";
	}
	void m_gen_above(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    mov ecx, 1\n";
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    cmp eax, ebx\n";
		m_output << "    cmova edx, ecx\n";
		m_output << "    push edx\n";
	}
	void m_gen_less(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    mov ecx, 1\n";
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    cmp eax, ebx\n";
		m_output << "    cmovb edx, ecx\n";
		m_output << "    push edx\n";
	}
	void m_gen_exit(int ip) {
		m_new_addr(ip);
		m_output << "    call ExitProcess@4\n";
		m_output << "    add esp, 4\n";
		m_output << "    pop ebp\n";
		m_output << "    ret\n";
	}
	void m_gen_prog_end(int ip) {
		m_new_addr(ip);
		Procedure mproc = proc_lookup(m_procs, "main").value();
		m_output << "    call addr_" << mproc.ip << "\n";
		m_output << "    push 0\n";
		m_output << "    call ExitProcess@4\n";
		m_output << "    add esp, 4\n";
		m_output << "    pop ebp\n";
		m_output << "    ret\n";
	}
	void m_gen_if(int ip) {
		m_new_addr(ip);
		m_output << "    pop edx\n";
		m_output << "    test edx, edx\n";
		m_output << "    jz addr_" << m_at(ip).operand1 << "\n";
	}
	void m_gen_end(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		if(cur.operand1 != -1 && cur.operand1 != -2 && cur.operand1 != -3) {
			m_output << "    jmp addr_" << cur.operand1 << "\n";
			return;
		}
		if(cur.operand1 == -2) {
			Procedure& proc_ret = m_procs[cur.operand2];
			int rsize = proc_ret.outs.size();
			if(rsize == 1) {
				m_output << "    pop eax\n";
			}
			else if(rsize == 0) {}
			else if(rsize == 2) {
				m_output << "    pop eax\n";
				m_output << "    pop ebx\n";
			}
			else if(rsize == 3) {
				m_output << "    pop eax\n";
				m_output << "    pop ebx\n";
				m_output << "    pop edx\n";
			} else {
				assert(false); // unreacheable
			}
			if(proc_ret.local_mem_cap != 0) {
				m_output << "    add esp, " << proc_ret.local_mem_cap << "\n";
			}
			m_output << "    pop ebp\n";
			m_output << "    ret\n";
		}
	}
	void m_gen_else(int ip) {
		m_new_addr(ip);
		m_output << "    jmp addr_" << m_at(ip).operand1 << "\n";
	}
	void m_gen_dup(int ip) {
		m_new_addr(ip);
		m_output << "    push dword [esp]\n";
	}
	void m_gen_2dup(int ip) {
		m_new_addr(ip);
		m_output << "    push dword [esp]\n";
		m_output << "    push dword [esp+8]\n";
	}
	void m_gen_do(int ip) {
		m_new_addr(ip);
		m_output << "    pop edx\n";
		m_output << "    test edx, edx\n";
		m_output << "    jz addr_" << m_at(ip).operand1 + 1 << "\n";
	}
	void m_gen_while(int ip) {
		m_new_addr(ip);
	}
	void m_gen_drop(int ip) {
		m_new_addr(ip);
		m_output << "    add esp, 4\n";
	}
	void m_gen_not(int ip) {
		m_new_addr(ip);
		m_output << "    mov edx, 0\n";
		m_output << "    mov ecx, 1\n";
		m_output << "    pop eax\n";
		m_output << "    cmp eax, 0\n";
		m_output << "    cmove edx, ecx\n";
		m_output << "    push edx\n";
	}
	void m_gen_or(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    or eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_and(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    pop eax\n";
		m_output << "    and eax, ebx\n";
		m_output << "    push eax\n";
	}
	void m_gen_shl(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    pop eax\n";
		m_output << "    shl eax, cl\n";
		m_output << "    push eax\n";
	}
	void m_gen_shr(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    pop eax\n";
		m_output << "    shr eax, cl\n";
		m_output << "    push eax\n";
	}
	void m_gen_true(int ip) {
		m_new_addr(ip);
		m_output << "    push 1\n";
	}
	void m_gen_false(int ip) {
		m_new_addr(ip);
		m_output << "    push 0\n";
	}
	void m_gen_store8(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    pop edx\n";
		m_output << "    mov byte [edx], cl\n";
	}
	void m_gen_load8(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    xor ebx, ebx\n";
		m_output << "    mov bl, byte [ecx]\n";
		m_output << "    push ebx\n";
	}
	void m_gen_store16(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    pop edx\n";
		m_output << "    mov word [edx], cx\n";
	}
	void m_gen_load16(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    xor ebx, ebx\n";
		m_output << "    mov bx, word [ecx]\n";
		m_output << "    push ebx\n";
	}
	void m_gen_store32(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    pop edx\n";
		m_output << "    mov dword [edx], ecx\n";
	}
	void m_gen_load32(int ip) {
		m_new_addr(ip);
		m_output << "    pop ecx\n";
		m_output << "    xor ebx, ebx\n";
		m_output << "    mov ebx, dword [ecx]\n";
		m_output << "    push ebx\n";
	}
	void m_gen_rot(int ip) {
		m_new_addr(ip);
		m_output << "    push dword [esp+8]\n";
	}
	void m_gen_over(int ip) {
		m_new_addr(ip);
		m_output << "    push dword [esp+4]\n";
	}
	void m_gen_swap(int ip) {
		m_new_addr(ip);
		m_output << "    pop eax\n";
		m_output << "    pop ebx\n";
		m_output << "    push eax\n";
		m_output << "    push ebx\n";
	}
	void m_gen_mem(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		if(cur.operand2 == 0) {
			m_output << "    mov ecx, mem\n";
			m_output << "    add ecx, " << m_at(ip).operand1 << "\n";
			m_output << "    push ecx\n";
		}
		else if(cur.operand2 == 1) {
			m_output << "    mov edx, ebp\n";
			m_output << "    sub edx, " << m_at(ip).operand1 + 4 << "\n";
			m_output << "    push edx\n";
		}
		else {
			assert(false); // unreacheable
		}
	}
	void m_gen_push_str(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		std::string svalue = cur.tok.value.value();
		String it = {0};
		bool finded = false;
		for(int i = 0;i < (int)m_strings.size();++i) {
			if(svalue == m_strings[i].data) {
				it = m_strings[i];
				finded = true;
				break;
			}
		}
		if(not finded) {
			int index = (int)m_strings.size();
			m_strings.push_back({ .index = (int)m_strings.size() , .data = svalue });
			m_output << "    push str_" << index << "\n";
			return;
		}
		m_output << "    push str_" << it.index << "\n";
	}
	void m_gen_skip_proc(int ip) {
		m_new_addr(ip);
		m_output << "    jmp addr_" << m_at(ip).operand1 + 1 << "\n";
	}
	void m_gen_call(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		Token tok = cur.tok;
		std::string pname = tok.value.value();
		Procedure proc = proc_lookup(m_procs, pname).value();
		int stack_allign = proc.ins.size();
		int rsize = proc.outs.size();
		m_output << "    call addr_" << proc.ip << "\n";
		m_output << "    add esp, " << stack_allign * 4 << "\n";
		if(rsize == 1) {
			m_output << "    push eax\n";
		}
		else if(rsize == 0) {}
		else if(rsize == 2) {
			m_output << "    push ebx\n";
			m_output << "    push eax\n";
		}
		else if(rsize == 3) {
			m_output << "    push ecx\n";
			m_output << "    push ebx\n";
			m_output << "    push eax\n";
		} else {
			assert(false); // unreacheable
		}
	}
	void m_gen_proc(int ip) {
		m_new_addr(ip);
		std::string pname = m_at(ip).tok.value.value();
		Procedure* proc = proc_lookup_rv(m_procs, pname);
		proc->ip = ip;
		m_output << "    push ebp\n";
		m_output << "    mov ebp, esp\n";
		if(proc->local_mem_cap != 0) {
			m_output << "    sub esp, " << proc->local_mem_cap << "\n";
		}
		for(int i = proc->ins.size() - 1;i > -1;--i) {
			m_output << "    push dword [ebp+" << i * 4 + 8 << "]\n";
		}
	}
	void m_gen_c_call(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		std::string fname = cur.tok.value.value();
		bool finded = false;
		for(int i = 0;i < std_externs.size();++i) {
			if(fname == std_externs[i]) {
				finded = true;
				break;
			}
		}
		if(!finded) {
			std_externs.push_back(fname);
		}
		m_output << "    call " << fname << "\n";
		if(cur.operand1 != 0) {
			m_output << "    add esp, " << cur.operand1 * 4 << "\n";
		}
		m_output << "    push eax\n";
	}
	void m_gen_bind(int ip) {
		m_new_addr(ip);
		m_output << "    pop ebx\n";
		m_output << "    mov dword [mstack+" << m_at(ip).operand2 * 4 << "], ebx\n";
	}
	void m_gen_start(int ip) {
		m_new_addr(ip);
		std::optional<Procedure> mproc = proc_lookup(m_procs, "main");
		if(!mproc.has_value()) {
			std::cout << "ERROR: `main` procedure not found!\n";
			exit(1);
		}
		int OFFSET_END = m_ops->size() - 1;
		m_output << "    jmp addr_" << OFFSET_END << "\n";
	}
	void m_gen_push_bind(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		int offset = cur.operand1 * 4;
		m_output << "    push dword [mstack+" << offset << "]\n";
	}
	std::string generate() {
		m_output << "section .text\n";
		m_output << "\n";
		m_output << "global main\n\n";
		m_output << "main:\n";
		m_output << "    push ebp\n";
		m_output << "    mov ebp, esp\n";
		for(int ip = 0;ip < m_ops->size();++ip) {
			switch(m_at(ip).type) {
				case OP_TYPE::PUSH_INT:
					m_gen_push_int(ip);
					break;
				case OP_TYPE::INTR_PRINT:
					m_gen_print_intrinsic(ip);
					break;
				case OP_TYPE::OPER_ADD:
					m_gen_add_oper(ip);
					break;
				case OP_TYPE::OPER_SUB:
					m_gen_sub_oper(ip);
					break;
				case OP_TYPE::OPER_MUL:
					m_gen_mul_oper(ip);
					break;
				case OP_TYPE::OPER_DIV:
					m_gen_div_oper(ip);
					break;
				case OP_TYPE::OP_IF:
					m_gen_if(ip);
					break;
				case OP_TYPE::OP_END:
					m_gen_end(ip);
					break;
				case OP_TYPE::OP_EXIT:
					m_gen_exit(ip);
					break;
				case OP_TYPE::OP_PROG_END:
					m_gen_prog_end(ip);
					break;
				case OP_TYPE::OP_ELSE:
					m_gen_else(ip);
					break;
				case OP_TYPE::OP_EQ:
					m_gen_eq(ip);
					break;
				case OP_TYPE::OP_DUP:
					m_gen_dup(ip);
					break;
				case OP_TYPE::OP_ABOVE:
					m_gen_above(ip);
					break;
				case OP_TYPE::OP_LESS:
					m_gen_less(ip);
					break;
				case OP_TYPE::OP_DO:
					m_gen_do(ip);
					break;
				case OP_TYPE::OP_WHILE:
					m_gen_while(ip);
					break;
				case OP_TYPE::OP_DROP:
					m_gen_drop(ip);
					break;
				case OP_TYPE::OP_NOT:
					m_gen_not(ip);
					break;
				case OP_TYPE::OP_OR:
					m_gen_or(ip);
					break;
				case OP_TYPE::OP_AND:
					m_gen_and(ip);
					break;
				case OP_TYPE::OP_TRUE:
					m_gen_true(ip);
					break;
				case OP_TYPE::OP_FALSE:
					m_gen_false(ip);
					break;
				case OP_TYPE::OP_STORE8:
					m_gen_store8(ip);
					break;
				case OP_TYPE::OP_LOAD8:
					m_gen_load8(ip);
					break;
				case OP_TYPE::OP_STORE16:
					m_gen_store16(ip);
					break;
				case OP_TYPE::OP_LOAD16:
					m_gen_load16(ip);
					break;
				case OP_TYPE::OP_STORE32:
					m_gen_store32(ip);
					break;
				case OP_TYPE::OP_LOAD32:
					m_gen_load32(ip);
					break;
				case OP_TYPE::OP_2DUP:
					m_gen_2dup(ip);
					break;
				case OP_TYPE::OP_BAND:
					m_gen_band(ip);
					break;
				case OP_TYPE::OP_BOR:
					m_gen_bor(ip);
					break;
				case OP_TYPE::OP_SHL:
					m_gen_shl(ip);
					break;
				case OP_TYPE::OP_SHR:
					m_gen_shr(ip);
					break;
				case OP_TYPE::OP_SWAP:
					m_gen_swap(ip);
					break;
				case OP_TYPE::OP_OVER:
					m_gen_over(ip);
					break;
				case OP_TYPE::OP_ROT:
					m_gen_rot(ip);
					break;
				case OP_TYPE::PUSH_STR:
					m_gen_push_str(ip);
					break;
				case OP_TYPE::OP_MOD:
					m_gen_mod(ip);
					break;
				case OP_TYPE::OP_MEM:
					m_gen_mem(ip);
					break;
				case OP_TYPE::OP_SKIP_PROC:
					m_gen_skip_proc(ip);
					break;
				case OP_TYPE::OP_CALL:
					m_gen_call(ip);
					break;
				case OP_TYPE::OP_PROC:
					m_gen_proc(ip);
					break;
				case OP_TYPE::OP_C_CALL:
					m_gen_c_call(ip);
					break;
				case OP_TYPE::OP_BIND:
					m_gen_bind(ip);
					break;
				case OP_TYPE::OP_START:
					m_gen_start(ip);
					break;
				case OP_TYPE::OP_PUSH_BIND:
					m_gen_push_bind(ip);
					break;
				default:
					GeneratorError("unkown op_type `" + op_to_string(m_at(ip).type) + "`", ip);
			}
		}
		m_output << "\n";
		for(int i = 0;i < std_externs.size();++i) {
			m_output << "extern " << std_externs[i] << "\n";
		}
		m_output << "\n";
		m_output << "\nsection .bss\n";
		m_output << "    mem: resb " << m_memsize << "\n";
		m_output << "    mstack: resb " << MSTACK_CAPACITY << "\n";
		m_output << "    msp: resd 1\n";
		m_output << "\nsection .data\n";
		m_output << "    numfmt: db \"%d\", 0x0\n";
		for(int i = 0;i < (int)m_strings.size();++i) {
			String& cur_s = m_strings[i];
			m_output << "    str_" << cur_s.index << ": db ";
			for(int j = 0;j < cur_s.data.length();++j) {
				m_output << "0x" << std::hex << (int)cur_s.data[j] << ", ";
			}
			m_output << "0x0\n";
		}
		return m_output.str();
	}
};