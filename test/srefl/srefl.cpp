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

// clang-format off
#include "mirrow/srefl/srefl_begin.hpp"
srefl_class(Person) {
    bases<>;
    ctors<ctor<const std::string&, float>>;
    fields<
        field<&Person::AddChild>,
        field<&Person::children>,
        field<&Person::Height>,
        field<&Person::Name>,
        field<&Person::name>,
        field<&Person::operator+>
        >;
};
#include "mirrow/srefl/srefl_end.hpp"

// clang-format on

TEST_CASE("field traits") {
    SECTION("member function") {
        using traits = field_traits<&Person::AddChild>;
        static_assert(std::is_same_v<traits::return_type, void>);
        static_assert(std::is_same_v<traits::args,
                                     mirrow::util::type_list<const Person&>>);
        static_assert(!traits::is_const);
        static_assert(traits::is_member);
        static_assert(traits::pointer == &Person::AddChild);
    }

    SECTION("member function") {
        using traits = field_traits < &Person::operator+>;
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::return_type, Person&>);
        static_assert(std::is_same_v<traits::args,
                                     mirrow::util::type_list<const Person&>>);
        static_assert(traits::pointer == &Person::operator+);
    }

    SECTION("member const function") {
        using traits = field_traits<&Person::Height>;
        static_assert(std::is_same_v<traits::return_type, float>);
        static_assert(std::is_same_v<traits::args, mirrow::util::type_list<>>);
        static_assert(traits::is_const);
        static_assert(traits::is_member);
        static_assert(traits::pointer == &Person::Height);
    }

    SECTION("member variable") {
        using traits = field_traits<&Person::name>;
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::type, std::string>);
        static_assert(traits::pointer == &Person::name);
    }

    SECTION("member variable") {
        using traits = field_traits<&Person::children>;
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::type, std::vector<Person>>);
        static_assert(traits::pointer == &Person::children);
    }

    SECTION("static member variable") {
        using traits = field_traits<&Person::family_name>;
        static_assert(!traits::is_member);
        static_assert(std::is_same_v<traits::type, std::string>);
        static_assert(traits::pointer == &Person::family_name);
    }

    SECTION("macro generation") {
        using type_info = mirrow::srefl::type_info<Person>;
        static_assert(std::is_same_v<type_info::type, Person>);
        static_assert(std::is_same_v<type_info::bases, mirrow::util::type_list<>>);
        static_assert(std::is_same_v<type_info::ctors, mirrow::util::type_list<mirrow::srefl::ctor<const std::string&, float>>>);
        static_assert(type_info::is_final);
        static_assert(std::is_same_v<type_info::fields,
                        mirrow::util::type_list<
                            mirrow::srefl::field_traits<&Person::AddChild>,
                            mirrow::srefl::field_traits<&Person::children>,
                            mirrow::srefl::field_traits<&Person::Height>,
                            mirrow::srefl::field_traits<&Person::Name>,
                            mirrow::srefl::field_traits<&Person::name>,
                            mirrow::srefl::field_traits<&Person::operator+>
                        >>);
    }
}
