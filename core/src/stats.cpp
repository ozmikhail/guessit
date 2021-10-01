#include "stats.hpp"
#include <algorithm>
#include <cmath>

static double median(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    std::size_t n = v.size();
    return (n % 2) ? v[n / 2] : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

static double stddev(const std::vector<double>& v, double mean) {
    if (v.empty()) return 0.0;
    double s = 0.0;
    for (double x : v) s += (x - mean) * (x - mean);
    return std::sqrt(s / static_cast<double>(v.size()));
}

bool studentPasses(const Gradebook& book, const std::string& id) {
    const Student& s = book.student(id);
    if (book.subjects().empty()) return false;
    for (const auto& [name, sub] : book.subjects()) {
        auto it = s.marks.find(name);
        if (it == s.marks.end()) return false;
        if (it->second < sub.passMark) return false;
    }
    return true;
}

ClassStats classStats(const Gradebook& book) {
    ClassStats out{};
    if (book.students().empty()) return out;

    std::vector<double> pcts;
    pcts.reserve(book.students().size());
    double bestPct = -1.0, worstPct = 1e308;
    std::size_t passed = 0;
    for (const auto& [id, st] : book.students()) {
        double p = book.percentFor(id);
        pcts.push_back(p);
        if (p > bestPct) { bestPct = p; out.topId = id; }
        if (p < worstPct) { worstPct = p; out.bottomId = id; }
        if (studentPasses(book, id)) ++passed;
    }
    out.count = pcts.size();
    double sum = 0.0;
    for (double p : pcts) sum += p;
    out.meanPct = sum / static_cast<double>(out.count);
    out.medianPct = median(pcts);
    out.stddevPct = stddev(pcts, out.meanPct);
    out.minPct = worstPct;
    out.maxPct = bestPct;
    out.passRate = 100.0 * static_cast<double>(passed) / static_cast<double>(out.count);
    return out;
}

SubjectStats subjectStats(const Gradebook& book, const std::string& subjectName) {
    const Subject& sub = book.subject(subjectName);
    SubjectStats out{};
    out.maxMarks = sub.maxMarks;
    out.passMark = sub.passMark;

    std::vector<double> scores;
    scores.reserve(book.students().size());
    std::size_t passed = 0;
    double mn = 1e308, mx = -1e308;
    for (const auto& [id, st] : book.students()) {
        auto it = st.marks.find(subjectName);
        if (it == st.marks.end()) continue;
        scores.push_back(it->second);
        if (it->second >= sub.passMark) ++passed;
        if (it->second < mn) mn = it->second;
        if (it->second > mx) mx = it->second;
    }
    out.count = scores.size();
    if (out.count == 0) return out;
    double sum = 0.0;
    for (double v : scores) sum += v;
    out.mean = sum / static_cast<double>(out.count);
    out.median = median(scores);
    out.stddev = stddev(scores, out.mean);
    out.min = mn;
    out.max = mx;
    out.passRate = 100.0 * static_cast<double>(passed) / static_cast<double>(out.count);
    return out;
}

std::string topperOf(const Gradebook& book, const std::string& subjectName) {
    book.subject(subjectName);
    std::string top;
    double best = -1.0;
    for (const auto& [id, st] : book.students()) {
        auto it = st.marks.find(subjectName);
        if (it == st.marks.end()) continue;
        if (it->second > best || (it->second == best && id < top)) {
            best = it->second;
            top = id;
        }
    }
    return top;
}

std::vector<std::pair<std::string, std::size_t>> gradeHistogram(const Gradebook& book) {
    std::vector<std::pair<std::string, std::size_t>> out;
    for (const auto& b : book.scale().bands())
        out.push_back({b.letter, 0});
    for (const auto& [id, st] : book.students()) {
        const std::string& letter = book.letterFor(id);
        for (auto& row : out)
            if (row.first == letter) { ++row.second; break; }
    }
    return out;
}
