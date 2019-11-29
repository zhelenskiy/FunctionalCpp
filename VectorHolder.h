//
// Created by zhele on 29.11.2019.
//

#ifndef FUNCTIONAL_VECTORHOLDER_H
#define FUNCTIONAL_VECTORHOLDER_H

#include <memory>
#include <vector>
#include <mutex>
#include <optional>
#include <atomic>

struct controller_t {
  std::atomic_bool isReady = false;
  std::mutex mutex = std::mutex();
};
/*std::vector<int> vec; auto ptr = std::make_shared<decltype(vec)>(vec); VectorHolder<int> seq{ptr, vec.begin(), vec.end()};*/

template<class T>
struct VectorHolder {
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  std::shared_ptr<std::vector<T>> data;
  iterator begin, end;
  std::shared_ptr<controller_t> controller = std::make_shared<controller_t>();

  explicit VectorHolder(std::vector<T> &&vector)
      : data(std::make_shared<std::vector<T>>(std::move(vector))), begin(data->begin()), end(data->end()) {}

  std::unique_lock<std::mutex> getGuard() {
    if (!controller->isReady.load())
      if (auto guard = std::unique_lock<std::mutex>(controller->mutex); !controller->isReady.load())
        return guard;
    return {};
  }
};

#endif //FUNCTIONAL_VECTORHOLDER_H
