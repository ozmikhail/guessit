#include "gradebook.hpp"

bool Gradebook::hasSubject(const std::string& name) const {
    return m_subjects.find(name) != m_subjects.end();
}

bool Gradebook::hasStudent(const std::string& id) const {
    return m_students.find(id) != m_students.end();
}
