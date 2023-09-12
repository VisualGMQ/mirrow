#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <iostream>
#include <sstream>

class Person {
public:
    Person() = default;
    Person(const std::string& name, float height, bool male)
        : name(name), height(height), male(male) {}

    std::string name;
    float height;
    std::vector<Person> children;
    bool male;
    
    bool operator==(const Person& o) const {
        return o.name == name && o.height == height && o.children == children && male == o.male;
    }
};

TEST_CASE("serialization") {
    mirrow::drefl::factory<Person>("Person")
        .ctor<const std::string&, float, bool>()
        .ctor<>()
        .var<&Person::name>("name")
        .var<&Person::height>("height")
        .var<&Person::male>("male")
        .var<&Person::children>("children");

    Person p("VisualGMQ", 123.0, true);
    p.children.push_back(Person{"XiaoMing", 144, false});
    p.children.push_back(Person{"XiaoWang", 127, true});
    mirrow::drefl::any data{p};
    auto tbl = mirrow::serd::drefl::serialize_class(data);

    std::stringstream ss;
    ss << toml::toml_formatter{tbl};

    toml::table deserd_tbl = toml::parse(ss.str()).table();

    Person deserd_p{"", 0.0, false};
    mirrow::drefl::reference_any deserd{p};
    mirrow::serd::drefl::deserialize_class(deserd_tbl, deserd_p);

    REQUIRE(deserd_p == p);
}