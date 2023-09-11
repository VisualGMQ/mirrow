#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <iostream>
#include <sstream>

class Person {
public:
    Person(const std::string& name, float height)
        : name(name), height(height) {}

    std::string name;
    float height;
    std::vector<Person> children;
};

TEST_CASE("serialization") {
    mirrow::drefl::factory<Person>("Person")
        .ctor<const std::string&, float>()
        .var<&Person::name>("name")
        .var<&Person::height>("height")
        .var<&Person::children>("children");

    Person p("VisualGMQ", 123.0);
    p.children.push_back(Person{"XiaoMing", 144});
    p.children.push_back(Person{"XiaoWang", 127});
    mirrow::drefl::any data = &p;
    auto tbl = mirrow::sred::drefl::serialize_class(data);
    std::cout << toml::toml_formatter{tbl} << std::endl;

    std::stringstream ss;
    ss << toml::toml_formatter{tbl};

    toml::table deserd_tbl = toml::parse(ss.str());

    mirrow::drefl::any deserd = Person{"", 0.0};
    mirrow::sred::drefl::deserialize_class(deserd, deserd_tbl);
}