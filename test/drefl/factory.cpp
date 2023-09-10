#include "mirrow/drefl/drefl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <array>

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
        .func<&Person::ChildSize>("ChildSize")
        .var<&Person::name>("name")
        .var<&Person::height>("height")
        .var<&Person::children>("children");

    Person p("VisualGMQ", 123);
    mirrow::drefl::any param = &p;

    auto info = mirrow::drefl::reflected_type<Person>();
    REQUIRE(info);
    REQUIRE(info->name == "Person");

    SECTION("member function") {
        REQUIRE(info->funcs.size() == 4);
        {
            REQUIRE(info->funcs[0]->name == "GetName");
            REQUIRE(info->funcs[0]->is_const_member);
            REQUIRE(info->funcs[0]->type == mirrow::drefl::reflected_type<decltype(&Person::GetName)>());
            REQUIRE(info->funcs[0]->invoke(&param).cast<const std::string&>() == "VisualGMQ");
            REQUIRE(info->funcs[0]->parent == info);
        }

        {
            REQUIRE(info->funcs[1]->name == "GetHeight");
            REQUIRE(info->funcs[1]->is_const_member);
            REQUIRE(info->funcs[1]->type == mirrow::drefl::reflected_type<decltype(&Person::GetHeight)>());
            REQUIRE(info->funcs[1]->invoke(&param).cast<float>() == 123);
            REQUIRE(info->funcs[1]->parent == info);
        }

        p.children.push_back(Person{"XiaoMing", 12});

        {
            REQUIRE(info->funcs[2]->name == "AccessChild");
            REQUIRE_FALSE(info->funcs[2]->is_const_member);

            size_t idx = 0;

            std::array<mirrow::drefl::any, 2> params = {
                param,
                mirrow::drefl::any{idx},
            };

            auto any_person = info->funcs[2]->invoke(params.data());
            auto& person = any_person.cast<Person&>();
            REQUIRE(person.name == "XiaoMing");
        }
    }

    SECTION("member variable") {
        REQUIRE(info->vars[0]->name == "name");
        REQUIRE(info->vars[0]->is_string);
        REQUIRE(info->vars[0]->invoke(&param).cast<const std::string&>() == "VisualGMQ");
        REQUIRE(info->vars[0]->type == mirrow::drefl::reflected_type<decltype(&Person::name)>());
        REQUIRE(info->vars[0]->parent == info);

        REQUIRE(info->vars[1]->name == "height");
        REQUIRE(info->vars[1]->is_floating_pointer);
        REQUIRE(info->vars[1]->invoke(&param).cast<float>() == 123);
        REQUIRE(info->vars[1]->type == mirrow::drefl::reflected_type<decltype(&Person::height)>());
        REQUIRE(info->vars[1]->parent == info);

        REQUIRE(info->vars[2]->name == "children");
        REQUIRE(info->vars[2]->is_container);
        REQUIRE(info->vars[2]->invoke(&param).cast<const std::vector<Person>&>().empty());
        REQUIRE(info->vars[2]->type == mirrow::drefl::reflected_type<decltype(&Person::children)>());
        REQUIRE(info->vars[2]->parent == info);
    }
}