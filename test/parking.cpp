#include <vector>
#include <exception>
#include <algorithm>
#include <iostream>

using namespace std;

using ints = vector<int>;
using range = pair<int, int>;

range find_hole(ints& a)
{
    size_t max_span = 0;
    size_t span = 0;
    ssize_t pos = -1;

    for (size_t i = 0; i < a.size(); i++)
        if (a[i] == 0) {
            span++;

            if (max_span < span) {
                max_span = span;
                pos = i - max_span + 1;
            }
        } else
            span = 0;

    if (pos == -1)
        throw range_error("no hole found");

    return range(pos, max_span);
}

int find_lowest_adjacent(const ints& a, const range& r)
{
    size_t idx;

    if (r.first == 0)
        idx = r.second;
    else {
        if (r.first + r.second == a.size())
            idx = r.first - 1;
        else {
            if (a[r.first - 1] - a[r.first + r.second] <= 0)
                idx = r.first - 1;
            else
                idx = r.first + r.second;
        }
    }

    return idx;
}

int center(const range& r)
{
        size_t result = r.second;

        if (r.second && r.second & 1)
                result /= 2;
        else
                (result /= 2)--;

        return result;
}

int main()
{
    vector<ints> arrays{
        { 1, 0, 0, 3, 3, 2, 1 },
        { 1, 0, 1, 0, 0, 0, 1 },    // 2: longest last
        { 1, 1, 1, 3, 3, 2, 1 },    // 3: no holes
        { 1, 1, 0, 0, 0, 0, 3 },
        { 3, 0, 0, 0, 2, 0, 3 },    // 5: longest first
        { 0, 0, 0, 0, 5, 0, 3 },
        { 4, 5, 0, 0, 0, 0, 0 }
    };
    int seq = 1;

    for (auto& x : arrays) {
        try {
            auto [pos, length] = find_hole(x);

            cout << seq << ": start range: " << pos << ", length: " << length << endl;
            size_t idx = find_lowest_adjacent(x, range(pos, length));
            cout << seq << ": lowest adj: " << x[idx] << " at " << idx << endl;
        } catch(range_error& e) {
            cout << "hole not found\n";
        }

        seq++;
    }

    cout << "center of [0; 3] " << center(range(0, 3)) << endl;
    cout << "center of [0; 6] " << center(range(0, 6)) << endl;
    cout << "center of [0; 0] " << center(range(0, 0)) << endl;
}
