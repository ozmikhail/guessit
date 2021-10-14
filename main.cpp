#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "error.hpp"
#include "gradebook.hpp"
#include "gradebook_io.hpp"
#include "ranking.hpp"
#include "stats.hpp"

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

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
        "  subject add <name> [max=N] [weight=W] [pass=P]\n"
        "  subject list\n"
        "  subject remove <name>\n"
        "  student add <id> <name...>\n"
        "  student list\n"
        "  student remove <id>\n"
        "  student rename <id> <new name...>\n"
        "  mark <id> <subject> <score>\n"
        "  unmark <id> <subject>\n"
        "  marks <id>                          show all marks for a student\n"
        "  marks <id> <s1=score> [s2=...]      bulk record marks\n"
        "  grades show\n"
        "  grades set <letter> <minPct> [gpa=G]\n"
        "  grades remove <letter>\n"
        "  grades reset\n"
        "  report <id>                         per-subject letters, total, GPA\n"
        "  rank                                full ranked list\n"
        "  top <N>                             top N by percent\n"
        "  bottom <N>                          bottom N by percent\n"
        "  stats                               class statistics\n"
        "  stats <subject>                     per-subject statistics\n"
        "  topper <subject>                    best student in subject\n"
        "  histogram                           ascii grade distribution\n"
        "  find <substring>                    fuzzy id/name match\n"
        "  sort <key> [asc|desc]               key: name id total percent gpa\n"
        "  filter grade <letter>\n"
        "  filter passed | filter failed\n"
        "  filter percent <op> <value>         op: >= > <= < ==\n"
        "  filter score <subject> <op> <value>\n"
        "  save <file>        write the gradebook to file\n"
        "  load <file>        merge directives from file\n"
        "  history            list previously entered commands\n"
        "  !N                 re-run history entry N (1-based)\n"
        "  !!                 re-run the most recent command\n"
        "  help               show this message\n"
        "  clear              empty the gradebook\n"
        "  quit / exit        exit\n\n";
}

static std::string joinFrom(const std::vector<std::string>& a, std::size_t start) {
    std::string s;
    for (std::size_t i = start; i < a.size(); ++i) {
        if (i > start) s.push_back(' ');
        s += a[i];
    }
    return s;
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

static void cmdStudentAdd(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() < 2)
        throw GradeError("usage: student add <id> <name...>");
    Student s;
    s.id = args[0];
    s.name = joinFrom(args, 1);
    book.addStudent(std::move(s));
    std::cout << "added student " << args[0] << '\n';
}

static void cmdStudentList(const Gradebook& book) {
    if (book.students().empty()) {
        std::cout << "(no students)\n";
        return;
    }
    std::size_t wid = 2, wname = 4;
    for (const auto& [id, st] : book.students()) {
        wid = std::max(wid, id.size());
        wname = std::max(wname, st.name.size());
    }
    std::cout << "  " << std::left << std::setw(static_cast<int>(wid)) << "id"
              << "  " << std::setw(static_cast<int>(wname)) << "name" << '\n';
    for (const auto& [id, st] : book.students())
        std::cout << "  " << std::left << std::setw(static_cast<int>(wid)) << id
                  << "  " << std::setw(static_cast<int>(wname)) << st.name << '\n';
}

static void cmdStudentRemove(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 1)
        throw GradeError("usage: student remove <id>");
    book.removeStudent(args[0]);
    std::cout << "removed student " << args[0] << '\n';
}

static void cmdStudentRename(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() < 2)
        throw GradeError("usage: student rename <id> <new name...>");
    book.renameStudent(args[0], joinFrom(args, 1));
    std::cout << "renamed " << args[0] << " to " << joinFrom(args, 1) << '\n';
}

static bool dispatchStudent(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "student") return false;
    if (tokens.size() < 2)
        throw GradeError("usage: student <add|list|remove|rename> ...");
    std::vector<std::string> args(tokens.begin() + 2, tokens.end());
    const std::string& sub = tokens[1];
    if (sub == "add") cmdStudentAdd(book, args);
    else if (sub == "list") cmdStudentList(book);
    else if (sub == "remove") cmdStudentRemove(book, args);
    else if (sub == "rename") cmdStudentRename(book, args);
    else throw GradeError("unknown student subcommand '" + sub + "'");
    return true;
}

static std::string fmtScore(double v) {
    std::ostringstream oss;
    if (v == std::floor(v)) oss << static_cast<long long>(v);
    else oss << std::setprecision(6) << v;
    return oss.str();
}

static void cmdMark(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 3)
        throw GradeError("usage: mark <id> <subject> <score>");
    double score = 0.0;
    try { score = std::stod(args[2]); }
    catch (...) { throw GradeError("invalid score '" + args[2] + "'"); }
    book.setMark(args[0], args[1], score);
    std::cout << "recorded " << args[1] << " = " << fmtScore(score)
              << " for " << args[0] << '\n';
}

static void cmdUnmark(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 2)
        throw GradeError("usage: unmark <id> <subject>");
    book.clearMark(args[0], args[1]);
    std::cout << "cleared " << args[1] << " for " << args[0] << '\n';
}

static void cmdMarksShow(const Gradebook& book, const std::string& id) {
    const Student& s = book.student(id);
    std::cout << s.id << "  " << s.name << '\n';
    if (book.subjects().empty()) {
        std::cout << "  (no subjects)\n";
        return;
    }
    std::size_t w = 0;
    for (const auto& [name, _] : book.subjects()) w = std::max(w, name.size());
    for (const auto& [name, sub] : book.subjects()) {
        auto it = s.marks.find(name);
        std::cout << "  " << std::left << std::setw(static_cast<int>(w)) << name << "  ";
        if (it == s.marks.end()) std::cout << "-";
        else std::cout << fmtScore(it->second) << " / " << fmtScore(sub.maxMarks);
        std::cout << '\n';
    }
    std::cout << "  total " << fmtScore(book.totalFor(id))
              << " / " << fmtScore(book.maxTotal())
              << "  (" << std::fixed << std::setprecision(2) << book.percentFor(id) << "%)\n";
    std::cout.unsetf(std::ios::fixed);
}

static void cmdMarksBulk(Gradebook& book, const std::vector<std::string>& args) {
    const std::string& id = args[0];
    std::size_t set = 0;
    for (std::size_t i = 1; i < args.size(); ++i) {
        std::string subject;
        double score = 0.0;
        if (!parseKv(args[i], subject, score))
            throw GradeError("expected subject=score, got '" + args[i] + "'");
        book.setMark(id, subject, score);
        ++set;
    }
    std::cout << "recorded " << set << " mark(s) for " << id << '\n';
}

static bool dispatchMarks(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "marks") return false;
    if (tokens.size() < 2)
        throw GradeError("usage: marks <id> [<s1=score> ...]");
    if (tokens.size() == 2) {
        cmdMarksShow(book, tokens[1]);
    } else {
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        cmdMarksBulk(book, args);
    }
    return true;
}

static void cmdGradesShow(const Gradebook& book) {
    const auto& bands = book.scale().bands();
    std::cout << "  letter   minPct   gpa\n";
    for (const auto& b : bands)
        std::cout << "  " << std::left << std::setw(6) << b.letter
                  << "   " << std::setw(6) << b.minPct
                  << "   " << b.gpa << '\n';
}

static void cmdGradesSet(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() < 2)
        throw GradeError("usage: grades set <letter> <minPct> [gpa=G]");
    const std::string& letter = args[0];
    double minPct = 0.0;
    try { minPct = std::stod(args[1]); }
    catch (...) { throw GradeError("invalid minPct '" + args[1] + "'"); }

    double gpa = 0.0;
    bool gpaSet = false;
    for (const auto& b : book.scale().bands())
        if (b.letter == letter) { gpa = b.gpa; gpaSet = true; break; }

    for (std::size_t i = 2; i < args.size(); ++i) {
        std::string key;
        double val = 0.0;
        if (!parseKv(args[i], key, val))
            throw GradeError("expected key=value, got '" + args[i] + "'");
        if (key == "gpa") { gpa = val; gpaSet = true; }
        else throw GradeError("unknown grade option '" + key + "'");
    }
    if (!gpaSet) gpa = 0.0;

    book.scale().set(letter, minPct, gpa);
    std::cout << "set grade " << letter << " >= " << minPct << "% (gpa " << gpa << ")\n";
}

static void cmdGradesRemove(Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 1)
        throw GradeError("usage: grades remove <letter>");
    book.scale().remove(args[0]);
    std::cout << "removed grade " << args[0] << '\n';
}

static void cmdGradesReset(Gradebook& book) {
    book.scale().reset();
    std::cout << "grade scale reset to default\n";
}

static bool dispatchGrades(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "grades") return false;
    if (tokens.size() < 2)
        throw GradeError("usage: grades <show|set|remove|reset> ...");
    std::vector<std::string> args(tokens.begin() + 2, tokens.end());
    const std::string& sub = tokens[1];
    if (sub == "show") cmdGradesShow(book);
    else if (sub == "set") cmdGradesSet(book, args);
    else if (sub == "remove") cmdGradesRemove(book, args);
    else if (sub == "reset") cmdGradesReset(book);
    else throw GradeError("unknown grades subcommand '" + sub + "'");
    return true;
}

static void cmdReport(const Gradebook& book, const std::vector<std::string>& args) {
    if (args.size() != 1)
        throw GradeError("usage: report <id>");
    const std::string& id = args[0];
    const Student& s = book.student(id);
    std::cout << s.id << "  " << s.name << '\n';
    if (book.subjects().empty()) {
        std::cout << "  (no subjects)\n";
        return;
    }
    std::size_t w = 0;
    for (const auto& [name, _] : book.subjects()) w = std::max(w, name.size());
    for (const auto& [name, sub] : book.subjects()) {
        auto it = s.marks.find(name);
        std::cout << "  " << std::left << std::setw(static_cast<int>(w)) << name << "  ";
        if (it == s.marks.end()) {
            std::cout << "-\n";
        } else {
            std::cout << fmtScore(it->second) << " / " << fmtScore(sub.maxMarks)
                      << "  " << book.letterInSubject(id, name) << '\n';
        }
    }
    std::cout << "  total " << fmtScore(book.totalFor(id))
              << " / " << fmtScore(book.maxTotal())
              << "  (" << std::fixed << std::setprecision(2) << book.percentFor(id) << "%)"
              << "  grade " << book.letterFor(id)
              << "  gpa " << std::setprecision(2) << book.gpaFor(id) << '\n';
    std::cout.unsetf(std::ios::fixed);
}

static bool dispatchReport(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "report") return false;
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    cmdReport(book, args);
    return true;
}

static void printRankRows(const std::vector<RankRow>& rows) {
    if (rows.empty()) {
        std::cout << "(no students)\n";
        return;
    }
    std::size_t wid = 2, wname = 4;
    for (const auto& r : rows) {
        wid = std::max(wid, r.id.size());
        wname = std::max(wname, r.name.size());
    }
    std::cout << "  " << std::left << std::setw(4) << "rank"
              << "  " << std::setw(static_cast<int>(wid)) << "id"
              << "  " << std::setw(static_cast<int>(wname)) << "name"
              << "    total      %     grade   gpa\n";
    for (const auto& r : rows) {
        std::cout << "  " << std::left << std::setw(4) << r.rank
                  << "  " << std::setw(static_cast<int>(wid)) << r.id
                  << "  " << std::setw(static_cast<int>(wname)) << r.name
                  << "  " << std::right << std::setw(4) << fmtScore(r.total)
                  << " / " << std::setw(4) << fmtScore(r.maxTotal)
                  << "  " << std::fixed << std::setprecision(2) << std::setw(6) << r.percent << "%"
                  << "    " << std::left << std::setw(2) << r.letter
                  << "    " << std::setprecision(2) << r.gpa << '\n';
    }
    std::cout.unsetf(std::ios::fixed);
    std::cout << std::left;
}

static std::size_t parseCount(const std::vector<std::string>& args, const std::string& usage) {
    if (args.size() != 1) throw GradeError(usage);
    long n = 0;
    try { n = std::stol(args[0]); }
    catch (...) { throw GradeError("invalid count '" + args[0] + "'"); }
    if (n <= 0) throw GradeError("count must be positive");
    return static_cast<std::size_t>(n);
}

static bool dispatchRank(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty()) return false;
    const std::string& v = tokens[0];
    if (v != "rank" && v != "top" && v != "bottom") return false;
    auto rows = rankList(book);
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    if (v == "rank") {
        if (!args.empty()) throw GradeError("usage: rank");
        printRankRows(rows);
    } else if (v == "top") {
        std::size_t n = parseCount(args, "usage: top <N>");
        if (n > rows.size()) n = rows.size();
        rows.resize(n);
        printRankRows(rows);
    } else {
        std::size_t n = parseCount(args, "usage: bottom <N>");
        if (n > rows.size()) n = rows.size();
        rows.erase(rows.begin(), rows.end() - static_cast<long>(n));
        printRankRows(rows);
    }
    return true;
}

static void cmdStatsClass(const Gradebook& book) {
    auto s = classStats(book);
    if (s.count == 0) { std::cout << "(no students)\n"; return; }
    std::cout << std::fixed << std::setprecision(2)
              << "  count       " << s.count << '\n'
              << "  mean %      " << s.meanPct << '\n'
              << "  median %    " << s.medianPct << '\n'
              << "  stddev %    " << s.stddevPct << '\n'
              << "  min %       " << s.minPct << "  (" << s.bottomId << ")\n"
              << "  max %       " << s.maxPct << "  (" << s.topId << ")\n"
              << "  pass rate   " << s.passRate << "%\n";
    std::cout.unsetf(std::ios::fixed);
}

static void cmdStatsSubject(const Gradebook& book, const std::string& name) {
    auto s = subjectStats(book, name);
    if (s.count == 0) { std::cout << "(no marks recorded for " << name << ")\n"; return; }
    std::cout << std::fixed << std::setprecision(2)
              << "  subject     " << name << "  (max=" << s.maxMarks
              << "  pass=" << s.passMark << ")\n"
              << "  count       " << s.count << '\n'
              << "  mean        " << s.mean << '\n'
              << "  median      " << s.median << '\n'
              << "  stddev      " << s.stddev << '\n'
              << "  min         " << s.min << '\n'
              << "  max         " << s.max << '\n'
              << "  pass rate   " << s.passRate << "%\n";
    std::cout.unsetf(std::ios::fixed);
}

static bool dispatchStats(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "stats") return false;
    if (tokens.size() == 1) cmdStatsClass(book);
    else if (tokens.size() == 2) cmdStatsSubject(book, tokens[1]);
    else throw GradeError("usage: stats [<subject>]");
    return true;
}

static bool dispatchTopper(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "topper") return false;
    if (tokens.size() != 2) throw GradeError("usage: topper <subject>");
    std::string id = topperOf(book, tokens[1]);
    if (id.empty()) {
        std::cout << "(no marks recorded for " << tokens[1] << ")\n";
    } else {
        const Student& s = book.student(id);
        double score = s.marks.at(tokens[1]);
        std::cout << "topper of " << tokens[1] << ": " << id << " " << s.name
                  << "  (" << fmtScore(score) << "/"
                  << fmtScore(book.subject(tokens[1]).maxMarks) << ")\n";
    }
    return true;
}

static bool dispatchHistogram(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "histogram") return false;
    if (tokens.size() != 1) throw GradeError("usage: histogram");
    auto rows = gradeHistogram(book);
    std::size_t total = 0;
    std::size_t peak = 0;
    for (const auto& [_, n] : rows) {
        total += n;
        if (n > peak) peak = n;
    }
    if (total == 0) { std::cout << "(no students)\n"; return true; }
    const std::size_t barMax = 30;
    for (const auto& [letter, n] : rows) {
        std::size_t bar = peak == 0 ? 0 : (n * barMax + peak - 1) / peak;
        std::cout << "  " << std::left << std::setw(3) << letter
                  << " " << std::right << std::setw(3) << n << "  "
                  << std::string(bar, '#') << '\n';
    }
    std::cout << std::left;
    return true;
}

static std::string toLower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

static bool dispatchSort(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "sort") return false;
    if (tokens.size() < 2 || tokens.size() > 3)
        throw GradeError("usage: sort <key> [asc|desc]");
    const std::string& key = tokens[1];

    bool asc = (key == "name" || key == "id");
    if (tokens.size() == 3) {
        if (tokens[2] == "asc") asc = true;
        else if (tokens[2] == "desc") asc = false;
        else throw GradeError("expected asc or desc, got '" + tokens[2] + "'");
    }

    auto rows = rankList(book);
    auto cmp = [&](const RankRow& a, const RankRow& b) {
        if (key == "name") return asc ? a.name < b.name : a.name > b.name;
        else if (key == "id") return asc ? a.id < b.id : a.id > b.id;
        else if (key == "total") return asc ? a.total < b.total : a.total > b.total;
        else if (key == "percent") return asc ? a.percent < b.percent : a.percent > b.percent;
        else if (key == "gpa") return asc ? a.gpa < b.gpa : a.gpa > b.gpa;
        throw GradeError("unknown sort key '" + key + "'");
    };
    std::sort(rows.begin(), rows.end(), cmp);
    printRankRows(rows);
    return true;
}

static bool applyOp(const std::string& op, double a, double b) {
    if (op == ">=") return a >= b;
    if (op == ">") return a > b;
    if (op == "<=") return a <= b;
    if (op == "<") return a < b;
    if (op == "==") return a == b;
    throw GradeError("unknown comparison op '" + op + "'");
}

static double parseNumber(const std::string& s) {
    try { return std::stod(s); }
    catch (...) { throw GradeError("invalid number '" + s + "'"); }
}

static bool dispatchSave(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "save") return false;
    if (tokens.size() < 2) throw GradeError("usage: save <file>");
    std::string path = joinFrom(tokens, 1);
    saveGradebook(book, path);
    std::cout << "saved " << book.subjects().size() << " subject(s), "
              << book.students().size() << " student(s) to " << path << '\n';
    return true;
}

static bool dispatchLoad(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "load") return false;
    if (tokens.size() < 2) throw GradeError("usage: load <file>");
    std::string path = joinFrom(tokens, 1);
    auto rep = loadGradebook(book, path, std::cerr);
    std::cout << "loaded " << rep.directives << " directive(s) from " << path;
    if (rep.errors) std::cout << "  (" << rep.errors << " error(s))";
    std::cout << '\n';
    return true;
}

static bool dispatchFilter(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "filter") return false;
    if (tokens.size() < 2) throw GradeError("usage: filter <grade|passed|failed|percent|score> ...");

    auto rows = rankList(book);
    std::vector<RankRow> hits;
    const std::string& which = tokens[1];

    if (which == "grade") {
        if (tokens.size() != 3) throw GradeError("usage: filter grade <letter>");
        for (const auto& r : rows) if (r.letter == tokens[2]) hits.push_back(r);
    } else if (which == "passed" || which == "failed") {
        if (tokens.size() != 2) throw GradeError("usage: filter " + which);
        bool wantPassed = (which == "passed");
        for (const auto& r : rows)
            if (studentPasses(book, r.id) == wantPassed) hits.push_back(r);
    } else if (which == "percent") {
        if (tokens.size() != 4) throw GradeError("usage: filter percent <op> <value>");
        double v = parseNumber(tokens[3]);
        for (const auto& r : rows) if (applyOp(tokens[2], r.percent, v)) hits.push_back(r);
    } else if (which == "score") {
        if (tokens.size() != 5) throw GradeError("usage: filter score <subject> <op> <value>");
        const std::string& sname = tokens[2];
        book.subject(sname);
        double v = parseNumber(tokens[4]);
        for (const auto& r : rows) {
            const auto& m = book.student(r.id).marks;
            auto it = m.find(sname);
            if (it == m.end()) continue;
            if (applyOp(tokens[3], it->second, v)) hits.push_back(r);
        }
    } else {
        throw GradeError("unknown filter kind '" + which + "'");
    }

    if (hits.empty()) std::cout << "(no matches)\n";
    else printRankRows(hits);
    return true;
}

static bool dispatchFind(const Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty() || tokens[0] != "find") return false;
    if (tokens.size() < 2) throw GradeError("usage: find <substring>");
    std::string needle = toLower(joinFrom(tokens, 1));
    auto rows = rankList(book);
    std::vector<RankRow> hits;
    for (const auto& r : rows)
        if (toLower(r.id).find(needle) != std::string::npos
            || toLower(r.name).find(needle) != std::string::npos)
            hits.push_back(r);
    if (hits.empty()) {
        std::cout << "(no matches for '" << joinFrom(tokens, 1) << "')\n";
    } else {
        printRankRows(hits);
    }
    return true;
}

static bool dispatchMark(Gradebook& book, const std::vector<std::string>& tokens) {
    if (tokens.empty()) return false;
    std::vector<std::string> args(tokens.begin() + 1, tokens.end());
    if (tokens[0] == "mark") cmdMark(book, args);
    else if (tokens[0] == "unmark") cmdUnmark(book, args);
    else return false;
    return true;
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

static bool readLine(std::string& line) {
#ifdef HAVE_READLINE
    char* raw = readline("> ");
    if (!raw) return false;
    line = raw;
    if (!line.empty()) add_history(raw);
    free(raw);
    return true;
#else
    std::cout << "> " << std::flush;
    return static_cast<bool>(std::getline(std::cin, line));
#endif
}

static bool resolveBang(std::string& line, const std::vector<std::string>& history) {
    if (line.size() < 2 || line[0] != '!') return true;
    std::size_t idx = 0;
    if (line == "!!") {
        if (history.empty()) { std::cerr << "no history yet\n"; return false; }
        idx = history.size();
    } else {
        try {
            std::size_t consumed = 0;
            long n = std::stol(line.substr(1), &consumed);
            if (consumed + 1 != line.size() || n < 1) {
                std::cerr << "usage: !N or !!  (N is a 1-based history index)\n";
                return false;
            }
            idx = static_cast<std::size_t>(n);
        } catch (...) {
            std::cerr << "usage: !N or !!  (N is a 1-based history index)\n";
            return false;
        }
    }
    if (idx > history.size()) {
        std::cerr << "no history entry " << idx << " (have " << history.size() << ")\n";
        return false;
    }
    line = history[idx - 1];
    std::cout << line << '\n';
    return true;
}

int main() {
    Gradebook book;
    std::vector<std::string> history;

#ifdef HAVE_READLINE
    using_history();
#endif

    std::cout << "guessit — type 'help' for commands or 'quit' to exit\n";

    while (true) {
        std::string line;
        if (!readLine(line)) break;

        if (line.empty()) continue;
        if (line == "quit" || line == "exit") break;

        if (line.size() >= 2 && line[0] == '!') {
            if (!resolveBang(line, history)) continue;
        }

        if (line == "help") { printHelp(); history.push_back(line); continue; }

        if (line == "history") {
            if (history.empty()) std::cout << "(no history)\n";
            else for (std::size_t i = 0; i < history.size(); ++i)
                std::cout << "[" << i + 1 << "] " << history[i] << '\n';
            continue;
        }

        if (line == "clear") {
            book.subjects().clear();
            book.students().clear();
            std::cout << "gradebook cleared\n";
            history.push_back(line);
            continue;
        }

        try {
            auto tokens = tokenize(line);
            bool handled =
                dispatchSubject(book, tokens) || dispatchStudent(book, tokens) ||
                dispatchMark(book, tokens) || dispatchMarks(book, tokens) ||
                dispatchGrades(book, tokens) || dispatchReport(book, tokens) ||
                dispatchRank(book, tokens) || dispatchStats(book, tokens) ||
                dispatchTopper(book, tokens) || dispatchHistogram(book, tokens)||
                dispatchFind(book, tokens) || dispatchSort(book, tokens) ||
                dispatchFilter(book, tokens) || dispatchSave(book, tokens) ||
                dispatchLoad(book, tokens);
            if (!handled) {
                std::cerr << "unknown command: " << tokens[0] << " (type 'help')\n";
                continue;
            }
            history.push_back(line);
        } catch (const std::exception& ex) {
            std::cerr << "error: " << ex.what() << '\n';
        }
    }
}
