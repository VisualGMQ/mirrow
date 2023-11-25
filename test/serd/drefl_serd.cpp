#define TOML_IMPLEMENTATION
#include "mirrow/serd/dynamic/backends/tomlplusplus.hpp"

#include "mirrow/drefl/cast_any.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/make_any.hpp"
#include <array>
#include <iostream>

class Person {
public:
    Person() = default;

    Person(const std::string& name, float height, bool male,
           const std::array<int, 5>& ids, std::optional<int> opt)
        : name(name), height(height), male(male), ids(ids), opt(opt) {}

    std::string name;
    float height;
    bool male;
    std::array<int, 5> ids;
    std::optional<int> opt;

    bool operator==(const Person& o) const {
        return o.name == name && o.height == height && male == o.male &&
               ids == o.ids && opt == o.opt;
    }
};

TEST_CASE("serialization & deserialization") {
    mirrow::drefl::class_factory<Person>::instance()
        .regist("Person")
        .property("name", &Person::name)
        .property("height", &Person::height)
        .property("male", &Person::male)
        .property("ids", &Person::ids)
        .property("opt", &Person::opt);

    auto value = mirrow::drefl::any_make_copy(Person{
        "VisualGMQ", 123.0, true, {1, 2, 3, 4, 5}, 3
    });

    toml::table tbl;
    mirrow::serd::drefl::serialize(tbl, value, "Person");

    std::cout << toml::toml_formatter{tbl} << std::endl;

    value = mirrow::drefl::any_make_copy(Person{
        "", 0.0, false, {0, 0, 0, 0, 0}, std::nullopt
    });
    mirrow::serd::drefl::deserialize(value, tbl);

    Person* person = mirrow::drefl::try_cast<Person>(value);
    REQUIRE(person);
    REQUIRE(*person == *mirrow::drefl::try_cast_const<Person>(value));
}