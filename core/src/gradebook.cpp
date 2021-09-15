#include "gradebook.hpp"
#include "error.hpp"

bool Gradebook::hasSubject(const std::string& name) const {
    return m_subjects.find(name) != m_subjects.end();
}

bool Gradebook::hasStudent(const std::string& id) const {
    return m_students.find(id) != m_students.end();
}

void Gradebook::addSubject(Subject sub) {
    if (sub.name.empty())
        throw GradeError("subject name cannot be empty");
    if (sub.maxMarks <= 0.0)
        throw GradeError("subject max marks must be positive");
    if (sub.weight < 0.0)
        throw GradeError("subject weight cannot be negative");
    if (hasSubject(sub.name))
        throw GradeError("subject '" + sub.name + "' already exists");
    m_subjects.emplace(sub.name, std::move(sub));
}

void Gradebook::removeSubject(const std::string& name) {
    auto it = m_subjects.find(name);
    if (it == m_subjects.end())
        throw GradeError("no subject '" + name + "'");
    m_subjects.erase(it);
    for (auto& [id, st] : m_students) st.marks.erase(name);
}

const Subject& Gradebook::subject(const std::string& name) const {
    auto it = m_subjects.find(name);
    if (it == m_subjects.end())
        throw GradeError("no subject '" + name + "'");
    return it->second;
}

void Gradebook::addStudent(Student s) {
    if (s.id.empty())
        throw GradeError("student id cannot be empty");
    if (s.name.empty())
        throw GradeError("student name cannot be empty");
    if (hasStudent(s.id))
        throw GradeError("student id '" + s.id + "' already exists");
    m_students.emplace(s.id, std::move(s));
}

void Gradebook::removeStudent(const std::string& id) {
    if (m_students.erase(id) == 0)
        throw GradeError("no student '" + id + "'");
}

void Gradebook::renameStudent(const std::string& id, const std::string& newName) {
    if (newName.empty())
        throw GradeError("student name cannot be empty");
    auto it = m_students.find(id);
    if (it == m_students.end())
        throw GradeError("no student '" + id + "'");
    it->second.name = newName;
}

const Student& Gradebook::student(const std::string& id) const {
    auto it = m_students.find(id);
    if (it == m_students.end())
        throw GradeError("no student '" + id + "'");
    return it->second;
}

Student& Gradebook::student(const std::string& id) {
    auto it = m_students.find(id);
    if (it == m_students.end())
        throw GradeError("no student '" + id + "'");
    return it->second;
}
