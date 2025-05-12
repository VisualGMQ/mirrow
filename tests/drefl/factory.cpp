#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/array.hpp"
#include "mirrow/drefl/raw_type.hpp"
#include "mirrow/util/misc.hpp"
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "mirrow/drefl/cast_any.hpp"
#include "mirrow/drefl/factory.hpp"
#include "mirrow/drefl/make_any.hpp"
#include "mirrow/drefl/type.hpp"
#include "mirrow/drefl/value_kind.hpp"

TEST_CASE("enum factory") {
    enum class MyEnum {
        Value1 = 1,
        Value2 = 2,
        Value3 = 3,
    };

    auto& inst = mirrow::drefl::enum_factory<MyEnum>::instance();

    inst.regist("MyEnum")
        .add("Value1", MyEnum::Value1)
        .add("Value2", MyEnum::Value2)
        .add("Value3", MyEnum::Value3);

    REQUIRE(inst.has_registed());
    auto& enum_info = inst.info();

    REQUIRE(enum_info.name() == "MyEnum");
    REQUIRE(enum_info.enums()[0].name() == "Value1");
    REQUIRE(enum_info.enums()[0].value() == 1);
    REQUIRE(enum_info.enums()[1].name() == "Value2");
    REQUIRE(enum_info.enums()[1].value() == 2);
    REQUIRE(enum_info.enums()[2].name() == "Value3");
    REQUIRE(enum_info.enums()[2].value() == 3);
}

TEST_CASE("simple property factory") {
    struct Person {
        float a;
        const bool& c;
        int* p;
    };

    SECTION("simple property") {
        auto prop =
            mirrow::drefl::numeric_property_factory{"a", &Person::a}.get();
        REQUIRE_FALSE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "a");
    }

    SECTION("pointer property") {
        auto prop =
            mirrow::drefl::numeric_property_factory{"p", &Person::p}.get();
        REQUIRE_FALSE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "p");
    }
}

TEST_CASE("numeric factory") {
    auto& n = mirrow::drefl::numeric_factory<int>::instance().info();

    SECTION("type check") {
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Numeric);
        REQUIRE(n.numeric_kind() == mirrow::drefl::numeric::numeric_kind::Int);
        REQUIRE(n.name() == "int");
    }

    SECTION("operations") {
        int a = 123;
        auto ref = mirrow::drefl::any_make_ref(a);
        REQUIRE(n.get_value(ref) == 123);
        n.set_value(ref, 456l);
        REQUIRE(a == 456);
    }
}

TEST_CASE("boolean factory") {
    auto& n = mirrow::drefl::boolean_factory::instance().info();
    REQUIRE(n.kind() == mirrow::drefl::value_kind::Boolean);
    REQUIRE(n.name() == "bool");
}

TEST_CASE("string factory") {
    SECTION("std::string") {
        auto& n = mirrow::drefl::string_factory<std::string>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::String);
        REQUIRE(n.is_string());
        REQUIRE_FALSE(n.is_string_view());
        REQUIRE(n.name() == "std::string");
    }

    SECTION("std::string_view") {
        auto& n =
            mirrow::drefl::string_factory<std::string_view>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::String);
        REQUIRE(n.is_string_view());
        REQUIRE_FALSE(n.is_string());
        REQUIRE(n.name() == "std::string_view");
    }
}

TEST_CASE("pointer factory") {
    SECTION("int*") {
        auto& n = mirrow::drefl::pointer_factory<int*>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.name() == "int*");
        REQUIRE(n.layers() == 1);
        REQUIRE_FALSE(n.is_const());
        REQUIRE_FALSE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }

    SECTION("int* const") {
        auto& n = mirrow::drefl::pointer_factory<int* const>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.layers() == 1);
        REQUIRE(n.is_const());
        REQUIRE_FALSE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }

    SECTION("const int*") {
        auto& n = mirrow::drefl::pointer_factory<const int*>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.layers() == 1);
        REQUIRE_FALSE(n.is_const());
        REQUIRE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }

    SECTION("const int* const") {
        auto& n =
            mirrow::drefl::pointer_factory<const int* const>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.layers() == 1);
        REQUIRE(n.is_const());
        REQUIRE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }

    SECTION("int***") {
        auto& n = mirrow::drefl::pointer_factory<int***>::instance().info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.layers() == 3);
        REQUIRE_FALSE(n.is_const());
        REQUIRE_FALSE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }

    SECTION("const int** const") {
        auto& n = mirrow::drefl::pointer_factory<const int** const>::instance()
                      .info();
        REQUIRE(n.kind() == mirrow::drefl::value_kind::Pointer);
        REQUIRE(n.layers() == 2);
        REQUIRE(n.is_const());
        REQUIRE(n.is_point_type_const());
        REQUIRE(n.type_info() == mirrow::drefl::typeinfo<int>());
    }
}

TEST_CASE("ordinary array", "array") {
    auto& n = mirrow::drefl::array_factory<int[3]>::instance().info();

    SECTION("type check") {
        REQUIRE(n.array_type() == mirrow::drefl::array::array_type::Static);
        REQUIRE(n.addressing_type() ==
                mirrow::drefl::array::addressing_type::Random);
    }

    SECTION("operation check") {
        int arr[3] = {1, 2, 3};
        auto ref = mirrow::drefl::any_make_ref(arr);
        REQUIRE(ref.type_info() == &n);

        auto elem = n.get(0, ref);
        REQUIRE(elem.access_type() == mirrow::drefl::any::access_type::Ref);
        REQUIRE(elem.type_info()->as_numeric()->get_value(elem) == 1);
        REQUIRE(n.size(ref) == 3);
        REQUIRE(n.capacity(ref) == 3);
        REQUIRE_FALSE(n.push_back(elem, ref));
        REQUIRE_FALSE(n.back(ref).has_value());
        REQUIRE_FALSE(n.resize(3, ref));
        REQUIRE_FALSE(n.insert(3, elem, ref));
    }
}

TEST_CASE("std::vector", "array") {
    using type = std::vector<int>;
    auto& n = mirrow::drefl::array_factory<type>::instance().info();

    SECTION("type check") {
        REQUIRE(n.array_type() == mirrow::drefl::array::array_type::Dynamic);
        REQUIRE(n.addressing_type() ==
                mirrow::drefl::array::addressing_type::Random);
        REQUIRE(n.elem_type() == mirrow::drefl::factory<int>::info());
    }

    SECTION("operation check") {
        type arr = {1, 2, 3};
        auto ref = mirrow::drefl::any_make_ref(arr);
        REQUIRE(ref.type_info() == &n);

        auto elem = n.get(0, ref);
        REQUIRE(elem.access_type() == mirrow::drefl::any::access_type::Ref);
        REQUIRE(elem.type_info()->as_numeric()->get_value(elem) == 1);
        REQUIRE(n.size(ref) == 3);
        REQUIRE(n.capacity(ref) == arr.capacity());
        REQUIRE(n.push_back(elem, ref));

        elem = n.back(ref);
        REQUIRE(elem.type_info()->as_numeric()->get_value(elem) == 1);
        REQUIRE(n.resize(2, ref));
        REQUIRE(n.size(ref) == 2);

        elem = elem.copy();
        elem.type_info()->as_numeric()->set_value(elem, 9l);
        REQUIRE(n.insert(0, elem, ref));
        REQUIRE(arr[0] == 9);
    }
}

TEST_CASE("std::list", "array") {
    using type = std::list<int>;
    auto& n = mirrow::drefl::array_factory<type>::instance().info();

    SECTION("type check") {
        REQUIRE(n.array_type() == mirrow::drefl::array::array_type::Dynamic);
        REQUIRE(n.addressing_type() ==
                mirrow::drefl::array::addressing_type::Forward);
        REQUIRE(n.elem_type() == mirrow::drefl::factory<int>::info());
    }

    SECTION("operation check") {
        type arr = {1, 2, 3};
        auto ref = mirrow::drefl::any_make_ref(arr);
        REQUIRE(ref.type_info() == &n);

        auto elem = n.get(0, ref);
        REQUIRE(elem.access_type() == mirrow::drefl::any::access_type::Ref);
        REQUIRE(elem.type_info()->as_numeric()->get_value(elem) == 1);
        REQUIRE(n.size(ref) == 3);
        REQUIRE(n.capacity(ref) == arr.size());
        REQUIRE(n.push_back(elem, ref));

        elem = n.back(ref);
        REQUIRE(elem.type_info()->as_numeric()->get_value(elem) == 1);
        REQUIRE(n.resize(2, ref));
        REQUIRE(n.size(ref) == 2);

        elem = elem.copy();
        elem.type_info()->as_numeric()->set_value(elem, 9l);
        REQUIRE(n.insert(0, elem, ref));
        REQUIRE(*arr.begin() == 9);
    }
}


TEST_CASE("class factory") {
    struct Person {
        std::string name;
        float height;
        const bool hasChild;
        const Person* couple;
    };

    auto& p = mirrow::drefl::class_factory<Person>::instance()
                  .regist("Person")
                  .property("name", &Person::name)
                  .property("height", &Person::height)
                  .property("hasChild", &Person::hasChild)
                  .property("couple", &Person::couple)
                  .info();

    REQUIRE(p.name() == "Person");
    REQUIRE(p.kind() == mirrow::drefl::value_kind::Class);
    auto& props = p.properties();

    Person inst { "VisualGMQ", 123.0, false, nullptr };
    auto ref = mirrow::drefl::any_make_constref(inst);

    SECTION("string property") {
        auto& prop = props[0];
        REQUIRE(prop->kind() == mirrow::drefl::value_kind::Property);
        REQUIRE(prop->class_info() == &p);
        REQUIRE_FALSE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "name");
        REQUIRE(prop->type_info() == mirrow::drefl::typeinfo<std::string>());

        auto name = prop->call(ref);
        REQUIRE(name.type_info() == mirrow::drefl::typeinfo<std::string>());
        REQUIRE(*mirrow::drefl::try_cast_const<std::string>(name) == "VisualGMQ");
    }

    SECTION("float property") {
        auto& prop = props[1];
        REQUIRE(prop->kind() == mirrow::drefl::value_kind::Property);
        REQUIRE(prop->class_info() == &p);
        REQUIRE_FALSE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "height");
        REQUIRE(prop->type_info() == mirrow::drefl::typeinfo<float>());

        auto height = prop->call(ref);
        REQUIRE(height.type_info() == mirrow::drefl::typeinfo<float>());
        REQUIRE(*mirrow::drefl::try_cast_const<float>(height) == 123);
    }

    SECTION("bool property") {
        auto& prop = props[2];
        REQUIRE(prop->kind() == mirrow::drefl::value_kind::Property);
        REQUIRE(prop->class_info() == &p);
        REQUIRE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "hasChild");
        REQUIRE(prop->type_info() == mirrow::drefl::typeinfo<bool>());

        auto hasChild = prop->call(ref);
        REQUIRE(hasChild.type_info() == mirrow::drefl::typeinfo<bool>());
        REQUIRE(*mirrow::drefl::try_cast_const<bool>(hasChild) == false);
    }

    SECTION("couple property") {
        auto& prop = props[3];
        REQUIRE(prop->kind() == mirrow::drefl::value_kind::Property);
        REQUIRE(prop->class_info() == &p);
        REQUIRE_FALSE(prop->is_const());
        REQUIRE_FALSE(prop->is_ref());
        REQUIRE(prop->name() == "couple");

        auto couple = prop->call_const(ref);


        auto q = static_cast<const Person*>(ref.payload());
        auto& value = q->*(&Person::couple);
        auto q2 = static_cast<const Person* const>(couple.payload());

        REQUIRE(couple.type_info() == mirrow::drefl::typeinfo<const Person*>());
        REQUIRE(*mirrow::drefl::try_cast_const<const Person*>(couple) == nullptr);
    }
}
