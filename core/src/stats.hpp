#pragma once
#include "gradebook.hpp"
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

struct ClassStats {
    std::size_t count;
    double meanPct;
    double medianPct;
    double stddevPct;
    double minPct;
    double maxPct;
    double passRate;
    std::string topId;
    std::string bottomId;
};

struct SubjectStats {
    std::size_t count;
    double mean;
    double median;
    double stddev;
    double min;
    double max;
    double passRate;
    double maxMarks;
    double passMark;
};

ClassStats classStats(const Gradebook& book);
ClassStats sectionStats(const Gradebook& book, const std::string& section);
SubjectStats subjectStats(const Gradebook& book, const std::string& subjectName);
std::string topperOf(const Gradebook& book, const std::string& subjectName);
std::vector<std::pair<std::string, std::size_t>> gradeHistogram(const Gradebook& book);
bool studentPasses(const Gradebook& book, const std::string& id);
