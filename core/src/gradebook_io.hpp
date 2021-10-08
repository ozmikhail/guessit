#pragma once
#include "gradebook.hpp"
#include <cstddef>
#include <iosfwd>
#include <string>

struct LoadReport {
    std::size_t directives;
    std::size_t errors;
};

void saveGradebook(const Gradebook& book, const std::string& path);
LoadReport loadGradebook(Gradebook& book, const std::string& path, std::ostream& err);
