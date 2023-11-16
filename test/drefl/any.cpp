#include "mirrow/drefl/exception.hpp"
#include "mirrow/util/misc.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/make_any.hpp"


using namespace mirrow::drefl;

struct test_class {
    int* copy_constructor_counter;
    int* move_constructor_counter;
    int* copy_assignment_counter;
    int* move_assignment_counter;

    test_class(int* a, int* b, int* c, int* d)
        : copy_constructor_counter(a),
          move_constructor_counter(b),
          copy_assignment_counter(c),
          move_assignment_counter(d) {}

    test_class(const test_class& o) {
        copy_all(o);
        (*copy_constructor_counter)++;
    }

    test_class(test_class&& o) {
        copy_all(o);
        (*move_constructor_counter)++;
    }

    test_class& operator=(const test_class& o) {
        copy_all(o);
        (*copy_assignment_counter)++;
        return *this;
    }

    test_class& operator=(test_class&& o) {
        copy_all(o);
        (*move_assignment_counter)++;
        return *this;
    }

private:
    void copy_all(const test_class& o) {
        copy_constructor_counter = o.copy_constructor_counter;
        copy_assignment_counter = o.copy_assignment_counter;
        move_constructor_counter = o.move_constructor_counter;
        move_assignment_counter = o.move_assignment_counter;
    }
};

TEST_CASE("create any") {
    int ccc = 0, mcc = 0, cac = 0, mac = 0;
    test_class test{&ccc, &mcc, &cac, &mac};

    auto a = any_make_copy(test);
    REQUIRE(a.access_type() == mirrow::drefl::any::access_type::Copy);
    REQUIRE(a.type_info() == typeinfo<test_class>());

    test_class* copy = try_cast<test_class>(a);

    REQUIRE(copy != nullptr);
    REQUIRE(*copy->copy_constructor_counter == 1);
    REQUIRE(*copy->move_constructor_counter == 0);
    REQUIRE(*copy->copy_assignment_counter == 0);
    REQUIRE(*copy->move_assignment_counter == 0);
}

TEST_CASE("reference hold") {
    int ccc = 0, mcc = 0, cac = 0, mac = 0;
    test_class test{&ccc, &mcc, &cac, &mac};

    auto c1 = any_make_ref(test);
    REQUIRE(c1.access_type() == any::access_type::Ref);
    REQUIRE(c1.type_info() == typeinfo<test_class>());

    REQUIRE(ccc == 0);
    REQUIRE(mcc == 0);
    REQUIRE(cac == 0);
    REQUIRE(mac == 0);

    *try_cast<test_class>(c1)->copy_assignment_counter += 1;
    REQUIRE(cac == 1);
}

TEST_CASE("const hold") {
    int ccc = 0, mcc = 0, cac = 0, mac = 0;
    test_class test{&ccc, &mcc, &cac, &mac};
    auto c1 = any_make_constref(test);

    REQUIRE(c1.access_type() == any::access_type::ConstRef);
    REQUIRE(c1.type_info() == typeinfo<test_class>());
    REQUIRE(try_cast_const<test_class>(c1) != nullptr);

    REQUIRE(ccc == 0);
    REQUIRE(mcc == 0);
    REQUIRE(cac == 0);
    REQUIRE(mac == 0);

    try {
        // constref try mutable cast will cause error
        auto elem = try_cast<test_class>(c1);
    } catch (const bad_any_access&) {
        REQUIRE(true);
    }
}

TEST_CASE("with enum") {
    enum class MyEnum {
        A = 1,
        B = 2,
        C = 3,
    };

    auto c1 = any_make_copy(MyEnum::A);
    REQUIRE(c1.type_info() == typeinfo<MyEnum>());
    REQUIRE(c1.type_info()->as_enum()->get_value(c1) == 1);
}

TEST_CASE("transform between any") {
    int ccc = 0, mcc = 0, cac = 0, mac = 0;
    test_class test{&ccc, &mcc, &cac, &mac};
    auto c1 = any_make_constref(test);

    SECTION("copy") {
        auto c2 = c1.copy();

        REQUIRE(c2.access_type() == any::access_type::Copy);
        REQUIRE(c2.type_info() == typeinfo<test_class>());
        REQUIRE(try_cast<test_class>(c2) != nullptr);
        REQUIRE(*test.copy_constructor_counter == 1);
        REQUIRE(*test.copy_assignment_counter == 0);
        REQUIRE(*test.move_constructor_counter == 0);
        REQUIRE(*test.move_assignment_counter == 0);
    }

    SECTION("steal") {
        auto c2 = c1.steal();

        REQUIRE(c1.access_type() == any::access_type::Null);
        REQUIRE(c2.access_type() == any::access_type::ConstRef);
        REQUIRE(*test.copy_constructor_counter == 0);
        REQUIRE(*test.copy_assignment_counter == 0);
        REQUIRE(*test.move_constructor_counter == 1);
        REQUIRE(*test.move_assignment_counter == 0);
    }

    SECTION("const ref") {
        auto c2 = c1.constref();

        REQUIRE(c2.access_type() == any::access_type::ConstRef);
        REQUIRE(*test.copy_constructor_counter == 0);
        REQUIRE(*test.copy_assignment_counter == 0);
        REQUIRE(*test.move_constructor_counter == 0);
        REQUIRE(*test.move_assignment_counter == 0);
    }

    SECTION("ref from constref") {
        // make ref from const-ref will cause error
        try {
            auto c2 = c1.ref();
        } catch (const bad_any_access&) {
            REQUIRE(true);
        }
    }

    SECTION("ref from ref") {
        int ccc = 0, mcc = 0, cac = 0, mac = 0;
        test_class test{&ccc, &mcc, &cac, &mac};
        auto c2 = any_make_ref(test);

        auto c3 = c2.ref();

        REQUIRE(c3.access_type() == any::access_type::Ref);
        REQUIRE(c3.type_info() == c2.type_info());
        *try_cast<test_class>(c3)->copy_assignment_counter += 1;

        REQUIRE(*try_cast<test_class>(c3)->copy_assignment_counter == 1);
    }
}