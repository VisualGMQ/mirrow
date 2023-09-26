#include "mirrow/serd/static/backends/tomlplusplus.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <array>
#include <iostream>

class Person final {
public:
    static std::string family_name;

    Person() = default;

    Person(const std::string& name, float height, bool male)
        : name(name), height(height), male(male) {}

    void AddChild(const Person& person) { children.push_back(person); }

    std::string Name() const { return name; }

    float Height() const { return height; }

    Person& operator+(const Person& child) {
        AddChild(child);
        return *this;
    }

    bool operator==(const Person& p) const {
        return p.name == name && p.children == children && p.height == height && p.male == male;
    }

    std::string name;
    bool male;
    float height;
    std::vector<Person> children;
    std::optional<int> age;
};

std::string Person::family_name = "little home";

// clang-format off
#include "mirrow/srefl/srefl_begin.hpp"
srefl_class(Person,
    bases()
    ctors(ctor(const std::string&, float))
    fields(
        field(&Person::age),
        field(&Person::AddChild),
        field(&Person::children),
        field(&Person::male),
        field(&Person::Height),
        field(&Person::height),
        field(&Person::Name),
        field(&Person::name),
        field(&Person::operator+)
    )
)
#include "mirrow/srefl/srefl_end.hpp"

// clang-format on

TEST_CASE("serd") {
    Person person("VisualGMQ", 172.3f, false);
    person.children.emplace_back("foo1", 100.0f, true);
    person.children.emplace_back("foo2", 120.3f, false);
    person.children.emplace_back("foo3", 130.3f, true);
    person.age = 16;
    toml::table person_serd;
    ::mirrow::serd::srefl::serialize(person, person_serd);

    std::stringstream ss;
    ss << toml::toml_formatter{person_serd};

    // std::cout << toml::toml_formatter{person_serd} << std::endl;

    toml::table person_tbl = toml::parse(ss.str()).table();

    Person deserd_person;

    ::mirrow::serd::srefl::deserialize<Person>(person_tbl, deserd_person);

    toml::table tbl;

    REQUIRE(deserd_person.name == "VisualGMQ");
    REQUIRE(deserd_person.height == 172.3f);
    REQUIRE(deserd_person.children.size() == 3);

    REQUIRE(deserd_person.children[0] == person.children[0]);
    REQUIRE(deserd_person.children[1] == person.children[1]);
    REQUIRE(deserd_person.children[2] == person.children[2]);

    REQUIRE(deserd_person.age.value() == 16);
}
