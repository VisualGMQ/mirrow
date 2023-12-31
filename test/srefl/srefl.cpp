#include "mirrow/srefl/srefl.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <vector>

using namespace mirrow::srefl;

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

enum class MyEnum {
    Value1 = 1,
    Value2,
    Value3,
};

TEST_CASE("strip name") {
    static_assert(strip_name("Person") == "Person");
    static_assert(strip_name("::Person") == "Person");
    static_assert(strip_name("namespace::Person") == "Person");
    static_assert(strip_name("&Person::Foo") == "Foo");
    static_assert(strip_name("&Foo") == "Foo");
}

TEST_CASE("field traits") {
    SECTION("member function") {
        constexpr auto traits =
            field_traits{&Person::AddChild, "&Person::AddChild"};
        static_assert(std::is_same_v<decltype(traits)::return_type, void>);
        static_assert(std::is_same_v<decltype(traits)::args,
                                     mirrow::util::type_list<const Person&>>);
        static_assert(!traits.is_const_member());
        static_assert(traits.is_member());
        static_assert(traits.pointer() == &Person::AddChild);
        static_assert(traits.name() == "AddChild");
    }

    SECTION("member function") {
        constexpr auto traits = field_traits{&Person::operator+, "&operator+"};
        static_assert(!traits.is_const_member());
        static_assert(std::is_same_v<decltype(traits)::return_type, Person&>);
        static_assert(std::is_same_v<decltype(traits)::args,
                                     mirrow::util::type_list<const Person&>>);
        static_assert(traits.pointer() == &Person::operator+);
        static_assert(traits.name() == "operator+");
    }

    SECTION("member const function") {
        constexpr auto traits =
            field_traits{&Person::Height, "&Person::Height"};
        static_assert(std::is_same_v<decltype(traits)::return_type, float>);
        static_assert(
            std::is_same_v<decltype(traits)::args, mirrow::util::type_list<>>);
        static_assert(traits.is_const_member());
        static_assert(traits.is_member());
        static_assert(traits.pointer() == &Person::Height);
        static_assert(traits.name() == "Height");
    }

    SECTION("member variable") {
        constexpr auto traits = field_traits(&Person::name, "&Person::name");
        static_assert(!traits.is_const_member());
        static_assert(traits.is_member());
        static_assert(std::is_same_v<decltype(traits)::type, std::string>);
        static_assert(traits.pointer() == &Person::name);
        static_assert(traits.name() == "name");
    }

    SECTION("member variable") {
        constexpr auto traits =
            field_traits(&Person::children, "&Person::children");
        static_assert(!traits.is_const_member());
        static_assert(traits.is_member());
        static_assert(
            std::is_same_v<decltype(traits)::type, std::vector<Person>>);
        static_assert(traits.pointer() == &Person::children);
        static_assert(traits.name() == "children");
    }

    SECTION("static member variable") {
        constexpr auto traits =
            field_traits(&Person::family_name, "&Person::family_name");
        static_assert(!traits.is_member());
        static_assert(std::is_same_v<decltype(traits)::type, std::string>);
        static_assert(traits.pointer() == &Person::family_name);
        static_assert(traits.name() == "family_name");
    }
}
