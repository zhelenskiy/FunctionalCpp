//
// Created by zhele on 23.11.2019.
//

#ifndef FUNCTIONAL_SMARTFUNCTION_H
#define FUNCTIONAL_SMARTFUNCTION_H

#include <iostream>
#include <variant>
#include <functional>
#include <memory>

inline const size_t MAX_SMART_FUNCTION_SIZE = 4 * sizeof(int);

template<class FuncType>
class SmartFunction {
  using StdFunctionType = std::function<FuncType>;
  std::variant<StdFunctionType, std::shared_ptr<StdFunctionType>> func_ = StdFunctionType();

 public:

  SmartFunction() noexcept = default;
  SmartFunction(const SmartFunction &) = default;
  SmartFunction(SmartFunction &&) noexcept = default;
  SmartFunction<FuncType> &operator=(SmartFunction &&) noexcept = default;
  SmartFunction<FuncType> &operator=(const SmartFunction &) = default;

  template<class Func, class = std::enable_if_t<std::is_constructible_v<StdFunctionType, Func>>>
  /*implicit*/ SmartFunction(Func f) { // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    if constexpr (std::is_same_v<Func, StdFunctionType>
        || (sizeof(f) <= MAX_SMART_FUNCTION_SIZE && std::is_trivially_copyable_v<Func>)) {
//      std::cout << "bruh\n";
      func_ = StdFunctionType(std::move(f));
    } else if constexpr (!std::is_constructible_v<Func, std::nullptr_t>) {
//      std::cout << "yeap\n";
      func_ = std::make_shared<StdFunctionType>(std::move(f));
    } else if (f) {
//      std::cout << "cool\n";
      func_ = std::make_shared<StdFunctionType>(std::move(f));
    }
//    std::cout << "null\n";
  }

  explicit operator bool() const noexcept {
    auto if_func = std::get_if<StdFunctionType>(&func_);
    return if_func ? static_cast<bool>(*if_func) : true;
  }

  template<class... Args>
  auto operator()(Args &&... args) const {
    return std::visit([&](const auto &f) {
      if constexpr (std::is_same_v<std::decay_t<decltype(f)>, StdFunctionType>) {
        return f(std::forward<Args>(args)...);
      } else {
        return (*f)(std::forward<Args>(args)...);
      }
    }, func_);
  }
};

#endif //FUNCTIONAL_SMARTFUNCTION_H
