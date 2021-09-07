#include <iostream>
#include <string>
#include "gradebook.hpp"

static void printHelp() {
    std::cout <<
        "\nguessit commands:\n"
        "  help               show this message\n"
        "  clear              empty the gradebook\n"
        "  quit / exit        exit\n\n";
}

int main() {
    Gradebook book;

    std::cout << "guessit — type 'help' for commands or 'quit' to exit\n";

    while (true) {
        std::cout << "> " << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (line.empty()) continue;
        if (line == "quit" || line == "exit") break;

        if (line == "help") { printHelp(); continue; }

        if (line == "clear") {
            book.subjects().clear();
            book.students().clear();
            std::cout << "gradebook cleared\n";
            continue;
        }

        std::cerr << "unknown command: " << line << " (type 'help')\n";
    }
}
