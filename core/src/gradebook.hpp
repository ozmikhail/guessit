#pragma once
#include "grades.hpp"
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

    void addSubject(Subject sub);
    void removeSubject(const std::string& name);
    const Subject& subject(const std::string& name) const;

    void addStudent(Student s);
    void removeStudent(const std::string& id);
    void renameStudent(const std::string& id, const std::string& newName);
    const Student& student(const std::string& id) const;
    Student& student(const std::string& id);

    void setMark(const std::string& id, const std::string& subjectName, double score);
    void clearMark(const std::string& id, const std::string& subjectName);

    double totalFor(const std::string& id) const;
    double maxTotal() const;
    double percentFor(const std::string& id) const;

    const std::string& letterFor(const std::string& id) const;
    double gpaFor(const std::string& id) const;
    const std::string& letterInSubject(const std::string& id, const std::string& subjectName) const;

    GradeScale& scale() { return m_scale; }
    const GradeScale& scale() const { return m_scale; }

private:
    std::map<std::string, Subject> m_subjects;
    std::map<std::string, Student> m_students;
    GradeScale m_scale;
};
