//
// Created by zhele on 18.06.2019.
//

#include "OrderedLazySeq.h"

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const comparer<T> &comp) const {
  return OrderedLazySeq(separateMore(classes_, comp), wrapPartialSkipHelper(
      [*this, comp](wide_size_t count) {
        auto[toBeSkipped, node] = std::apply(simpleSkip, applyPartialSkipHelper(count));
        if (!node) {
          return std::pair{toBeSkipped, equivClasses<T>()};
        }
        auto[smallerToBeSkipped, rest] = smartSkip(node->first, toBeSkipped, comp);
        return std::pair{smallerToBeSkipped, rest + separateMore(node->second, comp)};
      }));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending(const comparer<T> &comp) const {
  return thenBy(descendingComparer<T>(comp));
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const std::function<R(T)> &func,
                                                      const comparer<R> &comp) const {
  return keys(map<std::pair<T, R>>([func](const T &item) { return std::pair{item, func(item)}; })
                  .thenBy([comp](const auto &pair1, const auto &pair2) { return comp(pair1.second, pair2.second); }));
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending(const std::function<R(T)> &func,
                                                                const comparer<R> &comp) const {
  return thenBy<R>(func, descendingComparer<R>(comp));
}

template<class T>
auto OrderedLazySeq<T>::partition(const equivClass<T> &items, const comparer<T> &comp) {
  equivClass<T> less, equal, greater;
  if (!items.empty()) {
    T pivot = items[getRandomIndex(0, items.size())];
    for (auto &item: items) {
      (comp(item, pivot)
       ? less
       : comp(pivot, item) ? greater : equal).emplace_back(std::move(item));
    }
  }
  return std::tuple{less, equal, greater};
}

template<class T>
//template<bool stable>
equivClasses<T> OrderedLazySeq<T>::separateMore(const equivClasses<T> &seq, const comparer<T> &comp) {
  return seq.template mapMany<equivClasses<T>>([comp](const equivClass<T> &vec) -> equivClasses<T> {
    if (vec.empty()) {
      return {};
    } else if (vec.size() <= BUCKET_SIZE_FOR_STD_SORT_CALL) {
      std::vector<equivClass<T>> classes{{}};
      for (auto &&item: stdSort(vec, comp)) {
        if (classes.back().empty() || !comp(classes.back().back(), item)) {
          classes.back().emplace_back(std::move(item));
        } else {
          classes.emplace_back(std::vector{std::move(item)});
        }
      }
      return makeLazy(std::move(classes));
    } else {
      auto[less, equal, greater] = partition(vec, comp);
      using classes_t = equivClasses<T>;
      /*parallel*/
      return classes_t([part = classes_t{std::move(less)}, comp] { return separateMore(part, comp); })
          + (classes_t{std::move(equal)}
              + classes_t([part = classes_t{std::move(greater)}, comp] { return separateMore(part, comp); }));
    }
  });
}

template<class T>
template<class R>
constexpr OrderedLazySeq<R> OrderedLazySeq<T>::map(const std::function<R(T)> &func) const {
  return OrderedLazySeq<R>(
      classes_.template map<equivClass<R>>(vectorMap(func)),
      OrderedLazySeq<R>::wrapPartialSkipHelper([func, *this](wide_size_t count) {
        auto[toBeSkipped, rest] = applyPartialSkipHelper(count);
        return std::pair{toBeSkipped, rest.template map<equivClass<R>>(vectorMap(func))};
      }));
}

template<class T>
template<class R>
std::function<equivClass<R>(equivClass<T>)> OrderedLazySeq<T>::vectorMap(const std::function<R(T)> &func) {
  return [func](const equivClass<T> &vec) {
    equivClass<R> res;
    res.reserve(vec.size());
    for (const auto &item: vec) {
      res.emplace_back(func(item));
    }
    return res;
  };
}

template<class T>
std::function<equivClass<T>(equivClass<T>)> OrderedLazySeq<T>::vectorFilter(const predicate<T> &pred) {
  return [pred](const equivClass<T> &vec) {
    equivClass<T> res;
    for (const auto &item: vec) {
      if (pred(item)) {
        res.emplace_back(item);
      }
    }
    return res;
  };
}

template<class T>
std::pair<wide_size_t, equivClasses<T>> OrderedLazySeq<T>::smartSkip(const equivClass<T> &items,
                                                                     wide_size_t count,
                                                                     const comparer<T> &comp) {
  if (count >= items.size()) {
    return {count - items.size(), {}};
  }
  auto[less, equal, greater] = partition(items, comp);
  if (count < less.size() + equal.size()) {
    auto equalOrGreaterClasses =
        separateMore(equivClasses<T>{std::move(greater)}, comp).emplaceFront(std::move(equal));
    if (count < less.size()) {
      auto[newCount, newClasses] = smartSkip(less, count, comp);
      return {newCount, newClasses + equalOrGreaterClasses};
    } else /* less.size() <= count < less.size() + equal.size()*/ {
      return {count - less.size(), equalOrGreaterClasses};
    }
  } else /*count >= less.size() + equal.size()*/ {
    return smartSkip(greater, count - less.size() - equal.size(), comp);
  }
}

template<class T>
equivClass<T> OrderedLazySeq<T>::stdSort(const equivClass<T> &items, const comparer<T> &comp) {
  auto items_copy = items;/*stable*/
  std::stable_sort(items_copy.begin(), items_copy.end(), comp);
  return items_copy;
}

template<class T>
std::pair<wide_size_t, node_ptr<equivClass<T>>> OrderedLazySeq<T>::simpleSkip(wide_size_t count,
                                                                              const equivClasses<T> &items) {
  auto node = items.eval();
  while (node && count >= node->first.size()) {
    count -= node->first.size();
    node = node->second.eval();
  }
  return std::pair{count, node};
}

template<class T>
template<class Lambda>
constexpr OrderedLazySeq<T>::OrderedLazySeq(const equivClasses<T> &classes,
                                            const Lambda &partialSkipHelper)
    : LazySeq<T>(join(classes).setSkipHelper(
    is_not_null_function(partialSkipHelper)
    ? typename LazySeq<T>::skip_helper_t([partialSkipHelper](wide_size_t count) {
      auto[toBeSkipped, seq] = partialSkipHelper(count);
      return join(seq).applySkipHelper(toBeSkipped);
    })
    : typename LazySeq<T>::skip_helper_t())),
      classes_(classes),
      partialSkipHelper_(partial_skip_helper_t(partialSkipHelper)) {}

template<class T>
constexpr OrderedLazySeq<T>::OrderedLazySeq(const equivClasses<T> &classes)
    : OrderedLazySeq<T>(classes, partial_skip_helper_t()) {}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::rest() const {
  return skip(1);
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skip(wide_size_t count) const {
  return OrderedLazySeq<T>(
      equivClasses<T>([*this, count] {
        auto[toBeSkipped, node] = std::apply(simpleSkip, applyPartialSkipHelper(count));
        if (toBeSkipped && node) {
          auto &vec = node->first;
          std::move(vec.begin() + toBeSkipped, vec.end(), vec.begin());
          vec.resize(vec.size() - toBeSkipped);
        }
        return equivClasses<T>(node);
      }), wrapPartialSkipHelper(
          [*this, count](wide_size_t newCount) {
            return applyPartialSkipHelper(count < WIDE_SIZE_T_MAX - newCount ? count + newCount : WIDE_SIZE_T_MAX);
          }));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::filter(const predicate<T> &pred) const {
  return OrderedLazySeq<T>(classes_.template map<equivClass<T>>(vectorFilter(pred)));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::take(wide_size_t count) const {
  return OrderedLazySeq<T>(
      getTakenClasses(classes_, count), wrapPartialSkipHelper(
          [*this, count](wide_size_t skipCount) {
            auto[notSkippedYet, classes] = applyPartialSkipHelper(skipCount);
            auto wereAlreadySkipped = skipCount - notSkippedYet;
            classes = getTakenClasses(classes, count > wereAlreadySkipped ? count - wereAlreadySkipped : 0);
            return std::pair{notSkippedYet, classes};
          }));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::takeWhile(const predicate<T> &pred) const {
  return OrderedLazySeq<T>(getTakenWhileClasses(classes_, pred));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skipWhile(const predicate<T> &pred) const {
  return OrderedLazySeq<T>(getSkippedWhileClasses(classes_, pred));
}

template<class T>
constexpr equivClasses<T> OrderedLazySeq<T>::getTakenClasses(const equivClasses<T> &classes, wide_size_t count) {
  return classes.template mapByNode<equivClass<T>>([count](auto node) {
    if (count < node->first.size()) {
      node->first.resize(count);
      node->second = equivClasses<T>();
    } else {
      node->second = getTakenClasses(node->second, count - node->first.size());
    }
    return node;
  });
}

template<class T>
constexpr equivClasses<T> OrderedLazySeq<T>::getTakenWhileClasses(const equivClasses<T> &classes,
                                                                  const predicate<T> &pred) {
  return classes.template mapByNode<equivClass<T>>([pred](auto node) {
    wide_size_t count = 0;
    for (; count < node->first.size() && pred(node->first[count]); ++count);
    if (count < node->first.size()) {
      node->first.resize(count);
      node->second = equivClasses<T>();
    } else {
      node->second = getTakenWhileClasses(node->second, pred);
    }
    return node;
  });
}

template<class T>
constexpr equivClasses<T> OrderedLazySeq<T>::getSkippedWhileClasses(const equivClasses<T> &classes,
                                                                    const predicate<T> &pred) {
  return classes.template mapByNode<equivClass<T>>([pred](auto node) {
    decltype(node->first.begin()) found;
    while (node) {
      auto &vec = node->first;
      if (!vec.empty() && (found = std::find_if_not(vec.begin(), vec.end(), pred)) != vec.end()) { break; }
      node = node->second.eval();
    }
    if (node && found != node->first.begin()) {
      std::move(found, node->first.end(), node->first.begin());
      node->first.resize(node->first.end() - found);
    }
    return node;
  });
}

template<class T>
constexpr bool OrderedLazySeq<T>::hasSpecialPartialSkipHelper() const {
  return static_cast<bool>(partialSkipHelper_);
}

template<class T>
constexpr std::pair<wide_size_t, equivClasses<T>> OrderedLazySeq<T>::applyPartialSkipHelper(wide_size_t count) const {
  return hasSpecialPartialSkipHelper() ? partialSkipHelper_(count) : std::pair{count, classes_};
}

template<class T>
template<class Lambda>
typename OrderedLazySeq<T>::partial_skip_helper_t
OrderedLazySeq<T>::wrapPartialSkipHelper(const Lambda &partialSkipHelper) {
  return partial_skip_helper_t(partialSkipHelper);
}
