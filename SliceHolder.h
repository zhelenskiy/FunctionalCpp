//
// Created by zhele on 29.11.2019.
//

#ifndef FUNCTIONAL_SliceHolder_H
#define FUNCTIONAL_SliceHolder_H

#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <optional>
#include <atomic>
#include <functional>

#include "LazySeq.h"

template<class T>
class DataController {
    using iterator = typename std::vector<T>::iterator;
    std::shared_ptr<std::vector<T>> data_;
    iterator begin_, end_;
    mutable std::atomic_bool isReady_ = true;
    mutable std::function<void(const DataController<T> &)> preAction_ = nullptr;
    mutable std::function<std::pair<iterator, wide_size_t>(const DataController<T> &, wide_size_t)> getter_ =
            [](const DataController<T> &this_, wide_size_t ind) {
                return ind >= this_.size() ? std::pair{this_.end(), 0ull} : std::pair{this_.begin() + ind,
                                                                                      this_.size() - ind};
            }; // Start of evaluated data and its size since nth
    mutable std::mutex mutex_ = std::mutex();

public:
    explicit DataController(std::vector<T> &&data, decltype(preAction_) &&preAction = nullptr)
            : data_(std::make_shared<std::vector<T>>(std::move(data))),
              begin_(data_->begin()),
              end_(data_->end()),
              isReady_(!static_cast<bool>(preAction)),
              preAction_(std::move(preAction)) {}

    DataController(decltype(data_) data, iterator begin, iterator end, decltype(preAction_) &&preAction = nullptr)
            : data_(std::move(data)), begin_(std::move(begin)), end_(std::move(end)),
              isReady_(!static_cast<bool>(preAction)),
              preAction_(std::move(preAction)) {}

    ~DataController() = default;

    [[nodiscard]] iterator begin() const {
        return begin_;
    }

    [[nodiscard]] iterator end() const {
        return end_;
    }

    [[nodiscard]] decltype(data_) data() const {
        return data_;
    }

    [[nodiscard]] wide_size_t size() const {
        return (size_t) (end_ - begin_);
    }

    [[nodiscard]] std::unique_lock<std::mutex> getGuard() const {
        if (!isReady())
            if (auto guard = std::unique_lock<std::mutex>(mutex_); !isReady())
                return guard;
        return {};
    }

    [[nodiscard]] bool isReady() const {
        return isReady_.load();
    }

    void setReady(bool value = true) const {
        isReady_ = value;
    }

    auto get(wide_size_t count) const {
        if (preAction_) {
            preAction_(*this);
            preAction_ = nullptr;
        }
        return getter_(*this, count);
    }

    auto iterators() const {
        return std::pair{begin_, end_};
    }

    // {'holders'...} are consecutive parts of the partition of the holder '*this'. '*this' will be modified.
    template<class... Holders>
    void setSubHolders(const std::unique_lock<std::mutex> &lock, const Holders &... holders) const {
        auto updateState = [holders...](const DataController<T> &this_) {
            this_.setReady((holders->isReady() && ...));
        };
        updateState(*this);
        getter_ = [holders..., updateState](const DataController<T> &this_, wide_size_t index) {
            if (index >= this_.size()) {
                return std::pair{this_.end(), 0ull};
            } else if (this_.isReady()) {
                return std::pair{this_.begin() + index, this_.size() - index};
            }
            auto res = chooseHolder(index, holders...);
            updateState(this_);
            return res;
        };
    }

    template<class Holder, class... Holders>
    static auto chooseHolder(wide_size_t index, const Holder &first, const Holders &... rest) {
        auto res = first->get(index);
        if constexpr (sizeof...(rest)) {
            if (!res.second) {
                return chooseHolder(index - first->size(), rest...);
            }
        }
        return res;
    }
};

template<class T>
class SliceHolder {
    std::shared_ptr<DataController<T>> controller;

public:

    SliceHolder(SliceHolder<T> &&) noexcept = default;

    SliceHolder(const SliceHolder<T> &) = default;

    template<class... Args, class = std::enable_if_t<std::is_constructible_v<DataController<T>, Args...>>>
    explicit SliceHolder(Args &&... args)
            : controller(std::make_shared<DataController<T>>(std::forward<Args>(args)...)) {}

    DataController<T> *operator->() {
        return &*controller;
    }

    const DataController<T> *operator->() const {
        return &*controller;
    }

    LazySeq<T> toLazySeq() const {
        return controller->isReady()
               ? LazySeq<T>(controller->begin(), controller->end(), controller->data())
               : LazySeq<T>([*this] { return startWith(0); })
                       .setSkipHelper([*this](wide_size_t count) {
                           return count >= controller->size() ? std::pair{count - controller->size(), LazySeq<T>()}
                                                              : std::pair{0ull, startWith(count)};
                       });
    }

private:
    LazySeq<T> startWith(wide_size_t index) const {
        auto[begin, count] = controller->get(index);
        return count
               ? (LazySeq<T>(begin, begin + count, controller->data())
                  + LazySeq<T>([*this, index, count] { return startWith(index + count); }))
               : LazySeq<T>();
    }
};

#endif //FUNCTIONAL_SLICE_HOLDER_H
