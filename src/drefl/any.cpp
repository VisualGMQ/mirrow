#include "mirrow/drefl/any.hpp"
#include "mirrow/drefl/invoke_util.hpp"

namespace mirrow {

namespace drefl {

basic_any::basic_any(internal::type_node* type, const any_methods* methods)
    : type_(type), methods_(methods) {}

void basic_any::reset() {
    if (methods_->basic().destroy) {
        methods_->basic().destroy(instance_);
        instance_ = nullptr;
        methods_ = nullptr;
    }
}

std::optional<unsigned long long> basic_any::try_cast_uintegral() const {
    return methods_->numeric_cast().try_cast_uintegral(instance_);
}

std::optional<long long> basic_any::try_cast_integral() const {
    return methods_->numeric_cast().try_cast_integral(instance_);
}

std::optional<double> basic_any::try_cast_floating_point() const {
    return methods_->numeric_cast().try_cast_floating_pointer(instance_);
}

void basic_any::deep_set(const basic_any& o) {
    methods_->basic().deep_set(instance_, o);
}

void basic_any::push_back(const basic_any& data) {
    if (methods_ && methods_->container().push_back) {
        methods_->container().push_back(instance_, data);
    }
}

size_t basic_any::size() {
    if (methods_ && methods_->container().size) {
        return methods_->container().size(instance_);
    } else {
        return 0;
    }
}

type_info basic_any::elem_type() {
    if (methods_ && methods_->container().size) {
        return methods_->container().elem_type();
    }
    return type_info{nullptr};
}

void basic_any::travel_elements(const std::function<void(any&)>& func) {
    if (methods_->container().travel) {
        methods_->container().travel(instance_, func);
    }
}

void basic_any::travel_elements_by_ref(
    const std::function<void(reference_any&)>& func) {
    if (methods_->container().travel_by_ref) {
        methods_->container().travel_by_ref(instance_, func);
    }
}

any::any(const any& o) : basic_any(o) {
    if (methods_ && methods_->basic().copy) {
        methods_->basic().copy(instance_, o.instance_);
    }
}

any::~any() {
    if (methods_ && methods_->basic().destroy) {
        methods_->basic().destroy(instance_);
    }
}

}  // namespace drefl

}  // namespace mirrow