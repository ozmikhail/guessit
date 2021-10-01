# guessit

> A keyboard-first gradebook for teachers — add students, mark them, and ask "who's failing math?" without leaving the terminal.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)

**guessit is a small gradebook REPL written in C++.** It models subjects, students, marks, and a configurable A–F grade scale, and surfaces per-class and per-subject statistics on demand. Aimed at teachers and TAs who want a single-binary tool rather than a spreadsheet.

## Features

- Subject and student CRUD with duplicate guards
- Per-student marks with bounds checking and class/subject totals
- Configurable `GradeScale` with default A–F bands and per-student lookup
- Class- and per-subject statistics (mean, median, spread)
- Competition-style ranking with tie handling

## Quick Start

```bash
git clone https://github.com/ozmikhail/guessit.git
cd guessit

cmake -B build
cmake --build build

./build/guessit
```

## Directory design conventions

**Philosophy:**
    * **Feature / Domain oriented** over technical layering
    * **High cohesion, low coupling** between modules
    * Clear separation between:
      - Domain types (student, subject, grades)
      - Aggregation (gradebook, ranking, stats)
      - Presentation (REPL in `main.cpp`)
    * Easy navigation and testability
    * Scalable from a single binary to a richer toolchain

```bash
.
├── CMakeLists.txt
├── CMakePresets.json
├── LICENSE
├── README.md
├── .gitattributes
├── .gitignore
├── main.cpp                       # REPL entry point
└── core/
    ├── CMakeLists.txt
    └── src/
        ├── error.hpp              # GradeError types
        ├── student.hpp            # Student model
        ├── grades.hpp / .cpp      # GradeScale + bands
        ├── gradebook.hpp / .cpp   # Subjects, students, marks
        ├── ranking.hpp / .cpp     # Rank with tie handling
        └── stats.hpp / .cpp       # Class + per-subject stats
```
