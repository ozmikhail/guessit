#include "gradebook.hpp"
#include "error.hpp"
#include <algorithm>

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

void Gradebook::setSection(const std::string& id, const std::string& section) {
    student(id).section = section;
}

void Gradebook::setAttendance(const std::string& id, double percent) {
    if (percent < 0.0 || percent > 100.0)
        throw GradeError("attendance must be in [0, 100]");
    student(id).attendance = percent;
}

std::vector<std::string> Gradebook::sections() const {
    std::vector<std::string> out;
    for (const auto& [id, st] : m_students) {
        if (st.section.empty()) continue;
        if (std::find(out.begin(), out.end(), st.section) == out.end())
            out.push_back(st.section);
    }
    std::sort(out.begin(), out.end());
    return out;
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

void Gradebook::setMark(const std::string& id, const std::string& subjectName, double score) {
    const Subject& sub = subject(subjectName);
    if (score < 0.0 || score > sub.maxMarks)
        throw GradeError("score " + std::to_string(score) + " out of range [0, "
                         + std::to_string(sub.maxMarks) + "] for " + subjectName);
    student(id).marks[subjectName] = score;
}

void Gradebook::clearMark(const std::string& id, const std::string& subjectName) {
    if (!hasSubject(subjectName))
        throw GradeError("no subject '" + subjectName + "'");
    student(id).marks.erase(subjectName);
}

double Gradebook::totalFor(const std::string& id) const {
    const Student& s = student(id);
    double sum = 0.0;
    for (const auto& [name, sub] : m_subjects) {
        auto it = s.marks.find(name);
        if (it != s.marks.end()) sum += it->second;
    }
    return sum;
}

double Gradebook::maxTotal() const {
    double sum = 0.0;
    for (const auto& [_, sub] : m_subjects) sum += sub.maxMarks;
    return sum;
}

double Gradebook::percentFor(const std::string& id) const {
    const Student& s = student(id);
    double weighted = 0.0, totalWeight = 0.0;
    for (const auto& [name, sub] : m_subjects) {
        if (sub.maxMarks <= 0.0 || sub.weight <= 0.0) continue;
        auto it = s.marks.find(name);
        double pct = (it == s.marks.end()) ? 0.0 : it->second / sub.maxMarks * 100.0;
        weighted += pct * sub.weight;
        totalWeight += sub.weight;
    }
    return totalWeight > 0.0 ? weighted / totalWeight : 0.0;
}

const std::string& Gradebook::letterFor(const std::string& id) const {
    return m_scale.letterFor(percentFor(id));
}

double Gradebook::gpaFor(const std::string& id) const {
    const Student& s = student(id);
    double weighted = 0.0, totalWeight = 0.0;
    for (const auto& [name, sub] : m_subjects) {
        if (sub.weight <= 0.0) continue;
        auto it = s.marks.find(name);
        double pct = (it == s.marks.end())
            ? 0.0
            : (sub.maxMarks > 0.0 ? it->second / sub.maxMarks * 100.0 : 0.0);
        weighted += m_scale.gpaFor(pct) * sub.weight;
        totalWeight += sub.weight;
    }
    return totalWeight > 0.0 ? weighted / totalWeight : 0.0;
}

const std::string& Gradebook::letterInSubject(const std::string& id, const std::string& subjectName) const {
    static const std::string none = "-";
    const Student& s = student(id);
    const Subject& sub = subject(subjectName);
    auto it = s.marks.find(subjectName);
    if (it == s.marks.end()) return none;
    double pct = sub.maxMarks > 0.0 ? it->second / sub.maxMarks * 100.0 : 0.0;
    return m_scale.letterFor(pct);
}
