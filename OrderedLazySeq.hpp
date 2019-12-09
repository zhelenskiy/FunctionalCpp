//
// Created by zhele on 18.06.2019.
//

#include "OrderedLazySeq.h"

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const Func &comp) const {
  return OrderedLazySeq(separateMore(classes_, comp),
                        hasSpecialPartialSkipHelper()
                        ? [*this, comp](wide_size_t count) {
                          auto[toBeSkipped, node] = std::apply(simpleSkip, applyPartialSkipHelper(count));
                          if (!node) {
                            return std::pair{toBeSkipped, equivClasses<T>()};
                          }
                          auto[smallerToBeSkipped, rest] = smartSkip(getHolder(node->first), toBeSkipped, comp);
                          return std::pair{smallerToBeSkipped, getClasses(rest) + separateMore(node->second, comp)};
                        }
                        : partial_skip_helper_t());
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy() const {
  return thenBy(std::less<T>());
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending(const Func &comp) const {
  return thenBy(descendingComparer(comp));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending() const {
  return thenByDescending(std::less<T>());
}

template<class T>
template<class ValueFunc, class Comparer, class, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const ValueFunc &func, const Comparer &comp) const {
  return keys(map([func = func](const T &item) { return std::pair{item, func(item)}; })
                  .thenBy([comp = comp](const auto &pair1, const auto &pair2) {
                    return comp(pair1.second, pair2.second);
                  }));
}

template<class T>
template<class ValueFunc, class R, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenBy(const ValueFunc &func) const {
  return thenBy(func, std::less<R>());
}

template<class T>
template<class ValueFunc, class Comparer, class, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending(const ValueFunc &func,
                                                                const Comparer &comp) const {
  return thenBy(func, descendingComparer(comp));
}

template<class T>
template<class ValueFunc, class R, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::thenByDescending(const ValueFunc &func) const {
  return thenByDescending(func, std::less<R>());
}

template<class T>
template<class Func, class>
auto OrderedLazySeq<T>::partition(typename std::vector<T>::iterator begin,
                                  typename std::vector<T>::iterator end, const Func &comp) {
  auto pivot = *(begin + getRandomIndex((size_t) (end - begin)));
  auto border1 = std::partition(begin, end, [comp = comp, pivot](const auto &item) { return comp(item, pivot); });
  auto border2 = std::find_if_not(border1, end, isEqualTo(std::move(pivot)));
  return std::tuple{std::pair{begin, border1}, std::pair{border1, border2}, std::pair{border2, end}};
}

template<class T>
//template<bool stable>
template<class Func, class>
equivClasses<T> OrderedLazySeq<T>::separateMore(const equivClasses<T> &seq, const Func &comp) {
  return getClasses(getHolders(seq).mapMany([comp](const auto &class_) { return separateMore(class_, comp); }));
  /*seq.mapMany([comp](const auto &class_) { return separateMore(getHolder(class_), comp); })*/
}

template<class T>
//template<bool stable>
template<class Func, class>
SliceHolderSeq<T> OrderedLazySeq<T>::separateMore(SliceHolder<T> holder, const Func &comp) {
  if (holder.getGuard()) {
    auto[begin, end] = std::tie(holder.begin, holder.end);
    if (begin == end) {
      return {};
    } else if (end - begin <= BUCKET_SIZE_FOR_STD_SORT_CALL) {
      std::stable_sort(begin, end, comp);
      std::vector<SliceHolder<T>> classes{};
      auto curBegin = begin;
      while (curBegin != end) {
        auto curEnd = std::find_if_not(curBegin, end, isEqualTo(*curBegin));
        classes.emplace_back(curBegin, curEnd, holder.controller);
        curBegin = curEnd;
      }
      holder.setReady();
      return makeLazy(std::move(classes));
    } else {
      auto[less, equal, greater] = partition(begin, end, comp);
      auto less_class = SliceHolder<T>(less.first, less.second, holder.controller->data);
      auto equal_class = SliceHolder<T>(equal.first, equal.second, holder.controller);
      equal_class.setReady();
      auto greater_class = SliceHolder<T>(greater.first, greater.second, holder.controller->data);
      /*parallel*/
      return SliceHolderSeq<T>([part = std::move(less_class), comp = comp] {
        return separateMore(part, comp);
      }) + (SliceHolder<T>(std::move(equal_class)) + SliceHolderSeq<T>([part = std::move(greater_class), comp = comp] {
        return separateMore(part, comp);
      }));
    }
  } else {
    return {holder};
  }
}

template<class T>
template<class Func, class R>
constexpr OrderedLazySeq<R> OrderedLazySeq<T>::map(const Func &func) const {
  auto nestedMap = [func = func](const auto &class_) { return class_.map(func); };
  return OrderedLazySeq<R>(
      classes_.map(nestedMap),
      hasSpecialPartialSkipHelper()
      ? [func = func, *this, nestedMap](wide_size_t count) {
        auto[toBeSkipped, rest] = applyPartialSkipHelper(count);
        return std::pair{toBeSkipped, rest.map(nestedMap)};
      }
      : typename OrderedLazySeq<R>::partial_skip_helper_t());
}

template<class T>
template<class Func, class>
std::pair<wide_size_t, SliceHolderSeq<T>> OrderedLazySeq<T>::smartSkip(const SliceHolder<T> &holder,
                                                                       wide_size_t count,
                                                                       const Func &comp) {
  if (count >= holder.size()) {
    return {count - holder.size(), {}};
  } else if (count == 0) {
    return {0, separateMore(holder, comp)};
  }
  auto[less, equal, greater] = partition(holder.begin, holder.end, comp);
  auto equal_class = SliceHolder<T>(equal.first, equal.second, holder.controller);
  equal_class.setReady();
  auto greater_class = SliceHolder<T>(greater.first, greater.second, holder.controller->data);
  auto less_size = less.second - less.first;
  auto greaterClasses =
      SliceHolderSeq<T>([part = greater_class, comp = comp] { return separateMore(part, comp); });
  if (count < less_size + equal_class.size()) {
    auto equalOrGreaterClasses = equal_class + greaterClasses;
    if (count < less_size) {
      auto less_class = SliceHolder<T>(less.first, less.second, holder.controller->data);
      auto[newCount, newClasses] = smartSkip(less_class, count, comp);
      return {newCount, newClasses + equalOrGreaterClasses};
    } else /* less.size() <= count < less.size() + equal.size()*/ {
      return {count - less_size, equalOrGreaterClasses};
    }
  } else /*count >= less.size() + equal.size()*/ {
    return smartSkip(std::move(greater_class), count - (equal_class.size() + less_size), comp);
  }
}

template<class T>
std::pair<wide_size_t, node_ptr<equivClass<T>>> OrderedLazySeq<T>::simpleSkip(wide_size_t count,
                                                                              const equivClasses<T> &items) {
  auto node = items.skipWhile([&count](const auto &items) {
    return items.count() <= count ? count -= items.count(), true : false;
  }).eval();
  return {count, node};
}

template<class T>
constexpr OrderedLazySeq<T>::OrderedLazySeq(const equivClasses<T> &classes,
                                            const partial_skip_helper_t &partialSkipHelper)
    : LazySeq<T>(join(classes).setSkipHelper(
    partialSkipHelper
    ? [partialSkipHelper](wide_size_t count) {
      auto[toBeSkipped, seq] = partialSkipHelper(count);
      return join(seq).applySkipHelper(toBeSkipped);
    }
    : typename LazySeq<T>::skip_helper_t())),
      classes_(classes),
      partialSkipHelper_(partialSkipHelper) {}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::rest() const {
  return skip(1);
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skip(wide_size_t count) const {
  return OrderedLazySeq<T>(
      equivClasses<T>([*this, count] {
        auto[toBeSkipped, node] = std::apply(simpleSkip, applyPartialSkipHelper(count));
        return equivClasses<T>(node && toBeSkipped ? (node->first = node->first.skip(toBeSkipped), node) : node);
      }),
      hasSpecialPartialSkipHelper()
      ? [*this, count](wide_size_t newCount) {
        return applyPartialSkipHelper(count < WIDE_SIZE_T_MAX - newCount ? count + newCount : WIDE_SIZE_T_MAX);
      }
      : partial_skip_helper_t());
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::filter(const Func &pred) const {
  return OrderedLazySeq<T>(classes_.map([pred = pred](const auto &seq) { return seq.filter(pred); }));
}

template<class T>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::take(wide_size_t count) const {
  return OrderedLazySeq<T>(
      getTakenClasses(classes_, count),
      hasSpecialPartialSkipHelper()
      ? [*this, count](wide_size_t skipCount) {
        auto[notSkippedYet, classes] = applyPartialSkipHelper(skipCount);
        auto wereAlreadySkipped = skipCount - notSkippedYet;
        classes = getTakenClasses(classes, count > wereAlreadySkipped ? count - wereAlreadySkipped : 0);
        return std::pair{notSkippedYet, classes};
      }
      : partial_skip_helper_t());
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::takeWhile(const Func &pred) const {
  return OrderedLazySeq<T>(getTakenWhileClasses(classes_, pred));
}

template<class T>
template<class Func, class>
constexpr OrderedLazySeq<T> OrderedLazySeq<T>::skipWhile(const Func &pred) const {
  return OrderedLazySeq<T>(getSkippedWhileClasses(classes_, pred));
}

template<class T>
constexpr equivClasses<T> OrderedLazySeq<T>::getTakenClasses(const equivClasses<T> &classes, wide_size_t count) {
  return classes.mapByNode([count](auto node) {
    if (count < node->first.count()) {
      node->first = node->first.take(count);
      node->second = equivClasses<T>();
    } else {
      node->second = getTakenClasses(node->second, count - node->first.count());
    }
    return node;
  });
}

template<class T>
template<class Func, class>
constexpr equivClasses<T> OrderedLazySeq<T>::getTakenWhileClasses(const equivClasses<T> &classes,
                                                                  const Func &pred) {
  return classes.mapByNode([pred = pred](auto node) {
    auto trues = node->first.takeWhile(pred).count();
    return node->first.count() == trues ? (node->second = getTakenWhileClasses(node->second, pred), *node)
                                        : std::pair{node->first.take(trues), equivClasses<T>()};
  });
}

template<class T>
template<class Func, class>
constexpr equivClasses<T> OrderedLazySeq<T>::getSkippedWhileClasses(const equivClasses<T> &classes,
                                                                    const Func &pred) {
  return classes.mapByNode([pred = pred](auto node) {
    node_ptr<T> found;
    for (; node && !(found = node->first.skipWhile(pred).eval()); node = node->second.eval());
    if (node) {
      node->first = LazySeq(found);
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
equivClasses<T> OrderedLazySeq<T>::getClasses(const SliceHolderSeq<T> &holders) {
  return holders.map([](const auto &holder) { return LazySeq(holder.begin, holder.end, holder.controller->data); });
}

template<class T>
SliceHolder<T> OrderedLazySeq<T>::getHolder(const equivClass<T> &class_) {
  return SliceHolder<T>(class_.toVector());
}

template<class T>
SliceHolderSeq<T> OrderedLazySeq<T>::getHolders(const equivClasses<T> &seq) {
  return seq.map(getHolder);
}

