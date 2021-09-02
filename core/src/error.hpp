#pragma once
#include <stdexcept>
#include <string>

class GradeError : public std::runtime_error {
public:
    explicit GradeError(std::string msg)
        : std::runtime_error(std::move(msg)) {}
};
