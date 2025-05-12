#define TOML_IMPLEMENTATION
#include "mirrow/serd/static/backends/tomlplusplus.hpp"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>

template <typename T>
struct Interpolation final {
    static T Linear(float t, T a, T b) { return a + (b - a) * t; };
};

template <typename T, typename TimeType>
struct BasicKeyFrame {
    using interpolate_func = std::function<T(float, T, T)>;
    using value_type = T;
    using time_type = TimeType;

    T value;
    TimeType timePoint;
    interpolate_func interpolate;

    static BasicKeyFrame Create(
        T value, TimeType t,
        interpolate_func interp = Interpolation<T>::Linear) {
        return {value, t, interp};
    }
};

template <typename T>
using KeyFrame = BasicKeyFrame<T, int>;

namespace mirrow::serd::srefl {

namespace impl {

template <typename T>
struct has_serialize_method<
    T, std::enable_if_t<std::is_same_v<T, KeyFrame<typename T::value_type>>>> {
    static constexpr bool value = true;
};

}  // namespace impl

template <typename T>
std::enable_if_t<std::is_same_v<T, KeyFrame<typename T::value_type>>> serialize(
    const T& frame, serialize_destination_type_t<T>& tbl) {
    mirrow::serd::srefl::serialize_destination_type_t<typename T::value_type>
        frameTbl;
    serialize(frame.value, frameTbl);
    tbl.emplace("value", frameTbl);
    tbl.emplace("time", static_cast<toml::int64_t>(frame.timePoint));
}

template <typename T>
std::enable_if_t<std::is_same_v<T, KeyFrame<typename T::value_type>>>
deserialize(const toml::node& node, T& frame) {
    auto& tbl = *node.as_table();
    if (auto valueNode = tbl["value"]; valueNode) {
        deserialize<util::remove_cvref_t<decltype(frame.value)>>(
            *valueNode.node(), frame.value);
    }
    if (auto timeNode = tbl["time"]; timeNode.is_integer()) {
        frame.timePoint = timeNode.as_integer()->get();
    }
}

}  // namespace mirrow::serd::srefl

TEST_CASE("serd") {
    KeyFrame<int> frame;
    frame.value = 123;
    frame.timePoint = 456;
    toml::table tbl;
    mirrow::serd::srefl::serialize(frame, tbl);
}
