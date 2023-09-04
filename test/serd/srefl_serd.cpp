#include "mirrow/serd/static/backends/tomlplusplus.hpp"
#include <iostream>
#include <array>

int main() {
    toml::table tbl;
    std::array<float, 3> arr = {1, 2, 3};

    tbl.emplace("arr", mirrow::serd::serialize(std::array<int, 3>{1, 2, 3}));
    tbl.emplace("tbl", mirrow::serd::serialize(std::unordered_map<std::string_view, int>{
        {"value1", 1},
        {"value2", 2},
        {"value3", 3},
    }));

    std::cout << toml::toml_formatter{tbl} << std::endl;
    return 0;
}