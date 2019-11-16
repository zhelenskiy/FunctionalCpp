//
// Created by zhele on 14.11.2019.
//

#ifndef FUNCTIONAL_QUICK_COPY_FUNCTION_H
#define FUNCTIONAL_QUICK_COPY_FUNCTION_H

#include <functional>

template<class T, class = void>
struct is_copy_simple {
  static const bool value = false;
};

template<class F>
class SimpleCopyFunction;

template<class F>
struct is_copy_simple<SimpleCopyFunction<F>> {
  static const bool value = true;
};

template<class T>
struct is_copy_simple<T, typename std::enable_if_t<std::is_trivially_copyable_v<T>>> {
  static const bool value = true;
};

template<class T>
constexpr bool is_copy_simple_v = is_copy_simple<T>::value;

template<class F>
class SimpleCopyFunction {
 public:

  template<class Lambda, class = typename std::enable_if_t<std::is_constructible_v<F, Lambda>>>
  /*explicit*/ SimpleCopyFunction(Lambda func) {
    if constexpr (std::is_same_v<Lambda, SimpleCopyFunction<F>>) {
      f_ = func.f_;
    } else if constexpr (is_copy_simple_v<Lambda> && sizeof func <= 4 * sizeof(size_t)) {
      f_ = func;
    } else if (is_not_null_function(func)) {
      f_ = [ptr = std::make_shared<Lambda>(std::move(func))](auto &&... args) { return (*ptr)(args...); };
    }
  }

  SimpleCopyFunction() : f_() {}

  explicit operator bool() const {
    return static_cast<bool>(f_);
  }

  template<class... Args>
  auto operator()(Args... args) const {
    return f_(std::forward<Args>(args)...);
  }

  using function_type = F;

 private:
  F f_;
};

#endif //FUNCTIONAL_QUICK_COPY_FUNCTION_H
