#include <iostream>
#include <cstring>
#include <string>
#include <filesystem>
#include <fstream>

#include "Lexer.h"
#include "Parser.h"

void print_usage() {
    std::cout << "<The Mozart Programming Language Compiler>" << std::endl << std::endl;
    std::cout << "Usage:" << std::endl << std::endl;
    std::cout << "Tokenize(with lexer) a source file into json:" << std::endl
        << "mozart t <source_file> [destination_file]" << std::endl << std::endl;
    std::cout << "Parse(with parser) and construct AST into json:" << std::endl
        << "mozart p <tokens_json_file> [destination_file]" << std::endl << std::endl;
}

int main(int args_num, char **args) {
    if (args_num >= 3) {
        if (std::strcmp(args[1], "t") == 0) {

            std::cout << "Tokenizing <<" << args[2] << ">>..." << std::endl;

            // Opening & reading source file
            std::filesystem::path file_path = args[2];

            if (!std::filesystem::exists(file_path)) {
                std::cout << "File <<" << file_path << ">> doesn't exist!" << std::endl;
                return -1;
            }

            std::ifstream file {file_path};
            std::string source_text;
            source_text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            // Tokenizing its content
            Lexer lexer(source_text);
            lexer.tokenize();

            // Opening output file
            std::ofstream out_file{"mozart.tokens.json"};
            out_file << lexer.serialize_to_json();
            out_file.close();

        } else if (std::strcmp(args[1], "p") == 0) {

            std::cout << "Parsing <<" << args[2] << ">>..." << std::endl;

            std::filesystem::path file_path = args[2];

            if (!std::filesystem::exists(file_path)) {
                std::cerr << "File <<" << file_path << ">> doesn't exist!" << std::endl;
                return -1;
            }

            std::ifstream file {file_path};
            std::string file_text;

            file_text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

            Lexer lexer{};
            lexer.load_from_json_str(file_text);

            Parser parser{lexer.tokens};
            parser.parse_program();
            assert(true);

        } else {
            print_usage();
        }
    } else {
        print_usage();
    }
}