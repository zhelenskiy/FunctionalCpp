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

template<class T>
struct lock_controller {
  std::shared_ptr<std::vector<T>> data;
  std::atomic_bool isReady = false;
  std::mutex mutex = std::mutex();

  lock_controller() = default;
  ~lock_controller() = default;
  lock_controller(lock_controller &&) = delete;
  lock_controller(const lock_controller &) = delete;
};
/*std::vector<int> vec; auto ptr = std::make_shared<decltype(vec)>(vec); VectorHolder<int> seq{ptr, vec.begin(), vec.end()};*/

template<class T>
struct VectorHolder {
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  iterator begin, end;
  std::shared_ptr<lock_controller<T>> controller = std::make_shared<lock_controller<T>>();

  VectorHolder(iterator begin, iterator end, decltype(controller) controller)
      : begin(std::move(begin)), end(std::move(end)), controller(std::move(controller)) {
//    if(!this->controller)
//      throw std::exception();
  }

  VectorHolder(iterator begin, iterator end, std::shared_ptr<std::vector<T>> data)
      : begin(std::move(begin)), end(std::move(end)) {
    controller->data = std::move(data);
//    if(!controller)
//      throw std::exception();
  }

  explicit VectorHolder(std::vector<T> &&vector) {
    controller->data = std::make_shared<std::vector<T>>(std::move(vector));
    begin = controller->data->begin();
    end = controller->data->end();
//    if(!controller)
//      throw std::exception();
  }

  std::unique_lock<std::mutex> getGuard() {
    if (!controller->isReady.load())
      if (auto guard = std::unique_lock<std::mutex>(controller->mutex); !controller->isReady.load())
        return guard;
    return {};
  }

  void setReady() {
    controller->isReady = true;
  }

  size_t size() const {
    return end - begin;
  }
};

#endif //FUNCTIONAL_VECTORHOLDER_H
