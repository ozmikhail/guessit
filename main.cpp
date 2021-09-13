#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "error.hpp"
#include "gradebook.hpp"

static std::vector<std::string> tokenize(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) out.push_back(std::move(tok));
    return out;
}

static bool parseKv(const std::string& tok, std::string& key, double& val) {
    auto eq = tok.find('=');
    if (eq == std::string::npos) return false;
    key = tok.substr(0, eq);
    try {
        val = std::stod(tok.substr(eq + 1));
    } catch (...) { return false; }
    return true;
}

static void printHelp() {
    std::cout <<
        "\nguessit commands:\n"
        "  subject add <name> [max=N] [weight=W]\n"
        "  subject list\n"
        "  subject remove <name>\n"
        "  help               show this message\n"
        "  clear              empty the gradebook\n"
        "  quit / exit        exit\n\n";
}

static void cmdSubjectAdd(Gradebook& book, const std::vector<std::string>& args) {
    if (args.empty())
        throw GradeError("usage: subject add <name> [max=N] [weight=W]");
    Subject sub;
    sub.name = args[0];
    for (std::size_t i = 1; i < args.size(); ++i) {
        std::string key;
        double val = 0.0;
        if (!parseKv(args[i], key, val))
            throw GradeError("expected key=value, got '" + args[i] + "'");
        if (key == "max") sub.maxMarks = val;
        else if (key == "weight") sub.weight = val;
        else if (key == "pass") sub.passMark = val;
        else throw GradeError("unknown subject option '" + key + "'");
    }
    book.addSubject(std::move(sub));
    std::cout << "added subject " << args[0] << '\n';
}

static void cmdSubjectList(const Gradebook& book) {
    if (book.subjects().empty()) {
        std::cout << "(no subjects)\n";
        return;
    }
    std::size_t w = 0;
    for (const auto& [name, _] : book.subjects()) w = std::max(w, name.size());
    std::cout << "  " << std::left << std::setw(static_cast<int>(w))
              << "name" << "   max  weight   pass\n";
    for (const auto& [name, sub] : book.subjects())
        std::cout << "  " << std::left << std::setw(static_cast<int>(w)) << name
                  << "   " << std::setw(3) << sub.maxMarks
                  << "  " << std::setw(5) << sub.weight
                  << "    " << sub.passMark << '\n';
}

static void cmdSubjectRemove(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 1)
        throw GradeError("usage: subject remove <name>");
    book.removeSubject(args[0]);
    std::cout << "removed subject " << args[0] << '\n';
}

static bool dispatchSubject(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "subject") return false;
    if (tokens.size() < 2)
        throw GradeError("usage: subject <add|list|remove> ...");
    std::vector<std::string> args(tokens.begin() + 2, tokens.end());
    const std::string& sub = tokens[1];
    if (sub == "add") cmdSubjectAdd(book, args);
    else if (sub == "list") cmdSubjectList(book);
    else if (sub == "remove") cmdSubjectRemove(book, args);
    else throw GradeError("unknown subject subcommand '" + sub + "'");
    return true;
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

        try {
            auto tokens = tokenize(line);
            if (dispatchSubject(book, tokens)) continue;
            std::cerr << "unknown command: " << tokens[0] << " (type 'help')\n";
        } catch (const std::exception& ex) {
            std::cerr << "error: " << ex.what() << '\n';
        }
    }
}
