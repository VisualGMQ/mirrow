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

            int a = 0;

            std::array<mirrow::drefl::any, 2> params = {
                mirrow::drefl::any{&p},
                mirrow::drefl::any{a},
            };

            auto& np = std::invoke(&Person::AccessChild, &p, 0);

            mirrow::drefl::internal::invoke<&Person::AccessChild>(params.data(), std::make_index_sequence<2>());
            auto person = funcs[2].node_->invoke(params.data());
            // auto& person = funcs[2].invoke(&p, 0).cast<Person&>();
            REQUIRE(person.cast<Person&>().name == "XiaoMing");
        }
    }
}