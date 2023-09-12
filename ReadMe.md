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
    