#include "mirrow/serd/static/backends/tomlplusplus.hpp"

#include <array>
#include <iostream>

class Person final {
public:
    static std::string family_name;

    Person(const std::string& name, float height)
        : name(name), height(height) {}

    void AddChild(const Person& person) { children.push_back(person); }

    std::string Name() const { return name; }

    float Height() const { return height; }

    Person& operator+(const Person& child) {
        AddChild(child);
        return *this;
    }

    std::string name;
    float height;
    std::vector<Person> children;
};

std::string Person::family_name = "little home";

// clang-format off
#include "mirrow/srefl/srefl_begin.hpp"
srefl_class(Person) {
    bases()
    ctors(ctor(const std::string&, float))
    fields(
        field(&Person::AddChild),
        field(&Person::children),
        field(&Person::Height),
        field(&Person::height),
        field(&Person::Name),
        field(&Person::name),
        field(&Person::operator+)
    )
};
#include "mirrow/srefl/srefl_end.hpp"

// clang-format on

int main() {
    toml::table tbl;
    Person person("VisualGMQ", 172.3f);
    person.children.emplace_back("foo1", 100.0f);
    person.children.emplace_back("foo2", 120.3f);
    person.children.emplace_back("foo3", 130.3f);
    auto person_serd = ::mirrow::serd::serialize(person);
    tbl.emplace("Person", person_serd);

    std::cout << toml::toml_formatter{tbl} << std::endl;
    return 0;
}