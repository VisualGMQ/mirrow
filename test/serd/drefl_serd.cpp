#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"
#include <toml++/impl/toml_formatter.hpp>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/make_any.hpp"
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
    bool male;
    std::array<int, 5> ids{1, 2, 3, 4, 5};

    bool operator==(const Person& o) const {
        return o.name == name && o.height == height && male == o.male;
    }
};

TEST_CASE("serialization & deserialization") {
    mirrow::drefl::class_factory<Person>::instance()
        .regist("Person")
        .property("name", &Person::name)
        .property("height", &Person::height)
        .property("male", &Person::male)
        .property("ids", &Person::ids);

    auto value = mirrow::drefl::any_make_copy(Person{"VisualGMQ", 123.0, true});

    toml::table tbl = mirrow::serd::drefl::serialize(value, "Person");

    std::cout << toml::toml_formatter{tbl} << std::endl;

    value = mirrow::drefl::any_make_copy(Person{"", 0.0, false});
    mirrow::serd::drefl::deserialize(value, tbl);

    Person* person = mirrow::drefl::try_cast<Person>(value);
    REQUIRE(person);
    REQUIRE(person->name == "VisualGMQ");
    REQUIRE(person->height == 123.0);
    REQUIRE(person->male == true);

    REQUIRE(person->ids == std::array<int, 5>{1, 2, 3, 4, 5});
}