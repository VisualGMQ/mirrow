# Mirrow - A Template Meta Programming utility framework

`mirrow` is  a TMP(template meta programming) utility framework in C++17. It aimed to make some utility to help programmer do TMP easier.

Nowadays, `mirrow` has these parts:

* `util`: some common utilities
* `srefl`: static reflection framework
* `drefl`: dynamic reflection framework
* `serd`: a serialize framework based on reflection(serial with drefl in WIP, with srefl is finished)

## :book: docs

### util

util(utility) has some convenient utility to do TMP:

* `type_list`: compile-time type list. [:microscope: unittest](./test/utility/type_list.cpp)
* `function_traits`: compile-time function info trait. [:microscope: unittest](./test/utility/function_traits.cpp)
* `variable_traits`: compile-time variable info trait. [:microscope: unittest](./test/utility/variable_traits.cpp)
* `const_str`: compile-time string. [:microscope: unittest](./test/utility/const_str.cpp)

### srefl

static reflection framework. :microscope: [do reflect unittest](./test/srefl/srefl.cpp), [get reflected info unittest](./test/srefl/reflect.cpp)

To reflect your class, you must do like this:

```cpp
// your class
class Foo final {
public:
    void foo() {}
    void foo(int) {}
    void foo(float) {}
    void another() {}

    int value_1 = 1;
    int value_2 = 2;
};

// include srefl_begin.hpp
#include "mirrow/srefl/srefl_begin.hpp"
// do your reflection
srefl_class(Foo,
    ctors()
    fields(
        field(static_cast<void(Foo::*)(void)>(&Foo::foo)),
        field(static_cast<void(Foo::*)(int)>(&Foo::foo)),
        field(static_cast<void(Foo::*)(float)>(&Foo::foo)),
        field(&Foo::another),
        field(&Foo::value_1),
        field(&Foo::value_2)
    )
)
// include srefl_end.hpp
#include "mirrow/srefl/srefl_end.hpp"
```

[srefl_begin.hpp](./include/mirrow/srefl/srefl_begin.hpp) provide a bunch of macros to help you regist your class. And [srefl_end.hpp](./include/mirrow/srefl/srefl_end.hpp) will `#undef` these macros to avoid pollute your codes.

Then, use `srefl_class(<your class>, ...)` to start regist your class. use `ctors()` to regist constructors(optional), use `fields(...)` to start regist member/static variable/functions.

After reflect, you can use `auto refl = reflect<Foo>();` to get reflected informations. And visit member variables:

```cpp
refl.visit_member_variables([&vars](auto&& value) {
    vars.push_back(value.name());
});
```

*visit function/static fields is WIP, it is easy to implement but currently I don't need them*

### drefl

dynamic reflection framework. :microscope: [unittest](./test/drefl/factory.cpp)
    
regist your type by:

```cpp
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

int main() {
    mirrow::drefl::factory<Person>("Person")
        // regist constructor
        .ctor<const std::string&, float>()
        // regist member/non-member function
        .func<&Person::GetName>("GetName")
        .func<&Person::GetHeight>("GetHeight")
        .func<&Person::AccessChild>("AccessChild")
        .func<&Person::ChildSize>("ChildSize")
        // regist variable
        .var<&Person::name>("name")
        .var<&Person::height>("height")
        .var<&Person::children>("children");
}
```

then use

```cpp
auto info = mirrow::drefl::reflected_type<Person>();
```

to get registed type information;

you can also use class name to get reflected info:

```cpp
auto info = mirrow::drefl::reflected_type("Person");
```

**NOTE:currently we allow types with the same name to exist, this may fixed later.**

the core type info class is `mirrow::drefl::type_info`, you can get many information from here(some examples):

```cpp
type.is_fundamental();  // check type is fundamental type(numeric type)

if (type.is_class()) {
    auto class_info = type.as_class();
}
...
```

another important type is `mirrow::drefl::any`, it is similar to `std::any`, but can create by `type_info`(see [:microscope: unittest](./test/drefl/any.cpp))

`mirrow::drefl::any` will copy the instance. If want only reference a exists instance, use `mirrow::drefl::reference_any`.


### serd

A serialize/deserialize tools based on dynamic/static reflection.

`serd` provide two serialize/deserialize method: dynamic and static, which need you use dynamic/static reflection to provide type info first.

dynamic reflection based serialize [:microscope:  unittest](./test/serd/drefl_serd.cpp)
static reflection based serialize [:microscope:  unittest](./test/serd/srefl_serd.cpp)

After reflected type, you can do serialize like:

```cpp
type instance;  // create an instance

// use static reflection based serialize
toml::table tbl = mirrow::serd::srefl::serialize(instance);
// use static reflection based deserialize
mirrow::serd::srefl::serialize(tbl, instance);

// convert instance to any to prepare serialize
mirrow::drefl::any data{instance};
// use dynamic reflection based serialize
toml::table tbl = mirrow::serd::drefl::serialize(instance);
// use dynamic reflection based deserialize
mirrow::serd::drefl::serialize(tbl, instance);
```

#### custom serialize function

all `serialize` and `deserialize` function will iterate all member fields in your type info and \[de\]serialize them. If field not exists when deserialize, it will log and ignore this field.

There are some inner-support type:

* numeric(integer like `int`, `char` ..., and floating point(`float`, `double`))
* `bool`
* `std::vector`
* `std::array`
* `std::optional`
* `std::unordered_map`
* `std::unordered_set`

If you want do specific \[de\]serialize method on your own type, here:

for static \[de\]serialize, need three step:

```cpp
namespace mirrow::serd::srefl::impl {

// 1. tell serd you need a custom serialize method
// use SFINEA
template <typename T>
struct has_serialize_method<
    T, std::void_t<std::enable_if_t<std::is_same_v<T, your_own_type>>>> {
    static constexpr bool value = true;
};

// 2. tell serd which toml node you want to serialize to
template <>
struct serialize_destination_type<your_own_type> {
    // we want [de]serialize to/from toml::value
    using type = toml::value<your_own_type>;
};


// 3. provide your [de]serialize method
// also use SFINEA, serialize function
template <typename T>
std::enable_if_t<std::is_same_v<your_own_type, T>>
serialize_impl(const T& value, serialize_destination_type_t<T>& node) {
    // try put value into node
    ...
}

// also use SFINEA, deserialize function
template <typename T>
std::enable_if_t<std::is_same_v<T, your_own_type>>
deserialize_impl(const toml::node& node, T& elem) {
    // try parse elem from node
    ...
}

}
```

for dynamic \[de\]serialize, you need regist your function into `serd_method_registry` at runtime:

```cpp
mirrow::serd::drefl::serd_method_registry::instance().regist(type_info, serialize_fn, deserialize_fn);
```

the first param `type_info` is your type info, can be accessed by `::mirrow::srefl::reflect<T>()` or by name `::mirrow::srefl::reflect(your_type_name)`

the second and thrid param is your serialize/deserialize function, must be:

```cpp
// serialize
void serialize(toml::node&, std::string_view name, mirrow::drefl::any&)>;
// deserialize
void deserialize(const toml::node&, mirrow::drefl::reference_any&)>;
```

after these, you can use serialize/deserialize:

```cpp
Person p;
mirrow::drefl::reference_any any{p};

// serialize to toml node
auto tbl = ::mirrow::serd::drefl::serialize(any);

// deserialize from toml node
::mirrow::serd::drefl::deserialize(tbl, p);
```