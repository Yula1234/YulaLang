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

struct DataElement {
	DataType type;
	friend std::ostream& operator<<(std::ostream& out, DataElement& de) {
        out << "<" << dt_tostr(de.type) << ">";
        return out;
    }
};

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


const std::vector<const char*> std_externs = {
	"ExitProcess@4",
	"printf",
	"malloc",
	"free"
};


void crossref_check_blocks(ops_list* ops) {
	std::vector<OP*> stack;
	std::vector<int> locs;
	for(int ip = 0;ip < ops->size();++ip) {
		switch((*(ops))[ip]->type) {
			case OP_TYPE::OP_IF:
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
				stack[stack.size() - 1]->operand1 = ip;
				if(stack[stack.size() - 1]->type == OP_TYPE::OP_DO) {
					stack.pop_back();
					cur->operand1 = locs[locs.size() - 1];
					locs.pop_back();
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

void typecheck_program(ops_list* ops) {
	std::vector<DataStack*> scopes;
	scopes.push_back(new DataStack);
	for(int ip = 0;ip < ops->size();++ip) {
		switch((*ops)[ip]->type) {
			case OP_TYPE::PUSH_INT:
				last_scope(scopes).push_back({ .type = DataType::_int });
				break;
			case OP_TYPE::OP_TRUE:
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::OP_FALSE:
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::INTR_PRINT:
				if(!typecheck(last_scope(scopes), 1, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "print excepts types [INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OPER_ADD:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "+ excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OPER_SUB:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "- excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OPER_MUL:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "* excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OPER_DIV:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "/ excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_IF:
				if(!typecheck(last_scope(scopes), 1, DataType::_bool)) {
					TypeError(last_scope(scopes), ip, ops, "if excepts types [BOOL], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_EXIT:
				if(!typecheck(last_scope(scopes), 1, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "at end of the program excepts types [INT], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_EQ:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "= excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::OP_ABOVE:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "> excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::OP_LESS:
				if(!typecheck(last_scope(scopes), 2, DataType::_int, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "< excepts types [INT, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_bool });
				break;
			case OP_TYPE::OP_NOT:
				if(!typecheck(last_scope(scopes), 1, DataType::_bool)) {
					TypeError(last_scope(scopes), ip, ops, "not excepts types [BOOL], but got: ");
				}
				break;
			case OP_TYPE::OP_OR:
				if(!typecheck(last_scope(scopes), 2, DataType::_bool, DataType::_bool)) {
					TypeError(last_scope(scopes), ip, ops, "or excepts types [BOOL, BOOL], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_AND:
				if(!typecheck(last_scope(scopes), 2, DataType::_bool, DataType::_bool)) {
					TypeError(last_scope(scopes), ip, ops, "and excepts types [BOOL, BOOL], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_DUP:
				if(last_scope(scopes).size() < 1) {
					TypeError(last_scope(scopes), ip, ops, "dup excepts 1 element, but got: ");
				}
				last_scope(scopes).push_back({ .type = last_scope(scopes)[last_scope(scopes).size() - 1] });
				break;
			case OP_TYPE::OP_DO:
				if(!typecheck(last_scope(scopes), 1, DataType::_bool)) {
					TypeError(last_scope(scopes), ip, ops, "do except types [BOOL], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_DROP:
				if(last_scope(scopes).size() < 1) {
					TypeError(last_scope(scopes), ip, ops, "drop excepts 1 element, but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::CAST_INT:
				if(last_scope(scopes).size() < 1) {
					TypeError(last_scope(scopes), ip, ops, "cast(int) excepts 1 element, but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_int });
				ops->erase(ops->begin() + ip--);
				break;
			case OP_TYPE::CAST_BOOL:
				if(last_scope(scopes).size() < 1) {
					TypeError(last_scope(scopes), ip, ops, "cast(bool) excepts 1 element, but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_bool });
				ops->erase(ops->begin() + ip--);
				break;
			case OP_TYPE::CAST_PTR:
				if(last_scope(scopes).size() < 1) {
					TypeError(last_scope(scopes), ip, ops, "cast(ptr) excepts 1 element, but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::ptr });
				ops->erase(ops->begin() + ip--);
				break;
			case OP_TYPE::OP_MALLOC:
				if(!typecheck(last_scope(scopes), 1, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "malloc excepts types [INT], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::ptr });
				break;
			case OP_TYPE::OP_FREE:
				if(!typecheck(last_scope(scopes), 1, DataType::ptr)) {
					TypeError(last_scope(scopes), ip, ops, "free excepts types [PTR], but got: ");
				}
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_STORE8:
				if(!typecheck(last_scope(scopes), 2, DataType::ptr, DataType::_int)) {
					TypeError(last_scope(scopes), ip, ops, "!8 excepts types [PTR, INT], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).pop_back();
				break;
			case OP_TYPE::OP_LOAD8:
				if(!typecheck(last_scope(scopes), 1, DataType::ptr)) {
					TypeError(last_scope(scopes), ip, ops, "@8 excepts types [PTR], but got: ");
				}
				last_scope(scopes).pop_back();
				last_scope(scopes).push_back({ .type = DataType::_int });
				break;
		}
	}
}

class Generator {
private:
	ops_list* m_ops;
	std::stringstream m_output;
public:
	explicit Generator(ops_list* _ops) {
		m_ops = _ops;
	}
	OP m_at(int index) {
		return *((*m_ops)[index]);
	}
	void m_new_addr(int ip) {
		m_output << "addr_" << ip << ":";
		OP cur_op = m_at(ip);
		Token tok = cur_op.tok;
		m_output << " ; " << tok.line << ":" << tok.col;
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
		m_output << "    imul eax, ebx\n";
		m_output << "    push eax\n";
		m_output << "    mov edx, ecx\n";
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
	void m_gen_if(int ip) {
		m_new_addr(ip);
		m_output << "    pop edx\n";
		m_output << "    test edx, edx\n";
		m_output << "    jz addr_" << m_at(ip).operand1 << "\n";
	}
	void m_gen_end(int ip) {
		m_new_addr(ip);
		OP cur = m_at(ip);
		if(cur.operand1 != -1) {
			m_output << "    jmp addr_" << cur.operand1 << "\n";
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
	void m_gen_true(int ip) {
		m_new_addr(ip);
		m_output << "    push 1\n";
	}
	void m_gen_false(int ip) {
		m_new_addr(ip);
		m_output << "    push 0\n";
	}
	void m_gen_malloc(int ip) {
		m_new_addr(ip);
		m_output << "    call malloc\n";
		m_output << "    add esp, 4\n";
		m_output << "    push eax\n";
	}
	void m_gen_free(int ip) {
		m_new_addr(ip);
		m_output << "    call free\n";
		m_output << "    add esp, 4\n";
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
	std::string generate() {
		m_output << "section .text\n\n";
		for(int i = 0;i < std_externs.size();++i) {
			m_output << "extern " << std_externs[i] << "\n";
		}
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
				case OP_TYPE::OP_MALLOC:
					m_gen_malloc(ip);
					break;
				case OP_TYPE::OP_FREE:
					m_gen_free(ip);
					break;
				case OP_TYPE::OP_STORE8:
					m_gen_store8(ip);
					break;
				case OP_TYPE::OP_LOAD8:
					m_gen_load8(ip);
					break;
				default:
					GeneratorError("unkown op_type `" + op_to_string(m_at(ip).type) + "`", ip);
			}
		}
		m_output << "\nsection .data\n";
		m_output << "    numfmt: db \"%d\", 10, 0\n";
		return m_output.str();
	}
};