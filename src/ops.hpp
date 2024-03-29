#pragma once

#include<iostream>
#include<vector>
#include<string>
#include<cassert>

#include "lexer.hpp"

enum class OP_TYPE {
	PUSH_INT,
	PUSH_STR,
	INTR_PRINT,
	OPER_ADD,
	OPER_SUB,
	OPER_MUL,
	OPER_DIV,
	OP_IF,
	OP_END,
	OP_EXIT,
	OP_ELSE,
	OP_EQ,
	OP_DUP,
	OP_ABOVE,
	OP_LESS,
	OP_DO,
	OP_WHILE,
	OP_DROP,
	OP_NOT,
	OP_OR,
	OP_AND,
	OP_FALSE,
	OP_TRUE,
	CAST_INT,
	CAST_BOOL,
	CAST_PTR,
	OP_LOAD8,
	OP_STORE8,
	OP_LOAD16,
	OP_STORE16,
	OP_LOAD32,
	OP_STORE32,
	OP_2DUP,
	OP_BOR,
	OP_BAND,
	OP_SHR,
	OP_SHL,
	OP_SWAP,
	OP_OVER,
	OP_DUMP,
	OP_MOD,
	OP_MEM,
	OP_ROT,
	OP_SKIP_PROC,
	OP_CALL,
	OP_PROC,
	OP_C_CALL,
	OP_PROG_END,
	OP_BIND,
	OP_PUSH_BIND,
	OP_START,
	OP_RETURN,
};

std::string op_to_string(OP_TYPE opt) {
	switch(opt) {
	case OP_TYPE::PUSH_INT:
		return "PUSH_INT";
	case OP_TYPE::INTR_PRINT:
		return "print_intrinsic";
	case OP_TYPE::OPER_ADD:
		return "OP_ADD";
	case OP_TYPE::OPER_SUB:
		return "OP_SUB";
	case OP_TYPE::OPER_MUL:
		return "OP_MUL";
	case OP_TYPE::OPER_DIV:
		return "OP_DIV";
	case OP_TYPE::OP_IF:
		return "OP_IF";
	case OP_TYPE::OP_END:
		return "OP_END";
	case OP_TYPE::OP_EXIT:
		return "OP_EXIT";
	case OP_TYPE::OP_ELSE:
		return "OP_ELSE";
	case OP_TYPE::OP_EQ:
		return "OP_EQ";
	case OP_TYPE::OP_DUP:
		return "OP_DUP";
	case OP_TYPE::OP_ABOVE:
		return "OP_ABOVE";
	case OP_TYPE::OP_LESS:
		return "OP_LESS";
	case OP_TYPE::OP_DO:
		return "OP_DO";
	case OP_TYPE::OP_WHILE:
		return "OP_WHILE";
	case OP_TYPE::OP_DROP:
		return "OP_DROP";
	case OP_TYPE::OP_NOT:
		return "OP_NOT";
	case OP_TYPE::OP_AND:
		return "OP_AND";
	case OP_TYPE::OP_OR:
		return "OP_OR";
	case OP_TYPE::OP_FALSE:
		return "OP_FALSE";
	case OP_TYPE::OP_TRUE:
		return "OP_TRUE";
	case OP_TYPE::CAST_INT:
		return "CAST_INT";
	case OP_TYPE::CAST_BOOL:
		return "CAST_BOOL";
	case OP_TYPE::CAST_PTR:
		return "CAST_PTR";
	case OP_TYPE::OP_LOAD8:
		return "OP_LOAD8";
	case OP_TYPE::OP_STORE8:
		return "OP_STORE8";
	case OP_TYPE::OP_LOAD16:
		return "OP_LOAD16";
	case OP_TYPE::OP_STORE16:
		return "OP_STORE16";
	case OP_TYPE::OP_LOAD32:
		return "OP_LOAD32";
	case OP_TYPE::OP_STORE32:
		return "OP_STORE32";
	case OP_TYPE::OP_2DUP:
		return "OP_2DUP";
	case OP_TYPE::OP_BOR:
		return "OP_BOR";
	case OP_TYPE::OP_BAND:
		return "OP_BAND";
	case OP_TYPE::OP_SHR:
		return "OP_SHR";
	case OP_TYPE::OP_SHL:
		return "OP_SHL";
	case OP_TYPE::OP_OVER:
		return "OP_OVER";
	case OP_TYPE::OP_SWAP:
		return "OP_SWAP";
	case OP_TYPE::OP_DUMP:
		return "OP_DUMP";
	case OP_TYPE::PUSH_STR:
		return "PUSH_STR";
	case OP_TYPE::OP_MOD:
		return "OP_MOD";
	case OP_TYPE::OP_MEM:
		return "OP_MEM";
	case OP_TYPE::OP_ROT:
		return "OP_ROT";
	case OP_TYPE::OP_SKIP_PROC:
		return "OP_SKIP_PROC";
	case OP_TYPE::OP_CALL:
		return "OP_CALL";
	case OP_TYPE::OP_PROC:
		return "OP_PROC";
	case OP_TYPE::OP_C_CALL:
		return "OP_C_CALL";
	case OP_TYPE::OP_PROG_END:
		return "OP_PROG_END";
	case OP_TYPE::OP_BIND:
		return "OP_BIND";
	case OP_TYPE::OP_PUSH_BIND:
		return "OP_PUSH_BIND";
	case OP_TYPE::OP_START:
		return "OP_START";
	case OP_TYPE::OP_RETURN:
		return "OP_RETURN";
	}
	assert(false);
	return "";
}

struct OP {
	int operand1 = 0;
	int operand2 = 0;
	Token tok;
	OP_TYPE type;
	OP(OP_TYPE _type, Token _tok, int oper1, int oper2) {
		this->type = _type;
		this->operand1 = oper1;
		this->operand2 = oper2;
		this->tok = _tok;
	}
	OP(OP_TYPE _type, Token _tok, int oper1) {
		this->type = _type;
		this->operand1 = oper1;
		this->tok = _tok;
	}
	OP(OP_TYPE _type, Token _tok) {
		this->type = _type;
		this->tok = _tok;
	}
	OP_TYPE gettype() {
		return this->type;
	}
	friend std::ostream& operator<<(std::ostream& out, OP& op) {
        out << op_to_string(op.gettype());
        return out;
    }
    friend std::ostream& operator<<(std::ostream& out, OP*& op) {
        out << op_to_string(op->gettype());
        return out;
    }
};

#define ops_list std::vector<OP*>

void free_ops_list(ops_list* list) {
	for(int i = 0;i < (int)list->size();++i) {
		delete list[0][i];
	}
}