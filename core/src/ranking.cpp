#include "ranking.hpp"
#include <algorithm>

std::vector<RankRow> rankList(const Gradebook& book) {
    std::vector<RankRow> rows;
    rows.reserve(book.students().size());
    double mx = book.maxTotal();
    for (const auto& [id, st] : book.students()) {
        RankRow r;
        r.id = id;
        r.name = st.name;
        r.total = book.totalFor(id);
        r.maxTotal = mx;
        r.percent = book.percentFor(id);
        r.letter = book.letterFor(id);
        r.gpa = book.gpaFor(id);
        rows.push_back(std::move(r));
    }
    std::sort(rows.begin(), rows.end(), [](const RankRow& a, const RankRow& b) {
        if (a.percent != b.percent) return a.percent > b.percent;
        return a.name < b.name;
    });
    std::size_t rank = 0;
    double prev = -1.0;
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (i == 0 || rows[i].percent != prev) rank = i + 1;
        rows[i].rank = rank;
        prev = rows[i].percent;
    }
    return rows;
}
