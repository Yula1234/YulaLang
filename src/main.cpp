#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<vector>

#include "ops.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "generator.hpp"


int main(int argc, const char** argv) {
	if (argc != 3) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "yula <input.yula> <output.exe>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
        input.close();
    }
    Lexer lexer(contents);
    std::vector<Token> tokens = lexer.lex();
    Parser parser;
    ops_list* opsl = parser.parse(tokens);
    Token stok;
    stok.line = 1;
    stok.col = 0;
    opsl->insert(opsl->begin(), new OP(OP_TYPE::OP_START, stok));
    typecheck_program(opsl, parser.get_procs());
    crossref_check_blocks(opsl, parser.get_procs());
    Generator generator(opsl);
    generator.set_memsize(parser.get_memsize());
    generator.set_procs(parser.get_procs());
    const std::string out_asm = generator.generate();
    std::fstream file("output.asm", std::ios::out);
    file << out_asm;
    file.close();
    free_ops_list(opsl);
    std::stringstream ld_com;
    ld_com << "gcc output.o -o " << argv[2];
    ld_com << " -m32";
    system("nasm --gprefix _ -fwin32 output.asm -o output.o");
    system(ld_com.str().c_str());
    system("del output.o");
}