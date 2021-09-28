#pragma once
#include "gradebook.hpp"
#include <cstddef>
#include <string>
#include <vector>

struct RankRow {
    std::size_t rank;
    std::string id;
    std::string name;
    double total;
    double maxTotal;
    double percent;
    std::string letter;
    double gpa;
};

std::vector<RankRow> rankList(const Gradebook& book);
