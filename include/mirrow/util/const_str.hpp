#pragma once

#include <string_view>
#include <utility>

namespace mirrow {

namespace util {

template <typename Char, Char... Chars>
struct const_string {
    static constexpr Char data[] = {Chars..., 0};
    static constexpr size_t length = sizeof...(Chars);

    constexpr std::string_view str() const noexcept { return data; }
};

template <typename T, std::size_t... N>
constexpr decltype(auto) prepareImpl(T, std::index_sequence<N...>) {
    return const_string<char, T::get()[N]...>();
}

template <typename T>
constexpr decltype(auto) prepare(T t) {
    return prepareImpl(t, std::make_index_sequence<sizeof(T::get()) - 1>());
}

#define CONST_STR(s)                                \
    (::mirrow::util::prepare([] {                   \
        struct tmp {                                \
            static constexpr decltype(auto) get() { \
                return s;                           \
            }                                       \
        };                                          \
        return tmp{};                               \
    }()))

}  // namespace util

}  // namespace mirrow