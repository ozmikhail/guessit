#pragma once
#include <map>
#include <string>

struct Subject {
    std::string name;
    double maxMarks{100.0};
    double weight{1.0};
    double passMark{40.0};
};

struct Student {
    std::string id;
    std::string name;
    std::map<std::string, double> marks;
};
