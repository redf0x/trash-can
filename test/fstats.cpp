#include <vector>
#include <string>
#include <sstream>
#include <iostream>

using namespace std;

using strings = vector<string>;
using ints = vector<int>;

std::string extract_extension(const std::string& filename)
{
    auto pos = filename.find_last_of('.');

    if (pos == std::string::npos)
        return string{};
    else
        return filename.substr(pos + 1);
}

enum class category : unsigned {
    cat_movies = 0,
    cat_images,
    cat_music,
    cat_other
};

const strings extensions = {
    "mp4/mkv/avi",
    "jpg/bmp/gif",
    "mp3/flac/wav"
};

category get_category(const string& file_name)
{
    category result = category::cat_other;
    size_t idx = 0;

    if (const auto& ext = extract_extension(file_name);
            !ext.empty())
        for (auto& cat : extensions) {
            if (cat.find(ext) != string::npos)
                result = static_cast<category>(idx);
            else
                idx++;
        }

    return result;
}

ostream& operator<<(ostream& s, category cat)
{
    switch (cat) {
        case category::cat_movies:
            s << "movies";
            break;

        case category::cat_images:
            s << "images";
            break;

        case category::cat_music:
            s << "music";
            break;

        case category::cat_other:
            s << "other";
            break;
    }

    return s;
}

strings split(const string& s, char delimiter)
{
    istringstream input_stream(s);
    string token;
    strings result;

    while (getline(input_stream, token, input_stream.widen(delimiter)))
        result.push_back(move(token));

    return result;
}

string chomp(const string& s)
{
    return string(s.begin(), s.end() - 1);
}

int catoi(category cat)
{
    return static_cast<int>(cat);
}

category itocat(size_t i)
{
    return static_cast<category>(i);
}

int main()
{
    string input = {
        "hjhasd.ghsda 12249b\n"
        "explorer.exe 8299b\n"
        "newsong.mp3 10000b\n"
        "thisisnewshit!.flac 200000b\n"
        "movie.mkv 10000b\n"
        "dick.jpg 289b\n"
        "autorun 10b"
    };
    auto lines = split(input, '\n');
    ints counters = { 0, 0, 0, 0 };

    for (const auto& l : lines) {
        const auto& str = split(l, ' ');

        cout << str[0] << endl;

        auto cat = get_category(str[0]);
        auto size = chomp(str[1]);

        cout << "    cat: " << cat << endl;
        cout << "   size: " << size << endl;
        counters[catoi(cat)] += stol(size);

//        const std::regex size_regex("(^[\w,\s-]+)\. ([0-9]+)b");
//        std::smatch base_match;

//        if (std::regex_match(str[1], base_match, size_regex)) {
//            if (base_match.size() == 2) {
//                std::ssub_match base_sub_match = base_match[1];
//                std::string base = base_sub_match.str();
//                std::cout << " has size " << base << '\n';
//            }
//        }
    }

    for (auto& c : counters) {
        auto icat = &c - &counters[0];
        cout << itocat(icat) << ": " << c << endl;
    }
}
