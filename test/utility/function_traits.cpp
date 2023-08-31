#include "mirrow/util/function_traits.hpp"
#include <string>

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

using namespace mirrow::util;

void GlobFunc(float, std::string);

struct Clazz {
    std::string Foo();
    void ConstFoo(double)const;
    static int& StaticFoo(float);
};

TEST_CASE("function pointer type") {
    static_assert(std::is_same_v<function_pointer_type_t<GlobFunc>,
                                void (*)(float, std::string)>);
    static_assert(std::is_same_v<function_pointer_type_t<&Clazz::Foo>,
                                std::string (Clazz::*)()>);
    static_assert(std::is_same_v<function_pointer_type_t<Clazz::StaticFoo>,
                                int& (*)(float)>);
    static_assert(std::is_same_v<function_pointer_type_t<&Clazz::ConstFoo>,
                                void (Clazz::*)(double)const>);
}

TEST_CASE("function type from pointer") {
    static_assert(std::is_same_v<function_type_from_pointer_t<GlobFunc>,
                                void(float, std::string)>);
    static_assert(std::is_same_v<function_type_from_pointer_t<&Clazz::Foo>,
                                std::string (Clazz::*)()>);
    static_assert(std::is_same_v<function_type_from_pointer_t<Clazz::StaticFoo>,
                                int&(float)>);
    static_assert(std::is_same_v<function_type_from_pointer_t<&Clazz::ConstFoo>,
                                void (Clazz::*)(double)const>);
}

TEST_CASE("function pointer traits") {
    SECTION("global function") {
        using traits = function_pointer_traits<GlobFunc>;
        static_assert(std::is_same_v<traits::args, type_list<float, std::string>>);
        static_assert(!traits::is_const);
        static_assert(!traits::is_member);
        static_assert(std::is_same_v<traits::pointer, void(*)(float, std::string)>);
        static_assert(std::is_same_v<traits::return_type, void>);
        static_assert(std::is_same_v<traits::type, void(float, std::string)>);
    }

    SECTION("member function") {
        using traits = function_pointer_traits<&Clazz::Foo>;
        static_assert(std::is_same_v<traits::args, type_list<>>);
        static_assert(!traits::is_const);
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::pointer, std::string(Clazz::*)(void)>);
        static_assert(std::is_same_v<traits::return_type, std::string>);
        static_assert(std::is_same_v<traits::type, std::string(Clazz::*)(void)>);
    }

    SECTION("const member function") {
        using traits = function_pointer_traits<&Clazz::ConstFoo>;
        static_assert(std::is_same_v<traits::args, type_list<double>>);
        static_assert(traits::is_const);
        static_assert(traits::is_member);
        static_assert(std::is_same_v<traits::pointer, void(Clazz::*)(double)const>);
        static_assert(std::is_same_v<traits::return_type, void>);
    }

    SECTION("static function") {
        using traits = function_pointer_traits<Clazz::StaticFoo>;
        static_assert(std::is_same_v<traits::args, type_list<float>>);
        static_assert(!traits::is_const);
        static_assert(!traits::is_member);
        static_assert(std::is_same_v<traits::pointer, int&(*)(float)>);
        static_assert(std::is_same_v<traits::return_type, int&>);
        static_assert(std::is_same_v<traits::type, int&(float)>);
    }
}