/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * Copyright (c) 2014-2015 Kohei Yoshida
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************/

#include "test_global.hpp"
#define MDDS_SORTED_STRING_MAP_DEBUG 1
#include "mdds/sorted_string_map.hpp"
#include "mdds/global.hpp"

#include <cstring>
#include <vector>
#include <fstream>

enum name_type
{
    name_none = 0,
    name_andy,
    name_bruce,
    name_charlie,
    name_david
};

void ssmap_test_basic()
{
    stack_printer __sp__(__func__);

    typedef mdds::sorted_string_map<name_type> map_type;

    map_type::entry entries[] = {
        {MDDS_ASCII("andy"), name_andy},   {MDDS_ASCII("andy1"), name_andy},      {MDDS_ASCII("andy13"), name_andy},
        {MDDS_ASCII("bruce"), name_bruce}, {MDDS_ASCII("charlie"), name_charlie}, {MDDS_ASCII("david"), name_david},
    };

    size_t entry_count = sizeof(entries) / sizeof(entries[0]);
    map_type names(entries, entry_count, name_none);
    for (size_t i = 0; i < entry_count; ++i)
    {
        cout << "* key = " << entries[i].key << endl;
        bool res = names.find(entries[i].key, strlen(entries[i].key)) == entries[i].value;
        assert(res);
    }

    // Try invalid keys.
    assert(names.find("foo", 3) == name_none);
    assert(names.find("andy133", 7) == name_none);
}

void ssmap_test_mixed_case_null()
{
    stack_printer __sp__(__func__);

    typedef mdds::sorted_string_map<int> map_type;

    map_type::entry entries[] = {
        {MDDS_ASCII("NULL"), 1},
        {MDDS_ASCII("Null"), 2},
        {MDDS_ASCII("null"), 3},
        {MDDS_ASCII("~"), 4},
    };

    size_t entry_count = sizeof(entries) / sizeof(entries[0]);
    map_type names(entries, entry_count, -1);
    for (size_t i = 0; i < entry_count; ++i)
    {
        cout << "* key = " << entries[i].key << endl;
        bool res = names.find(entries[i].key, strlen(entries[i].key)) == entries[i].value;
        assert(res);
    }

    // Try invalid keys.
    assert(names.find(MDDS_ASCII("NUll")) == -1);
    assert(names.find(MDDS_ASCII("Oull")) == -1);
    assert(names.find(MDDS_ASCII("Mull")) == -1);
    assert(names.find(MDDS_ASCII("hell")) == -1);
}

void ssmap_test_string_view_entry()
{
    stack_printer __sp__(__func__);

    constexpr int cv_unknown = -1;
    constexpr int cv_days = 0;
    constexpr int cv_hours = 1;
    constexpr int cv_minutes = 2;
    constexpr int cv_months = 3;
    constexpr int cv_quarters = 4;
    constexpr int cv_range = 5;
    constexpr int cv_seconds = 6;
    constexpr int cv_years = 7;

    using map_type = mdds::sorted_string_map<int, mdds::string_view_map_entry>;

    constexpr map_type::entry entries[] = {
        {"days", cv_days},         {"hours", cv_hours}, {"minutes", cv_minutes}, {"months", cv_months},
        {"quarters", cv_quarters}, {"range", cv_range}, {"seconds", cv_seconds}, {"years", cv_years},
    };

    map_type mapping{entries, std::size(entries), cv_unknown};

    for (const auto& entry : entries)
    {
        auto v = mapping.find(entry.key);
        assert(v == entry.value);
    }

    constexpr std::string_view unknown_keys[] = {
        "dayss", "Days", "ddays", "adfsd", "secoonds", "years ",
    };

    for (const auto& key : unknown_keys)
    {
        auto v = mapping.find(key);
        assert(v == cv_unknown);
    }
}

void ssmap_test_perf()
{
    stack_printer __sp__(__func__);

    std::ifstream in("misc/sorted_string_data.dat");
    typedef mdds::sorted_string_map<int> map_type;
    std::vector<map_type::entry> data;
    std::string line;
    int i = 0;
    while (std::getline(in, line))
    {
        data.push_back(map_type::entry());
        data.back().key_length = line.size();
        data.back().value = i;
        char* str = new char[line.size()];
        data.back().key = str;
        strcpy(str, &line[0]);
        ++i;
    }

    assert(data.size() > 1000);
    {
        stack_printer __stack_printer__("::ssmap_test_perf");
        map_type names(data.data(), data.size(), -1);

        assert(names.find(MDDS_ASCII("test")) == -1);
    }

    for (auto it = data.begin(), itEnd = data.end(); it != itEnd; ++it)
    {
        delete[] it->key;
    }
}

int main(int argc, char** argv)
{
    cmd_options opt;
    if (!parse_cmd_options(argc, argv, opt))
        return EXIT_FAILURE;

    if (opt.test_func)
    {
        ssmap_test_basic();
        ssmap_test_mixed_case_null();
        ssmap_test_string_view_entry();
    }

    if (opt.test_perf)
    {
        ssmap_test_perf();
    }

    fprintf(stdout, "Test finished successfully!\n");
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
