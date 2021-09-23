#pragma once
#include <string>
#include <vector>

struct GradeBand {
    std::string letter;
    double minPct;
    double gpa;
};

class GradeScale {
public:
    GradeScale();

    void set(const std::string& letter, double minPct, double gpa);
    void remove(const std::string& letter);
    void reset();

    const std::string& letterFor(double percent) const;
    double gpaFor(double percent) const;

    const std::vector<GradeBand>& bands() const { return m_bands; }

private:
    std::vector<GradeBand> m_bands;
    void sortDesc();
};
