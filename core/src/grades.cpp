#include "grades.hpp"
#include "error.hpp"
#include <algorithm>

GradeScale::GradeScale() { reset(); }

void GradeScale::reset() {
    m_bands = {
        {"A", 90.0, 4.0},
        {"B", 80.0, 3.0},
        {"C", 70.0, 2.0},
        {"D", 60.0, 1.0},
        {"F", 0.0, 0.0},
    };
    sortDesc();
}

void GradeScale::set(const std::string& letter, double minPct, double gpa) {
    if (letter.empty())
        throw GradeError("grade letter cannot be empty");
    if (minPct < 0.0 || minPct > 100.0)
        throw GradeError("grade minPct must be in [0, 100]");
    if (gpa < 0.0)
        throw GradeError("grade gpa cannot be negative");
    for (auto& b : m_bands) {
        if (b.letter == letter) {
            b.minPct = minPct;
            b.gpa = gpa;
            sortDesc();
            return;
        }
    }
    m_bands.push_back({letter, minPct, gpa});
    sortDesc();
}

void GradeScale::remove(const std::string& letter) {
    auto it = std::find_if(m_bands.begin(), m_bands.end(),
                           [&](const GradeBand& b) { return b.letter == letter; });
    if (it == m_bands.end())
        throw GradeError("no grade '" + letter + "'");
    m_bands.erase(it);
}

const std::string& GradeScale::letterFor(double percent) const {
    static const std::string none = "-";
    for (const auto& b : m_bands)
        if (percent >= b.minPct) return b.letter;
    return none;
}

double GradeScale::gpaFor(double percent) const {
    for (const auto& b : m_bands)
        if (percent >= b.minPct) return b.gpa;
    return 0.0;
}

void GradeScale::sortDesc() {
    std::sort(m_bands.begin(), m_bands.end(),
              [](const GradeBand& a, const GradeBand& b) { return a.minPct > b.minPct; });
}
