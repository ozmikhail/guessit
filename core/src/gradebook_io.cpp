#include "gradebook_io.hpp"
#include "error.hpp"
#include <fstream>
#include <ostream>
#include <sstream>

static std::string fmtNum(double v) {
    std::ostringstream oss;
    oss << v;
    return oss.str();
}

void saveGradebook(const Gradebook& book, const std::string& path) {
    std::ofstream out(path);
    if (!out) throw GradeError("cannot open '" + path + "' for writing");
    out << "# guessit save file\n";
    for (const auto& b : book.scale().bands())
        out << "GRADE " << b.letter << ' ' << fmtNum(b.minPct) << ' ' << fmtNum(b.gpa) << '\n';
    for (const auto& [name, sub] : book.subjects())
        out << "SUBJECT " << name << ' ' << fmtNum(sub.maxMarks) << ' '
            << fmtNum(sub.weight) << ' ' << fmtNum(sub.passMark) << '\n';
    for (const auto& [id, st] : book.students())
        out << "STUDENT " << id << ' ' << st.name << '\n';
    for (const auto& [id, st] : book.students()) {
        if (!st.section.empty())
            out << "SECTION " << id << ' ' << st.section << '\n';
        if (st.attendance != 100.0)
            out << "ATTEND " << id << ' ' << fmtNum(st.attendance) << '\n';
    }
    for (const auto& [id, st] : book.students())
        for (const auto& [name, score] : st.marks)
            out << "MARK " << id << ' ' << name << ' ' << fmtNum(score) << '\n';
}

static std::vector<std::string> splitWs(const std::string& s) {
    std::vector<std::string> out;
    std::istringstream iss(s);
    std::string tok;
    while (iss >> tok) out.push_back(std::move(tok));
    return out;
}

static std::string joinFrom(const std::vector<std::string>& a, std::size_t start) {
    std::string s;
    for (std::size_t i = start; i < a.size(); ++i) {
        if (i > start) s.push_back(' ');
        s += a[i];
    }
    return s;
}

static double parseNum(const std::string& s, const std::string& field) {
    try { return std::stod(s); }
    catch (...) { throw GradeError("invalid " + field + " '" + s + "'"); }
}

LoadReport loadGradebook(Gradebook& book, const std::string& path, std::ostream& err) {
    std::ifstream in(path);
    if (!in) throw GradeError("cannot open '" + path + "'");
    LoadReport rep{0, 0};
    std::string line;
    std::size_t lineNo = 0;
    while (std::getline(in, line)) {
        ++lineNo;
        auto toks = splitWs(line);
        if (toks.empty()) continue;
        if (toks[0][0] == '#') continue;
        try {
            const std::string& kind = toks[0];
            if (kind == "GRADE") {
                if (toks.size() != 4) throw GradeError("usage: GRADE <letter> <minPct> <gpa>");
                book.scale().set(toks[1], parseNum(toks[2], "minPct"), parseNum(toks[3], "gpa"));
            } else if (kind == "SUBJECT") {
                if (toks.size() != 5) throw GradeError("usage: SUBJECT <name> <max> <weight> <pass>");
                Subject s;
                s.name = toks[1];
                s.maxMarks = parseNum(toks[2], "max");
                s.weight = parseNum(toks[3], "weight");
                s.passMark = parseNum(toks[4], "pass");
                book.addSubject(std::move(s));
            } else if (kind == "STUDENT") {
                if (toks.size() < 3) throw GradeError("usage: STUDENT <id> <name...>");
                Student st;
                st.id = toks[1];
                st.name = joinFrom(toks, 2);
                book.addStudent(std::move(st));
            } else if (kind == "MARK") {
                if (toks.size() != 4) throw GradeError("usage: MARK <id> <subject> <score>");
                book.setMark(toks[1], toks[2], parseNum(toks[3], "score"));
            } else if (kind == "SECTION") {
                if (toks.size() != 3) throw GradeError("usage: SECTION <id> <name>");
                book.setSection(toks[1], toks[2]);
            } else if (kind == "ATTEND") {
                if (toks.size() != 3) throw GradeError("usage: ATTEND <id> <pct>");
                book.setAttendance(toks[1], parseNum(toks[2], "attendance"));
            } else {
                throw GradeError("unknown directive '" + kind + "'");
            }
            ++rep.directives;
        } catch (const std::exception& ex) {
            err << "  " << path << ":" << lineNo << ": " << ex.what() << '\n';
            ++rep.errors;
        }
    }
    return rep;
}
