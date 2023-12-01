# Mirrow - A Template Meta Programming utility framework

`mirrow` is  a TMP(template meta programming) utility framework in C++17. It aimed to make some utility to help programmer do TMP easier. Referenced [meta](https://github.com/skypjack/meta) and [ponder](https://github.com/billyquith/ponder).

Nowadays, `mirrow` has these parts:

* `util`: some common utilities
* `srefl`: static reflection framework
* `drefl`: dynamic reflection framework
* `serd`: a serialize framework based on reflection(serial with `drefl`&`srefl`) with TOML

## :book: docs

### util

`util`(utility) has some convenient utility to do TMP:

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

dynamic reflection framework.

#### any

`any` is similar to `std::any`, but support ownership  :microscope:[unittest](./test/drefl/any.cpp)

`any` has 3 ownership(defined in `any::access_type`):

*   `Null`: don't contain data
*   `ConstRef`: const reference to a value
*   `Ref`: mutable reference to a value
*   `Copy`: the data's ownership is any itself, when any destruct, data will destruct together

use `any_make_xxx` to create an any from ownership:

```cpp
int a = 123;
auto cref = mirrow::drefl::any_make_constref(a);	// make a const reference
auto ref = mirrow::drefl::any_make_ref(a);	// make a reference
auto new_value = mirrow::drefl::any_make_copy(a); // copy a to any inner data
```

and use member function `constref()`, `ref()`, `copy()`, `steal()` to translate ownership.



use `try_cast()` & `try_cast_const()` to cast any to a determined type. use `try_cast()` on `ConstRef` any will throw a `bad_any_access` exception and return `nullptr`.

#### factory

`factory` is where you reflect your type:microscope:[unittest](./test/drefl/factory.cpp)

register your class by:

```cpp
struct Person {
    std::string name;
    float height;
    const bool hasChild;
    const Person* couple;
};

// in main():
mirrow::drefl::factory<Person>::instance()
              .regist("Person")
              .property("name", &Person::name)
              .property("height", &Person::height)
              .property("hasChild", &Person::hasChild)
              .property("couple", &Person::couple);
```

or register your enum by:

```cpp
enum class MyEnum {
        Value1 = 1,
        Value2 = 2,
        Value3 = 3,
    };

// in main():
auto& inst = mirrow::drefl::factory<MyEnum>::instance()
    .regist("MyEnum")
    .add("Value1", MyEnum::Value1)
    .add("Value2", MyEnum::Value2)
    .add("Value3", MyEnum::Value3);	
```

then use

```cpp
auto info = mirrow::drefl::typeinfo<Person>();
```

to get registered type information;

**NOTE: currently we don't support register member function.**



there are may type information you can access(by member function`as_xxx()`):

*   `enum_info`: enumerates
*   `numeric`: numerics(`int`,`float`,`char`...)
*   `boolean`: boolean
*   `string`:`std::string` or `std::string_view` (may add `const char*` support later)
*   `pointer`: pointers like `T*`, `T* const`, `T**`...
*   `array`: `std::vector`, `std::array`, `std::list`, `T[N]`
*   `class`: other classes

*future support:*

*   `map`: `std::unordered_map`, `std::map`
*   `set`: `std::unordered_set`, `std::set`
*   `optional`: `std::optional`
*   `smart points`: `std::unique_ptr`. `std::shared_ptr`
*   `pair`: `std::pair`


### serd

A serialize/deserialize tools based on dynamic/static reflection.

`serd` provide two serialize/deserialize method: dynamic and static, which need you use dynamic/static reflection to provide type info first.

dynamic reflection based serialize [:microscope:  unittest](./test/serd/drefl_serd.cpp)
static reflection based serialize [:microscope:  unittest](./test/serd/srefl_serd.cpp)

After reflected type, you can do serialize like:

```cpp
type instance;  // create an instance

// use static reflection based serialize
toml::table tbl;
mirrow::serd::srefl::serialize(instance, tbl);
// use static reflection based deserialize
mirrow::serd::srefl::deserialize(tbl, instance);

// convert instance to any to prepare serialize
mirrow::drefl::reference_any data{instance};
// use dynamic reflection based serialize
toml::table tbl = mirrow::serd::drefl::serialize(data);
// use dynamic reflection based deserialize
mirrow::serd::drefl::deserialize(tbl, data);
```

If you don't know which toml node would be serialize/deserialize, you can use `mirrow::serd::srefl::serialize_destination_type_t<your-type>` to get the type.

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

for static \[de\]serialize, need two step:

```cpp
namespace mirrow::serd::srefl {

// 1. tell serd which toml node you want to serialize to
// use SFINEA
namespace impl {

template <>
struct serialize_destination_type<your_own_type> {
    // we want [de]serialize to/from toml::value
    using type = toml::value<your_own_type>;
};

}


// 2. provide your [de]serialize method
// also use SFINEA, serialize function
template <typename T>
std::enable_if_t<std::is_same_v<your_own_type, T>>
serialize(const T& value, serialize_destination_type_t<T>& node) {
    // try put value into node
    ...
}

// also use SFINEA, deserialize function
template <typename T>
std::enable_if_t<std::is_same_v<T, your_own_type>>
deserialize(const toml::node& node, T& elem) {
    // try parse elem from node
    ...
}

}
```

for dynamic \[de\]serialize, you need regist your function into `serialize_method_storage` at runtime:

```cpp
mirrow::serd::drefl::serialize_method_storage::instance().regist(type_info, serialize_fn, deserialize_fn);
```

the second and thrid param is your serialize/deserialize function, must be:

```cpp
// serialize
void serialize(toml::node&, const any&)>;
// deserialize
void deserialize(const toml::node&, any&)>;
```

after these, you can use serialize/deserialize:

```cpp
Person p;
mirrow::drefl::any any = mirrow::drefl::any_make_ref(p);

// serialize to toml node
auto tbl = ::mirrow::serd::drefl::serialize(any);

// deserialize from toml node
::mirrow::serd::drefl::deserialize(p, tbl);
```
