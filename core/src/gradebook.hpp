#pragma once
#include "student.hpp"
#include <map>
#include <string>

class Gradebook {
public:
    const std::map<std::string, Subject>& subjects() const { return m_subjects; }
    const std::map<std::string, Student>& students() const { return m_students; }
    std::map<std::string, Subject>& subjects() { return m_subjects; }
    std::map<std::string, Student>& students() { return m_students; }

    bool hasSubject(const std::string& name) const;
    bool hasStudent(const std::string& id) const;
    bool empty() const { return m_subjects.empty() && m_students.empty(); }

private:
    std::map<std::string, Subject> m_subjects;
    std::map<std::string, Student> m_students;
};
