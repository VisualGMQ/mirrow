#include "mirrow/drefl/drefl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>

class Person {
public:
    Person(const std::string& name, float height)
        : name(name), height(height) {}

    const std::string& GetName() const { return name; }

    float GetHeight() const { return height; }

    Person& AccessChild(size_t idx) { return children[idx]; }

    size_t ChildSize() const { return children.size(); }

    std::string name;
    float height;
    std::vector<Person> children;
};

TEST_CASE("factory") {
    mirrow::drefl::factory<Person>("Person")
        .ctor<const std::string&, float>()
        .func<&Person::GetName>("GetName")
        .func<&Person::GetHeight>("GetHeight")
        .func<&Person::AccessChild>("AccessChild")
        .func<&Person::ChildSize>("ChildSize");

    Person p("VisualGMQ", 123);

    SECTION("member function") {
        auto info = mirrow::drefl::resolve<Person>();
        REQUIRE(info);
        REQUIRE(info.name() == "Person");

        auto funcs = info.funcs();
        REQUIRE(funcs.size() == 4);
        {
            REQUIRE(funcs[0].name() == "GetName");
            REQUIRE(funcs[0].is_const());
            REQUIRE(funcs[0].is_member());
            REQUIRE(funcs[0].invoke(&p).cast<const std::string&>() == "VisualGMQ");
        }

        {
            REQUIRE(funcs[1].name() == "GetHeight");
            REQUIRE(funcs[1].is_const());
            REQUIRE(funcs[1].is_member());
            REQUIRE(funcs[1].invoke(&p).cast<float>() == 123);
        }

        p.children.push_back(Person{"XiaoMing", 12});

        {
            REQUIRE(funcs[2].name() == "AccessChild");
            REQUIRE(!funcs[2].is_const());
            REQUIRE(funcs[2].is_member());

            size_t idx = 0;

            // auto& person = funcs[2].invoke(&p, idx).cast<Person&>();
            auto any_person = funcs[2].invoke(&p, idx);
            auto& person = any_person.cast<Person&>();
            REQUIRE(person.name == "XiaoMing");
        }
    }
}