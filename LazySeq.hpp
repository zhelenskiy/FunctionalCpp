
//
// Created by zhele on 06.06.2019.
//

#include "LazySeq.h"
namespace {
template<class T>
class adder {
  template<class Z, Z (Z::*)() = &Z::operator+>
  struct wrapper {};

  template<class C>
  static std::true_type check(wrapper<C> *);

  template<class C>
  static std::false_type check(...);

 public:
  static const bool hasPlus = std::is_same<decltype(check<T>(0)), std::true_type>::value;

  static T invoke(T item, wide_size_t count) {
    if constexpr (hasPlus) {
      return item + count;
    } else {
      while (count) {
        ++item;
        --count;
      }
      return item;
    }
  }
};
}

template<class T>
constexpr LazySeq<T>::LazySeq(fabric_ptr<T> fabric, skip_helper_ptr_t skipHelper)
    : evaluator_(std::move(fabric)), skipHelper_(std::move(skipHelper)) {}

template<class T>
constexpr node_ptr<T> LazySeq<T>::eval() const {
  return (*evaluator_)();
}

template<class T>
constexpr bool LazySeq<T>::isEmpty() const {
  return !eval();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::filter(const predicate<T> &pred) const {
  return mapByNode<T>([pred](node_ptr<T> pair) -> node_ptr<T> {
    bool correct;
    while (!(correct = pred(pair->first)) && pair->second.setAndCheckIfNotEmpty(pair));
    if (correct) {
      pair->second = pair->second.filter(pred);
    }
    return pair;
  });
}
template<class T>
constexpr LazySeq<T> LazySeq<T>::filter(const T &item) const {
  return filter(isEqualTo(item));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::filterByIndex(const predicate<indexed_t<T>> &pred) const {
  return values(getIndexed().filter(pred));
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::map(const std::function<R(T)> &func) const {
  return mapByNode<R>([func](node_ptr<T> pair) -> node<R> {
    return {func(pair->first), pair->second.map(func)};
  }).setSkipHelper(
      hasSpecialSkipHelper()
      ? [func, *this](wide_size_t count) {
        auto[notSkippedYet, rest] = applySkipHelper(count);
        return std::pair{notSkippedYet, rest.template map<R>(func)};
      }
      : typename LazySeq<R>::skip_helper_t());
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::mapByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().map(func);
}

template<class T>
template<class Iter, class... Args>
constexpr LazySeq<T>::LazySeq(Iter first, Iter last, Args... captured)
    : LazySeq((std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>
               ? range(first, (wide_size_t) std::distance(first, last))
               : infiniteRange(first).takeWhile([last, captured...](Iter iter) { return iter != last; }))
                  .template map<T>([captured...](Iter iter) { return *iter; })) {}

template<class T>
T LazySeq<T>::reduce(const std::function<T(T, T)> &func, T &&defaultValue) const {
  auto pair = std::make_shared<node<T>>(defaultValue, LazySeq<T>());
  return setAndCheckIfNotEmpty(pair) ? pair->second.template reduce<T>(pair->first, func)
                                     : std::forward<T>(defaultValue);
}

template<class T>
template<class R>
R LazySeq<T>::reduce(const R &init, const std::function<R(R, T)> &func) const {
  R res = init;
  for (const auto &item: *this) {
    res = func(std::move(res), item);
  }
  return res;
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::castTo() const {
  return map<R>([](const T &a) { return (R) a; });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::staticCastTo() const {
  return map<R>([](const T &a) { return static_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::reinterpretCastTo() const {
  return map<R>([](const T &a) { return reinterpret_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::constCastTo() const {
  return map<R>([](const T &a) { return const_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::dynamicCastTo() const {
  return map<R>([](const T &a) { return dynamic_cast<R>(a); });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::concat(const LazySeq<T> &other) const {
  return LazySeq<T>([*this, other]() -> node_ptr<T> {
    node_ptr<T> pair;
    return setAndCheckIfNotEmpty(pair) ? (pair->second = pair->second.concat(other), pair) : other.eval();
  }).setSkipHelper(
      hasSpecialSkipHelper() || other.hasSpecialSkipHelper()
      ? [*this, other](wide_size_t count) {
        auto[thisToBeSkipped, thisRest] = applySkipHelper(count);
        return thisToBeSkipped ? other.applySkipHelper(thisToBeSkipped) : std::pair{thisToBeSkipped, thisRest + other};
      }
      : skip_helper_t());
}

template<class T>
constexpr LazySeq<T> join(const LazySeq<LazySeq<T>> &seq) {
  return LazySeq<T>([seq] {
    auto rest = seq;
    while (auto external = rest.eval()) {
      if (auto internal = external->first.eval()) {
        internal->second += join(external->second);
        return internal;
      }
      rest = external->second;
    }
    return node_ptr<T>();
  }).setSkipHelper([seq](wide_size_t count) {
    if (!count) {
      return std::pair{count, join(seq)};
    }
    for (auto node = seq.eval(); node; node = node->second.eval()) {
      auto[toBeSkipped, rest] = node->first.applySkipHelper(count);
      if (!toBeSkipped) {
        return std::pair{toBeSkipped, rest + join(node->second)};
      }
      count = toBeSkipped;
    }
    return std::pair{count, LazySeq<T>()};
  });
}

template<class Container>
constexpr auto join(const LazySeq<Container> &seq) {
  return seq.template mapMany<LazySeq<typename Container::value_type>>(
      [](const auto &container) { return makeLazy(std::move(container)); });
}

template<class Container>
constexpr auto join(const ReversedLazySeq<Container> &seq) {
  return ReversedLazySeq(join(LazySeq(seq)), join(LazySeq(seq.reverse())));
}

template<class T>
constexpr LazyIterator<T> LazySeq<T>::begin() const {
  return const_iterator(*this);
}

template<class T>
constexpr LazyIterator<T> LazySeq<T>::end() const {
  return LazyIterator<T>(LazySeq<T>());
}

template<class T>
template<class R>
bool LazySeq<T>::operator==(const LazySeq<R> &other) const {
  if (evaluator_.get() == other.evaluator_.get()) {
    return true;
  }
  auto iter1 = LazyIterator(*this), iter2 = LazyIterator(other);
  for (; iter1 != LazySeq<T>::end() && iter2 != LazySeq<R>::end(); ++iter1, ++iter2) {
    if (!(*iter1 == *iter2)) {
      return false;
    }
  }
  return iter1 == LazySeq<T>::end() && iter2 == LazySeq<R>::end();
}

template<class T>
template<class R>
bool LazySeq<T>::operator!=(const LazySeq<R> &other) const {
  return !(*this == other);
}

template<class LazySeq>
constexpr auto operator+(const LazySeq &a, const LazySeq &b) {
  return a.concat(b);
}

template<class T>
constexpr T LazySeq<T>::first() const {
  return eval()->first;
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::rest() const {
  return eval()->second;
}

template<class T>
constexpr LazyIterator<T>::LazyIterator(const LazySeq<T> &seq) : evaluated(seq.eval()) {}

template<class T>
constexpr LazyIterator<T> &LazyIterator<T>::operator++() {
  return *this = LazyIterator(evaluated->second);
}

template<class T>
const LazyIterator<T> LazyIterator<T>::operator++(int) {
  auto res = *this;
  ++*this;
  return res;
}

template<class T>
constexpr T LazyIterator<T>::operator->() const {
  return evaluated->first;
}

template<class T>
constexpr bool LazyIterator<T>::operator==(const LazyIterator<T> &other) const {
  return evaluated == other.evaluated;
}

template<class T>
constexpr bool LazyIterator<T>::operator!=(const LazyIterator<T> &other) const {
  return !(*this == other);
}

template<class T>
constexpr T LazyIterator<T>::operator*() const {
  return evaluated->first;
}

template<class T>
LazyIterator<T> LazyIterator<T>::operator+(wide_size_t count) const {
  return count ? LazyIterator<T>(evaluated->second.skip(count - 1)) : *this;
}

template<class T>
LazyIterator<T> LazyIterator<T>::operator+=(wide_size_t count) {
  return *this = *this + count;
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::mapByNode(const std::function<node<R>(node_ptr<T>)> &f) const {
  return mapByNode<R>([f](node_ptr<T> node1) -> node_ptr<R> {
    return std::make_shared<node<R>>(f(node1));
  });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::mapByNode(const std::function<node_ptr<R>(node_ptr<T>)> &f) const {
  return LazySeq<R>([*this, f]() -> node_ptr<R> {
    node_ptr<T> old;
    return setAndCheckIfNotEmpty(old) ? f(old) : node_ptr<R>(nullptr);
  });
}

template<class T>
LazySeq<T>::LazySeq(std::initializer_list<T> list)
    : evaluator_(makeLazy(std::move(std::vector<T>(list))).evaluator_) {}

template<class T>
constexpr LazySeq<T>::LazySeq(const node<T> &node1) : LazySeq(std::make_shared<node<T>>(node1)) {}

template<class T>
constexpr LazySeq<T>::LazySeq(const node_ptr<T> &nodePtr) : LazySeq<T>(constantly(nodePtr)) {}

template<class T>
constexpr bool LazySeq<T>::setAndCheckIfNotEmpty(node_ptr<T> &res) const {
  // Its usage is based on priority of evaluating of ternary operator: condition is evaluated before.
  return static_cast<bool>(res = eval());
}

template<class T>
constexpr LazySeq<T>::LazySeq() : LazySeq(node_ptr<T>(nullptr)) {}

template<class T>
constexpr LazySeq<T> LazySeq<T>::repeat(wide_size_t count) const {
  return range<wide_size_t>(0, count).mapMany<LazySeq<T >>(constantly(*this))
      .setSkipHelper([count, *this](wide_size_t skipCount) {
        auto[toBeSkipped, rest] = applySkipHelper(skipCount);
        if (!toBeSkipped) {
          return std::pair{toBeSkipped, count ? rest + repeat(count - 1) : LazySeq<T>()};
        }
        const auto bucketCount = skipCount - toBeSkipped;
        const auto fullCount = bucketCount * count;
        if (fullCount <= skipCount) {
          return std::pair{skipCount - fullCount, LazySeq<T>()};
        }
        const auto resFullCount = fullCount - skipCount;
        return std::pair{(wide_size_t) 0, (skipCount % bucketCount != 0 ? skip(skipCount % bucketCount) : LazySeq<T>())
            + repeat(resFullCount / bucketCount)};
      });
}

template<class T>
constexpr LazySeq<T> operator*(const LazySeq<T> &a, wide_size_t count) {
  return a.repeat(count);
}

template<class T>
template<class R>
constexpr LazySeq<std::pair<T, R>> LazySeq<T>::operator*(const LazySeq<R> &other) const {
  return mapMany<LazySeq<std::pair<T, R>>>([other](const T &first) -> LazySeq<std::pair<T, R>> {
    return other.template map<std::pair<T, R>>(
        [first](const R &second) -> std::pair<T, R> { return {first, second}; }
    );
  });
}

template<class T>
constexpr bool LazySeq<T>::every(const predicate<T> &pred) const {
  return none(negate(pred));
}

template<class T>
constexpr bool LazySeq<T>::every(const T &item) const {
  return every(isEqualTo(item));
}

template<class T>
constexpr bool LazySeq<T>::everyByIndex(const predicate<indexed_t<T>> &pred) const {
  return getIndexed().every(pred);
}

template<class T>
constexpr bool LazySeq<T>::any(const predicate<T> &pred) const {
  return !none(pred);
}

template<class T>
constexpr bool LazySeq<T>::any(const T &item) const {
  return any(isEqualTo(item));
}

template<class T>
constexpr bool LazySeq<T>::anyByIndex(const predicate<indexed_t<T>> &pred) const {
  return !noneByIndex(pred);
}

template<class T>
constexpr bool LazySeq<T>::none(const predicate<T> &pred) const {
  return filter(pred).isEmpty();
}

template<class T>
constexpr bool LazySeq<T>::none(const T &item) const {
  return none(isEqualTo(item));
}

template<class T>
constexpr bool LazySeq<T>::noneByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(pred).isEmpty();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::take(wide_size_t count) const {
  return mapByNode<T>([count](node_ptr<T> pair) -> node_ptr<T> {
    return count ? std::make_shared<node<T>>(pair->first, pair->second.take(count - 1))
                 : node_ptr<T>(nullptr);
  }).setSkipHelper(
      hasSpecialSkipHelper()
      ? [count, *this](wide_size_t countToSkip) {
        if (countToSkip >= count) {
          return std::pair{countToSkip - count, LazySeq<T>()};
        }
        auto[notSkipped, rest] = applySkipHelper(countToSkip); //notSkipped <= countToSkip < count
        return std::pair{notSkipped, rest.take(count - countToSkip)};
      }
      : skip_helper_t());
  //Implementation is not via 'takeWhileByIndex' because 'range'-implementation is via 'take'
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::takeWhile(const predicate<T> &pred) const {
  return mapByNode<T>([pred](node_ptr<T> pair) -> node_ptr<T> {
    return pred(pair->first) ? std::make_shared<node<T>>(pair->first, pair->second.takeWhile(pred))
                             : node_ptr<T>(nullptr);
  });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::takeWhile(const T &item) const {
  return takeWhile(isEqualTo(item));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::takeWhileByIndex(const predicate<indexed_t<T>> &pred) const {
  return values(getIndexed().takeWhile(pred));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::skip(wide_size_t count) const {
  return LazySeq<T>([*this, count] { return applySkipHelper(count).second; })
      .setSkipHelper(
          hasSpecialSkipHelper()
          ? [count, *this](auto newCount) {
            return applySkipHelper(count < WIDE_SIZE_T_MAX - newCount ? count + newCount : WIDE_SIZE_T_MAX);
          }
          : skip_helper_t());
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::skipWhile(const predicate<T> &pred) const {
  return mapByNode<T>([pred](node_ptr<T> pair) -> node_ptr<T> {
    while (pred(pair->first) && pair->second.setAndCheckIfNotEmpty(pair));
    return pair;
  });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::skipWhile(const T &item) const {
  return skipWhile(isEqualTo(item));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::skipWhileByIndex(const predicate<indexed_t<T>> &pred) const {
  return values(getIndexed().skipWhile(pred));
}

template<class T>
wide_size_t LazySeq<T>::count() const {
  return WIDE_SIZE_T_MAX - applySkipHelper(WIDE_SIZE_T_MAX).first;
}

template<class T>
wide_size_t LazySeq<T>::count(const predicate<T> &pred) const {
  return filter(pred).count();
}

template<class T>
wide_size_t LazySeq<T>::count(const T &item) const {
  return count(isEqualTo(item));
}

template<class T>
wide_size_t LazySeq<T>::countByIndex(const predicate<indexed_t<T>> &pred) const {
  return getIndexed().count(pred);
}

template<class T>
auto LazySeq<T>::sum() const {
  return reduce(std::plus<T>());
}

template<class T>
template<class R>
auto LazySeq<T>::sum(const std::function<R(T)> &f) const {
  return map(f).sum();
}

template<class T>
template<class R>
auto LazySeq<T>::sumByIndex(const std::function<R(indexed_t<T>)> &f) const {
  return mapByIndex(f).sum();
}

template<class T>
auto LazySeq<T>::subtract() const {
  return reduce(std::minus<T>());
}

template<class T>
template<class R>
auto LazySeq<T>::subtract(const std::function<R(T)> &f) const {
  return map(f).subtract();
}

template<class T>
template<class R>
auto LazySeq<T>::subtractByIndex(const std::function<R(indexed_t<T>)> &f) const {
  return mapByIndex(f).subtract();
}

template<class T>
auto LazySeq<T>::multiply() const {
  return reduce(std::multiplies<T>(), static_cast<T>(1));
}

template<class T>
template<class R>
auto LazySeq<T>::multiply(const std::function<R(T)> &f) const {
  return map(f).multiply();
}

template<class T>
template<class R>
auto LazySeq<T>::multiplyByIndex(const std::function<R(indexed_t<T>)> &f) const {
  return mapByIndex(f).multiply();
}

template<class T>
auto LazySeq<T>::divide() const {
  return reduce(std::divides<T>(), static_cast<T>(1));
}

template<class T>
template<class R>
auto LazySeq<T>::divide(const std::function<R(T)> &f) const {
  return map(f).divide();
}

template<class T>
template<class R>
auto LazySeq<T>::divideByIndex(const std::function<R(indexed_t<T>)> &f) const {
  return mapByIndex(f).divide();
}

template<class T>
T LazySeq<T>::min(const comparer<T> &comp) const {
  return reduce([&comp](const T &a, const T &b) { return comp(a, b) ? a : b; });
}

template<class T>
template<class R>
T LazySeq<T>::min(const std::function<R(T)> &f, const comparer<R> &comp) const {
  return map<std::pair<T, R>>([&f](const T &item) { return std::pair{item, f(item)}; })
      .min([&comp](const auto &pair1, const auto &pair2) { return comp(pair1.second, pair2.second); }).first;
}

template<class T>
template<class R>
T LazySeq<T>::minByIndex(const std::function<R(indexed_t<T>)> &f, const comparer<R> &comp) const {
  return getIndexed().template min<R>(f, comp).second;
}

template<class T>
constexpr comparer<T> descendingComparer(const comparer<T> &comp) {
  return [comp](const T &a, const T &b) { return comp(b, a); };
}

template<class T>
T LazySeq<T>::max(const comparer<T> &comp) const {
  return min(descendingComparer(comp));
}

template<class T>
template<class R>
T LazySeq<T>::max(const std::function<R(T)> &f, const comparer<R> &comp) const {
  return min(f, descendingComparer(comp));
}

template<class T>
template<class R>
T LazySeq<T>::maxByIndex(const std::function<R(indexed_t<T>)> &f, const comparer<R> &comp) const {
  return getIndexed().template max<R>(f, comp).second;
}

template<class T>
std::pair<T, T> LazySeq<T>::minMax(const comparer<T> &comp) const {
  auto firstNode = eval();
  return firstNode->second.template reduce<std::pair<T, T>>(
      std::pair{firstNode->first, firstNode->first},
      [&comp](const auto &pair, const T &cur) {
        return std::pair{comp(cur, pair.first) ? cur : pair.first,
                         comp(pair.first, cur) ? cur : pair.second};
      });
}

template<class T>
template<class R>
std::pair<T, T> LazySeq<T>::minMax(const std::function<R(T)> &f, const comparer<R> &comp) const {
  auto ans = map<std::pair<T, R>>([&f](const T &item) { return std::pair{item, f(item)}; })
      .minMax([&comp](const auto &pair1, const auto &pair2) { return comp(pair1.second, pair2.second); });
  return std::pair{ans.first.first, ans.second.first};
}

template<class T>
template<class R>
std::pair<T, T> LazySeq<T>::minMaxByIndex(const std::function<R(indexed_t<T>)> &f, const comparer<R> &comp) const {
  auto res = getIndexed().template minMax<R>(f, comp);
  return std::pair{res.first.second, res.second.second};
}

template<class T>
std::string LazySeq<T>::toString(const std::string &separator) const {
  std::stringstream stream;
  auto pair = eval();
  if (!pair) {
    return "";
  }
  stream << pair->first;
  for (const auto &item: pair->second) {
    stream << separator << item;
  }
  return stream.str();
}

template<class T>
T LazySeq<T>::last() const {
  return reduce([](const T &, const T &b) -> T {
    return b;
  });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLast() const {
  return LazySeq<T>([*this] {
    auto first = eval();
    if (!first) {
      return LazySeq<T>();
    }
    return keys(LazySeq<std::pair<T, node_ptr<T>>>(
        std::pair{first->first, first->second.eval()},
        [](const auto &pair) {
          auto restNode = pair.second;
          return std::pair{restNode->first, restNode->second.eval()};
        }).takeWhile([](const auto &pair) { return static_cast<bool>(pair.second); }));
  }).setSkipHelper(
      hasSpecialSkipHelper()
      ? [*this](wide_size_t count) {
        auto[toBeSkipped, rest] = applySkipHelper(count);
        auto node = rest.eval();
        return node ? std::pair{(wide_size_t) 0, LazySeq(node).butLast()}
                    : std::pair{toBeSkipped < WIDE_SIZE_T_MAX ? toBeSkipped + 1 : WIDE_SIZE_T_MAX, LazySeq<T>()};
      }
      : skip_helper_t());
}

template<class T>
template<class R>
constexpr LazySeq<std::pair<T, R>> LazySeq<T>::match(const LazySeq<R> &other) const {
  return mapByNode<std::pair<T, R>>([other](node_ptr<T> pair) -> node_ptr<std::pair<T, R>> {
    auto otherPair = other.eval();
    return otherPair ? std::make_shared<node<std::pair<T, R>>>(std::pair{pair->first, otherPair->first},
                                                               pair->second.match(otherPair->second))
                     : node_ptr<std::pair<T, R>>(nullptr);
  }).setSkipHelper(
      hasSpecialSkipHelper() || other.hasSpecialSkipHelper()
      ? [*this, other](wide_size_t count) {
        auto[thisToBeSkipped, thisRest] = hasSpecialSkipHelper() ? applySkipHelper(count) : std::pair{count, *this};
        auto[otherToBeSkipped, otherRest] = other.hasSpecialSkipHelper() ? other.applySkipHelper(count)
                                                                         : std::pair{count, other};
        return std::pair{std::max(thisToBeSkipped, otherToBeSkipped), thisRest.match(otherRest)};
      }
      : typename LazySeq<std::pair<T, R>>::skip_helper_t());
}

template<class T>
template<class R>
std::string LazySeq<T>::toString(const std::function<R(T)> &f, const std::string &separator) const {
  return map(f).toString(separator);
}

template<class T>
template<class R>
std::string LazySeq<T>::toStringByIndex(const std::function<R(indexed_t<T>)> &f, const std::string &separator) const {
  return getIndexed().toString(f, separator);
}

template<class T>
constexpr LazySeq<T> operator*(wide_size_t count, const LazySeq<T> &seq) {
  return seq * count;
}

template<class T>
constexpr LazySeq<T>::LazySeq(const fabric<T> &fab) : LazySeq(std::make_shared<fabric<T>>(fab)) {}

template<class T>
std::vector<T> LazySeq<T>::toVector() const {
  return toContainer<std::vector<T>>();
}

template<class T>
template<class R>
std::vector<R> LazySeq<T>::toVector(const std::function<R(T)> &func) const {
  return map(func).toVector();
}

template<class T>
template<class R>
std::vector<R> LazySeq<T>::toVectorByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().toVector(func);
}

template<class T>
template<class K, class V>
std::map<K, V> LazySeq<T>::toMap(const std::function<std::pair<K, V>(T)> &func) const {
  return toContainer<std::map<K, V>, std::pair<K, V>>(func);
}

template<class T>
template<class K, class V>
std::map<K, V> LazySeq<T>::toMapByIndex(const std::function<std::pair<K, V>(indexed_t<T>)> &func) const {
  return getIndexed().toMap(func);
}

template<class T>
template<class K, class V>
std::map<K, V> LazySeq<T>::toMap(const std::function<K(T)> &keyFunc, const std::function<V(T)> &valueFunc) const {
  return toContainer<std::map<K, V>, K, V>(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::map<K, V> LazySeq<T>::toMapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                        const std::function<V(indexed_t<T>)> &valueFunc) const {
  return getIndexed().toMap(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::unordered_map<K, V> LazySeq<T>::toUnorderedMap(const std::function<std::pair<K, V>(T)> &func) const {
  return toContainer<std::unordered_map<K, V>, std::pair<K, V>>(func);
}

template<class T>
template<class K, class V>
std::unordered_map<K, V> LazySeq<T>::toUnorderedMapByIndex(
    const std::function<std::pair<K, V>(indexed_t<T>)> &func) const {
  return getIndexed().toUnorderedMap(func);
}

template<class T>
template<class K, class V>
std::unordered_map<K, V> LazySeq<T>::toUnorderedMap(const std::function<K(T)> &keyFunc,
                                                    const std::function<V(T)> &valueFunc) const {
  return toContainer<std::unordered_map<K, V>, K, V>(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::unordered_map<K, V> LazySeq<T>::toUnorderedMapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                                           const std::function<V(indexed_t<T>)> &valueFunc) const {
  return getIndexed().toUnorderedMap(keyFunc, valueFunc);
}

template<class T>
std::set<T> LazySeq<T>::toSet() const {
  return toContainer<std::set<T>>();
}

template<class T>
template<class R>
std::set<R> LazySeq<T>::toSet(const std::function<R(T)> &func) const {
  return map(func).toSet();
}

template<class T>
template<class R>
std::set<R> LazySeq<T>::toSetByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().toSet(func);
}

template<class T>
template<class R>
std::unordered_set<R> LazySeq<T>::toUnorderedSet(const std::function<R(T)> &func) const {
  return map(func).toUnorderedSet();
}

template<class T>
template<class R>
std::unordered_set<R> LazySeq<T>::toUnorderedSetByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().toUnorderedSet(func);
}

template<class T>
std::unordered_set<T> LazySeq<T>::toUnorderedSet() const {
  return toContainer<std::unordered_set<T>>();
}

template<class T>
template<class K, class V>
std::multimap<K, V> LazySeq<T>::toMultimap(const std::function<std::pair<K, V>(T)> &func) const {
  return toContainer<std::multimap<K, V>, std::pair<K, V>>(func);
}

template<class T>
template<class K, class V>
std::multimap<K, V> LazySeq<T>::toMultimapByIndex(const std::function<std::pair<K, V>(indexed_t<T>)> &func) const {
  return getIndexed().toMultimap(func);
}

template<class T>
template<class K, class V>
std::multimap<K, V> LazySeq<T>::toMultimap(const std::function<K(T)> &keyFunc,
                                           const std::function<V(T)> &valueFunc) const {
  return toContainer<std::multimap<K, V>, K, V>(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::multimap<K, V> LazySeq<T>::toMultimapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                                  const std::function<V(indexed_t<T>)> &valueFunc) const {
  return getIndexed().toMultimap(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::unordered_multimap<K, V> LazySeq<T>::toUnorderedMultimap(const std::function<std::pair<K, V>(T)> &func) const {
  return toContainer<std::unordered_multimap<K, V>, std::pair<K, V>>(func);
}

template<class T>
template<class K, class V>
std::unordered_multimap<K, V> LazySeq<T>::toUnorderedMultimapByIndex(
    const std::function<std::pair<K, V>(indexed_t<T>)> &func) const {
  return getIndexed().toUnorderedMultimap(func);
}

template<class T>
template<class K, class V>
std::unordered_multimap<K, V> LazySeq<T>::toUnorderedMultimap(const std::function<K(T)> &keyFunc,
                                                              const std::function<V(T)> &valueFunc) const {
  return toContainer<std::unordered_multimap<K, V>, K, V>(keyFunc, valueFunc);
}

template<class T>
template<class K, class V>
std::unordered_multimap<K, V> LazySeq<T>::toUnorderedMultimapByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                                                     const std::function<V(indexed_t<T>)> &valueFunc) const {
  return getIndexed().toUnorderedMultimap(keyFunc, valueFunc);
}

template<class T>
std::multiset<T> LazySeq<T>::toMultiset() const {
  return toContainer<std::multiset<T>>();
}

template<class T>
template<class R>
std::multiset<R> LazySeq<T>::toMultiset(const std::function<R(T)> &func) const {
  return map(func).toMultiset();
}

template<class T>
template<class R>
std::multiset<R> LazySeq<T>::toMultisetByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().toMultiset(func);
}

template<class T>
std::unordered_multiset<T> LazySeq<T>::toUnorderedMultiset() const {
  return toContainer<std::unordered_multiset<T>>();
}

template<class T>
template<class R>
std::unordered_multiset<R> LazySeq<T>::toUnorderedMultiset(const std::function<R(T)> &func) const {
  return map(func).toUnorderedMultiset();
}

template<class T>
template<class R>
std::unordered_multiset<R> LazySeq<T>::toUnorderedMultisetByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().toUnorderedMultiset(func);
}

template<class T>
template<class Container>
auto LazySeq<T>::toContainer() const {
  return Container(begin(), end());
}

template<class T>
template<class Container, class R>
auto LazySeq<T>::toContainer(const std::function<R(T)> &func) const {
  return map(func).template toContainer<Container>();
}

template<class T>
template<class Container, class R>
auto LazySeq<T>::toContainerByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return mapByIndex(func).template toContainer<Container>();
}

template<class T>
template<class Container, class K, class V>
auto LazySeq<T>::toContainer(const std::function<K(T)> &keyFunc, const std::function<V(T)> &valueFunc) const {
  return toContainer<Container, std::pair<K, V>>([keyFunc, valueFunc](const T &item) {
    return std::pair{keyFunc(item), valueFunc(item)};
  });
}

template<class T>
template<class Container, class K, class V>
auto LazySeq<T>::toContainerByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                    const std::function<V(indexed_t<T>)> &valueFunc) const {
  return toContainerByIndex<Container, std::pair<K, V>>([keyFunc, valueFunc](const indexed_t<T> &item) {
    return std::pair{keyFunc(item), valueFunc(item)};
  });
}

template<class T>
template<class R>
auto LazySeq<T>::average() const {
  auto sum = map<std::pair<T, wide_size_t>>([](const T &item) { return std::pair{std::move(item), (wide_size_t) 1}; })
      .reduce([](const auto &a, const auto &b) { return std::pair{a.first + b.first, a.second + b.second}; });
  return static_cast<R>(sum.first) / sum.second;
}

template<class T>
template<class R, class C>
auto LazySeq<T>::average(const std::function<R(T)> &func) const {
  return map(func).template average<C>();
}

template<class T>
template<class R, class C>
auto LazySeq<T>::averageByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return mapByIndex(func).template average<C>();
}

template<class T>
ReversedLazySeq<T> LazySeq<T>::reverse() const {
  return ReversedLazySeq(*this);
}

template<class T>
template<class R>
constexpr ReversedLazySeq<R> LazySeq<T>::reverse(const std::function<R(T)> &func) const {
  return map(func).reverse();
}

template<class T>
template<class R>
constexpr ReversedLazySeq<R> LazySeq<T>::reverseByIndex(const std::function<R(indexed_t<T>)> &func) const {
  return getIndexed().reverse(func);
}

template<class T>
T LazySeq<T>::last(const predicate<T> &pred) const {
  return filter(pred).last();
}

template<class T>
T LazySeq<T>::lastByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(pred).last();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLast(const predicate<T> &pred) const {
  return filter(pred).butLast();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLast(const T &item) const {
  return butLast(isEqualTo(item));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLastByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(pred).butLast();
}

template<class T>
constexpr T LazySeq<T>::first(const predicate<T> &pred) const {
  return filter(pred).first();
}

template<class T>
constexpr T LazySeq<T>::firstByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(pred).first();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::emplaceFront(const T &value) const {
  return LazySeq<T>(std::pair{value, *this}).setSkipHelper(
      hasSpecialSkipHelper()
      ? [*this, value](wide_size_t count) {
        return count ? applySkipHelper(count - 1) : std::pair{(wide_size_t) 0, emplaceFront(value)};
      }
      : skip_helper_t());
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::emplaceBack(const T &value) const {
  return concat(LazySeq<T>(std::pair{value, LazySeq<T>()}));
}

template<class T>
constexpr bool LazySeq<T>::contains(const T &value) const {
  return any(value);
}

template<class T>
template<class Container>
constexpr auto LazySeq<T>::mapMany(const std::function<Container(T)> &func) const {
  return join(map(func));
}

template<class T>
template<class Container>
constexpr auto LazySeq<T>::mapManyByIndex(const std::function<Container(indexed_t<T>)> &func) const {
  return getIndexed().mapMany(func);
}

template<class T>
constexpr LazySeq<T> operator+(const LazySeq<T> &a, const T &item) {
  return a.emplaceBack(item);
}

template<class T>
constexpr T LazySeq<T>::itemAt(wide_size_t index) const {
  return skip(index).first();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::unite(const LazySeq<T> &other) const {
  return concat(other).distinct();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::distinct() const {
  return LazySeq<T>([*this] {
    std::unordered_set<T> set{};
    std::vector<T> vector{};
    for (const auto &item: *this) {
      if (!set.count(item)) {
        set.insert(item);
        vector.emplace_back(item);
      }
    }
    return makeLazy(std::move(vector));
  });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::intersect(const LazySeq<T> &other) const {
  return LazySeq<T>([*this, other] {
    const auto set_ptr = std::make_shared<std::unordered_set<T>>(std::move(other.toUnorderedSet()));
    return filter([set_ptr](const T &a) -> bool { return static_cast<bool>(set_ptr->count(a)); });
  });
}
//TODO copy-paste
template<class T>
constexpr LazySeq<T> LazySeq<T>::except(const LazySeq<T> &other) const {
  return LazySeq<T>([*this, other] {
    const auto set_ptr = std::make_shared<std::unordered_set<T>>(std::move(other.toUnorderedSet()));
    return filter([set_ptr](const T &a) -> bool { return !set_ptr->count(a); });
  });
}

template<class T>
template<class S, class R>
constexpr LazySeq<R> LazySeq<T>::match(const LazySeq<S> &other, const std::function<R(std::pair<T, S>)> &func) const {
  return match(other).map(func);
}

template<class T>
template<class S, class R>
constexpr LazySeq<R> LazySeq<T>::matchByIndex(const LazySeq<S> &other,
                                              const std::function<R(indexed_t<std::pair<T, S>>)> &func) const {
  return match(other).mapByIndex(func);
}

template<class T>
LazySeq<LazySeq<T>> LazySeq<T>::groupBy(wide_size_t portion) const {
  return LazySeq<LazySeq<T >>(
      [*this, portion] {
        auto evaled = eval();
        if (!evaled) {
          return node_ptr<LazySeq<T>>();
        }
        auto curGroup = portion ? LazySeq(std::pair{evaled->first, evaled->second.take(portion - 1)}) : LazySeq<T>();
        auto otherGroups = (portion ? evaled->second.skip(portion - 1) : *this).groupBy(portion);
        return std::make_shared<node<LazySeq<T>>>(curGroup, otherGroups);
      }).setSkipHelper(
      [*this, portion](wide_size_t skipCount) {
        if (!portion || !skipCount) {
          return std::pair{skipCount, groupBy(portion)};
        }
        auto[toBeSkipped, rest] = applySkipHelper(skipCount < WIDE_SIZE_T_MAX / portion
                                                  ? skipCount * portion
                                                  : WIDE_SIZE_T_MAX);
        return std::pair{toBeSkipped / portion, rest.groupBy(portion)};
      });
}

template<class T>
template<class F>
LazySeq<std::pair<F, LazySeq<T>>> LazySeq<T>::groupBy(const std::function<F(T)> &keyFinder) const {
  std::unordered_map<F, std::vector<T>> map;
  for (const auto &item: *this) {
    map[keyFinder(item)].emplace_back(item);
  }
  return makeLazy(std::move(map)).template map<std::pair<F, LazySeq<T>>>(
      [](const auto &pair) { return std::pair{pair.first, makeLazy(std::move(pair.second))}; });
}

template<class T>
template<class F, class S, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupBy(const std::function<F(T)> &keyFinder,
                                             const std::function<S(T)> &valueFunc,
                                             const std::function<R(LazySeq<S>)> &seqFunc) const {
  return groupBy(keyFinder).template map<std::pair<F, R>>([valueFunc, seqFunc](const auto &pair) {
    return std::pair{pair.first, seqFunc(pair.second.map(valueFunc))};
  });
}

template<class T>
template<class F, class S, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupByIndexBy(const std::function<F(indexed_t<T>)> &keyFinder,
                                                    const std::function<S(indexed_t<T>)> &valueFunc,
                                                    const std::function<R(LazySeq<S>)> &seqFunc) const {
  return getIndexed().groupBy(keyFinder, valueFunc, seqFunc);
}

template<class T>
template<class F, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupBy(const std::function<F(T)> &keyFinder,
                                             const std::function<R(LazySeq<T>)> &seqFunc) const {
  return groupBy(keyFinder).template map<std::pair<F, R>>([seqFunc](const auto &pair) {
    return std::pair{pair.first, seqFunc(pair.second)};
  });
}

template<class T>
template<class F, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupByIndexBy(const std::function<F(indexed_t<T>)> &keyFinder,
                                                    const std::function<R(LazySeq<indexed_t<T>>)
                                                    > &seqFunc) const {
  return
      getIndexed()
          .
              groupBy(keyFinder, seqFunc
          );
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const comparer<T> &comp) const {
  return makeOrdered().thenBy(comp);
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending(const comparer<T> &comp) const {
  return makeOrdered().thenByDescending(comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const std::function<R(T)> &func, const comparer<R> &comp) const {
  return makeOrdered().thenBy(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                                  const comparer<R> &comp) const {
  return getIndexed().orderBy(func, comp);
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::makeOrdered() const {
  return OrderedLazySeq<T>(equivClasses<T>([*this] { return equivClasses<T>{toVector()}; }),
                           [*this](size_t count) { return std::pair{count, equivClasses<T>{toVector()}}; });
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending(const std::function<R(T)> &func,
                                                          const comparer<R> &comp) const {
  return makeOrdered().thenByDescending(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByDescendingByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                                              const comparer<R> &comp) const {
  return getIndexed().orderByDescending(func, comp);
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::rest(const predicate<T> &pred) const {
  return filter(pred).rest();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::restByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(pred).rest();
}

template<class T>
constexpr LazySeq<T> operator+(const T &item, const LazySeq<T> &seq) {
  return seq.emplaceFront(item);
}

template<class T>
constexpr LazySeq<T> &LazySeq<T>::operator+=(const LazySeq<T> &other) {
  return *this = *this + other;
}

template<class T>
constexpr LazySeq<T> &LazySeq<T>::operator+=(const T &item) {
  return *this = *this + item;
}

template<class T>
constexpr LazySeq<T> &LazySeq<T>::operator*=(wide_size_t count) {
  return *this = *this * count;
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::mapIf(const std::function<T(T)> &func, const predicate<T> &pred) const {
  return map<T>([func, pred](const T &a) { return pred(a) ? func(a) : a; });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::mapIfByIndex(const std::function<T(indexed_t<T>)> &func,
                                              const predicate<indexed_t<T>> &pred) const {
  return mapByIndex<T>([func, pred](const indexed_t<T> &pair) { return pred(pair) ? func(pair) : pair.second; });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::remove(const predicate<T> &pred) const {
  return filter(negate(pred));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::remove(const T &item) const {
  return remove(isEqualTo(item));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::removeByIndex(const predicate<indexed_t<T>> &pred) const {
  return filterByIndex(negate(pred));
}

template<class T>
constexpr wide_size_t LazySeq<T>::indexOf(const T &item, wide_size_t index) {
  return getIndexed().filter([&item](const auto &cur) { return cur.second == item; }).itemAt(index).first;
}

template<class T>
constexpr LazySeq<indexed_t<T>> LazySeq<T>::getIndexed() const {
  return indexed(*this);
}

template<class T>
constexpr LazySeq<T>::LazySeq(const T &initializer, const std::function<T(T)> &next)
    : LazySeq(node<T>{initializer, LazySeq(
    [next, initializer] { return LazySeq(next(initializer), next).eval(); }
)}) {}

template<class T>
constexpr LazySeq<T>::LazySeq(wide_size_t count, const T &value) : LazySeq(count * LazySeq{value}) {}

template<class T>
template<class R>
bool LazySeq<T>::operator<(const LazySeq<R> &other) const {
  if (evaluator_.get() == other.evaluator_.get()) {
    return false;
  }
  auto iter2 = other.begin();
  for (auto iter1 = begin(); iter1 != end() && iter2 != other.end(); ++iter1, ++iter2) {
    if (*iter1 != *iter2) {
      return *iter1 < *iter2;
    }
  }
  return iter2 != other.end();
}

template<class T>
template<class R>
bool LazySeq<T>::operator>(const LazySeq<R> &other) const {
  return other < *this;
}

template<class T>
template<class R>
bool LazySeq<T>::operator<=(const LazySeq<R> &other) const {
  return !(*this > other);
}

template<class T>
template<class R>
bool LazySeq<T>::operator>=(const LazySeq<R> &other) const {
  return !(*this < other);
}

template<class T>
template<class R>
constexpr void LazySeq<T>::foreach(const std::function<R(T)> &func) const {
  for (const auto &item: *this) {
    func(item);
  }
}

template<class T>
template<class R>
constexpr void LazySeq<T>::foreachByIndex(const std::function<R(indexed_t<T>)> &func) const {
  getIndexed().foreach(func);
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::rest(const T &item) const {
  return rest(isEqualTo(item));
}

namespace {
template<class T, wide_size_t n>
struct Pow {
  constexpr static auto invoke(const LazySeq<T> &seq) {
    using TFirst = typename decltype(Pow<T, n / 2>::invoke(seq))::value_type;
    using TSecond = typename decltype(Pow<T, n - n / 2>::invoke(seq))::value_type;
    using TRes = decltype(std::tuple_cat(std::declval<TFirst>(), std::declval<TSecond>()));
    return (Pow<T, n / 2>::invoke(seq) * Pow<T, n - n / 2>::invoke(seq))
        .template map<TRes>([](const auto &pair) { return std::tuple_cat(pair.first, pair.second); });
  }
};

template<class T>
struct Pow<T, 1> {
  constexpr static auto invoke(const LazySeq<T> &seq) {
    return seq.template map<std::tuple<T>>(std::make_tuple<T>);
  }
};

template<class T>
struct Pow<T, 0> {
  constexpr static auto invoke(const LazySeq<T> &seq) {
    return seq.map(std::make_tuple<>);
  }
};
}

template<class T>
template<wide_size_t n>
constexpr auto LazySeq<T>::pow() const {
  return Pow<T, n>::invoke(*this);
}

template<class T>
constexpr LazySeq<T>::LazySeq(const std::function<LazySeq<T>()> &generator)
    : LazySeq(LazySeq([generator] { return generator().eval(); })
                  .setSkipHelper([generator](wide_size_t count) { return generator().applySkipHelper(count); })) {}

template<class T>
constexpr LazySeq<T>::operator bool() const {
  return !isEmpty();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::setSkipHelper(const skip_helper_t &specialSkip) const {
  return LazySeq<T>(evaluator_, specialSkip ? std::make_shared<skip_helper_t>(specialSkip) : skip_helper_ptr_t());
}

template<class T>
constexpr bool LazySeq<T>::hasSpecialSkipHelper() const {
  return static_cast<bool>(skipHelper_);
}

template<class T>
auto LazySeq<T>::applySkipHelper(wide_size_t count) const {
  if (skipHelper_) {
    return (*skipHelper_)(count);
  } else {
    auto node = eval();
    while (count && node) {
      --count;
      node = node->second.eval();
    }
    return std::pair{count, LazySeq<T>(node)};
  }
}

template<class T>
constexpr LazySeq<T> range(const T &start, wide_size_t count) {
  return infiniteRange(start).take(count);
}

template<class T>
constexpr LazySeq<T> infiniteRange(const T &start) {
  return LazySeq<T>(start, increment<T>).setSkipHelper(
      [start](wide_size_t count) { return std::pair{(wide_size_t) 0, infiniteRange(adder<T>::invoke(start, count))}; });
}

template<class Container>
auto makeLazy(Container &&container) {
  using bareContainer = typename std::remove_reference<Container>::type;
  using Iter = typename bareContainer::const_iterator;
  using T = typename bareContainer::value_type;
  if constexpr(std::is_base_of<LazySeq<T>, Container>::value) {
    return container;
  } else if constexpr(std::is_same<Container, std::initializer_list<T>>::value) {
    return LazySeq(container);
  } else {
    auto ptr = std::make_shared<bareContainer>(std::forward<Container>(container));
    return LazySeq<T>(ptr->begin(), ptr->end(), ptr);
  }
}

template<class T>
constexpr auto square(const T &seq) {
  return seq * seq;
}

template<class T>
std::ostream &operator<<(std::ostream &out, const LazySeq<T> &seq) {
  return out << seq.toString();
}

template<class T>
constexpr T identity(const T &x) {
  return x;
}

template<class... Args>
constexpr std::function<bool(Args...)> negate(const std::function<bool(Args...)> &pred) {
  return [pred](const Args &... args) { return !pred(args...); };
}

template<class Container>
LazySeq<size_t> indexesOf(const Container &container) {
  return range<size_t>(0, container.size());
}

template<class T>
LazySeq<wide_size_t> indexesOf(const LazySeq<T> &seq) {
  return keys(indexed(seq));
}

template<class T>
constexpr LazySeq<std::pair<wide_size_t, T>> indexed(const LazySeq<T> &seq) {
  return indexes().match(seq);
}

LazySeq<wide_size_t> indexes(wide_size_t start) {
  return infiniteRange<wide_size_t>(start);
}

LazySeq<natural_t> naturalNumbers() {
  return indexes(1);
}

LazySeq<integer_t> integerNumbers() {
  auto mapper = [](const auto &seq) {
    return seq.template mapMany<LazySeq<integer_t>>(
        [](wide_size_t index) { return LazySeq{(integer_t) index, -(integer_t) index}; });
  };
  return mapper(naturalNumbers())
      .setSkipHelper([mapper](wide_size_t count) {
        return std::pair{(wide_size_t) 0, mapper(naturalNumbers().skip(count / 2)).skip(count % 2)};
      })
      .emplaceFront(0);
}

//Lazy bfs of Calkin-Wilf tree
//https://ru.wikipedia.org/wiki/Дерево_Калкина_—_Уилфа
//https://en.wikipedia.org/wiki/Calkin–Wilf_tree
LazySeq<rational_t> positiveRationalNumbers() {
  auto nextGenerator = [](rational_t q) -> rational_t {
    return {q.second, q.first / q.second * q.second * 2 - q.first + q.second};
  };
  return LazySeq<rational_t>(rational_t{1, 1}, nextGenerator).setSkipHelper(
      [nextGenerator](wide_size_t count) {
        std::function<natural_t(wide_size_t)> fusc;
        fusc = [&fusc](wide_size_t count) {
          return count <= 1 ? count
                            : count % 2 == 0
                              ? fusc(count / 2)
                              : fusc(count / 2) + fusc(count / 2 + 1);
        };
        return std::pair{(wide_size_t) 0,
                         LazySeq<rational_t>(rational_t{fusc(count + 1), fusc(count + 2)}, nextGenerator)};
      });
}

LazySeq<rational_t> rationalNumbers() {
  auto mapper = [](const auto &seq) {
    return seq
        .template mapMany<LazySeq<rational_t>>([](rational_t r) { return LazySeq{r, rational_t{-r.first, r.second}}; });
  };
  return mapper(positiveRationalNumbers())
      .setSkipHelper([mapper](wide_size_t count) {
        return std::pair{(wide_size_t) 0, mapper(positiveRationalNumbers().skip(count / 2)).skip(count % 2)};
      })
      .emplaceFront(rational_t{0, 1});
}

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<K> keys(const LazySeq<std::pair<K, V>> &seq) {
  return seq.template map<K>([](const auto &pair) { return pair.first; });
}

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<V> values(const LazySeq<std::pair<K, V>> &seq) {
  return seq.template map<V>([](const auto &pair) { return pair.second; });
}

template<class T>
constexpr predicate<T> isEqualTo(const T &item) {
  return partial(std::equal_to<T>(), item);
}

template<class T>
constexpr predicate<T> isNotEqualTo(const T &item) {
  return negate(isEqualTo(item));
}

template<class T>
constexpr predicate<T> isLessThan(const T &item) {
  return [item](const T &other) { return other < item; };
}

template<class T>
constexpr predicate<T> isGreaterThan(const T &item) {
  return partial(std::less<T>(), item);
}

template<class T>
constexpr predicate<T> isNotLessThan(const T &item) {
  return negate(isLessThan(item));
}

template<class T>
constexpr predicate<T> isNotGreaterThan(const T &item) {
  return negate(isGreaterThan(item));
}

template<class T>
constexpr predicate<T> dividesBy(const T &n) {
  return [n](const T &m) { return m % n == 0; };
}

template<class T>
constexpr bool even(const T &item) {
  return item % 2 == 0;
}

template<class T>
constexpr bool odd(const T &item) {
  return !even(item);
}

template<class T>
constexpr auto constantly(const T &item) {
  return [item](const auto &...) { return item; };
}

template<class T>
constexpr T increment(T obj) {
  return ++obj;
}

template<class T>
constexpr T decrement(T obj) {
  return --obj;
}

template<class T>
constexpr LazySeq<T> powersByFixedBase(const T &base) {
  return LazySeq<T>(1, partial(std::multiplies<T>(), base));
}

template<class T>
constexpr LazySeq<T> factorials() {
  return values(LazySeq<std::pair<T, T>>(
      std::pair<T, T>{1, 1},
      [](const auto &pair) { return std::pair{pair.first + 1, pair.second * (pair.first + 1)}; }));
}

template<class T>
constexpr T fibonacci(const T &n) {
//  std::cout << "fib(" << n << ")" << std::endl;
  return n <= 2
         ? (n + 1) / 2
         : n % 2 == 0 ? square(fibonacci(n / 2 + 1)) - square(fibonacci(n / 2 - 1))
                      : square(fibonacci(n / 2 + 1)) + square(fibonacci(n / 2));
}

template<class T>
constexpr LazySeq<T> fibonacciSeq() {
  auto seq = [](const std::pair<T, T> &start) {
    return LazySeq<std::pair<T, T>>(start,
                                    [](const auto &pair) { return std::pair{pair.second, pair.first + pair.second}; });
  };
  return keys(seq({(T) 0, (T) 1}).setSkipHelper(
      [seq](wide_size_t index) {
//        std::cout << index << ":" << std::endl << fibonacci(index) << " " << fibonacci(index + 1) << std::endl;
        return std::pair{(wide_size_t) 0, seq(std::pair<T, T>{fibonacci(index), fibonacci(index + 1)})};
      }
  ));
}

template<class T>
constexpr LazySeq<T> powersByFixedExponent(const T &exponent) {
  return infiniteRange<T>(0)
      .template map<T>([exponent](const T &base) { return LazySeq<T>{base}.repeat(exponent).multiply(); });
}

template<class T>
constexpr LazySeq<T> identitySeq(const T &item) {
  return infiniteRange(0).map<T>(constantly(item));
}

template<class... Args>
LazySeq<wide_size_t> randomNumbers(Args... args) {
  return infiniteRange(0).map<wide_size_t>([args...](wide_size_t) { return getRandomIndex(args...); });
}

template<class F, class... Args>
constexpr decltype(auto) partial(const F &f, const Args &... args) {
  return [f, args...](const auto &... otherArgs) { return f(args..., otherArgs...); };
}
