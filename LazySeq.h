//
// Created by zhele on 06.06.2019.
//

#ifndef FUNCTIONAL_LAZY_SEQ_H
#define FUNCTIONAL_LAZY_SEQ_H

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

#include "SmartFunction.h"

template<class T>
class LazySeq;

template<class T>
class OrderedLazySeq;

template<class T>
class ReversedLazySeq;

template<class T>
class LazyIterator;

template<class T>
using node = std::pair<T, LazySeq<T>>;
template<class T>
using node_ptr = std::optional<node<T>>;
template<class T>
using fabric = SmartFunction<node_ptr<T>()>;

using wide_size_t = unsigned long long;
#define WIDE_SIZE_T_MAX UINT64_MAX
using natural_t = unsigned long long;
using integer_t = signed long long;
using rational_t = std::pair<integer_t, natural_t>;
template<class T>
using indexed_t = std::pair<wide_size_t, T>;

template<class T>
constexpr T identity(const T &x);

template<class Func, class R, class... Args>
using when_is_function = std::enable_if_t<std::is_invocable_r_v<R, Func, Args...>>;

template<class Func, class... Args>
using ResType = decltype(std::declval<Func>()(std::declval<Args>()...));

template<class Func, class... Args>
using when_is_predicate = when_is_function<Func, bool, Args...>;

template<class Func, class T>
using when_is_comparer = when_is_predicate<Func, T, T>;

#include "SliceHolder.h"

template<class T>
class LazySeq {
public:
    //DONE get rid of useless recursion
    //DONE orderBy functions
    //TODO get rid of copying data
    //DONE from init_list

    //DONE operators
    //DONE Range
    //DONE Repeat
    //DONE Bad filter
    //DONE from/toSTL functions
    //DONE OrderBy
    //DONE get rid of necessity of T()
    //DONE ::evaluator_
    //DONE rvalue bug fixing
    //DONE make public private
    //TODO add versions with specified (or default) comparer
    //DONE add versions with .map based on index
    //TODO add parallel functions support
    //TODO check laziness with natural numbers
    //TODO add ...OrDefault method versions
    //DONE simplify lambdas (c++14)
    //TODO const auto& -> auto&& && check with 'valgrind --tool=massif'
    //TODO identity<...> -> identity
    //TODO ...ByIndex -> auto deduce with 'if constexpr'
    //TODO make node_ptr easy copyable (node_ptr<std::vector> -> :-( )
    //TODO set smart functions to check: emptiness, [having fast skipHelpers], time of their evaluating, length

    typedef LazyIterator<T> const_iterator;
    typedef T value_type;
    typedef wide_size_t size_type;
    typedef signed long long difference_type;
    typedef std::allocator<T> allocator_type;
    typedef T &reference;
    typedef const T &const_reference;
    typedef typename std::allocator_traits<allocator_type>::pointer pointer;
    typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

    [[nodiscard]] constexpr const_iterator begin() const;

    [[nodiscard]] constexpr const_iterator end() const;

    using skip_helper_t = SmartFunction<std::pair<wide_size_t, LazySeq<T>>(wide_size_t)>;

    constexpr /*implicit */LazySeq<T>(std::initializer_list<T> list);

    template<class Iter, class... Args>
    constexpr LazySeq<T>(Iter first, Iter last, Args... captured);

    constexpr LazySeq<T>();

    constexpr LazySeq<T>(const LazySeq<T> &) = default;

    constexpr LazySeq<T>(LazySeq<T> &&) noexcept = default;

    constexpr explicit LazySeq<T>(wide_size_t count, T value = T());

    template<class Func, class = when_is_function<Func, T, T>>
    constexpr explicit LazySeq<T>(const T &initializer, const Func &next);

    constexpr explicit LazySeq<T>(fabric<T> evaluator, skip_helper_t skipHelper = nullptr);

    constexpr explicit LazySeq<T>(node_ptr<T> nodePtr);

    constexpr explicit LazySeq<T>(node<T> node1);

    template<class Func, class = when_is_function<Func, LazySeq<T>>>
    constexpr explicit LazySeq<T>(const Func &generator);

    [[nodiscard]] constexpr LazySeq<T> setSkipHelper(const skip_helper_t &specialSkip) const;

    virtual ~LazySeq() = default;

    template<class Container>
    [[nodiscard]] auto toContainer() const;

    template<class Container, class F>
    auto toContainer(const F &func) const;

    template<class Container, class F>
    auto toContainerByIndex(const F &func) const;

    template<class Container, class KeyFinder, class ValueFinder>
    auto toContainer(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class Container, class KeyFinder, class ValueFinder>
    auto toContainerByIndex(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    [[nodiscard]] std::vector<T> toVector() const;

    template<class F>
    auto toVector(const F &func) const;

    template<class F>
    auto toVectorByIndex(const F &func) const;

    [[nodiscard]] std::set<T> toSet() const;

    template<class F>
    auto toSet(const F &func) const;

    template<class F>
    auto toSetByIndex(const F &func) const;

    [[nodiscard]] std::unordered_set<T> toUnorderedSet() const;

    template<class F>
    auto toUnorderedSet(const F &func) const;

    template<class F>
    auto toUnorderedSetByIndex(const F &func) const;

    template<class F>
    auto toMap(const F &func) const;

    template<class F>
    auto toMapByIndex(const F &func) const;

    template<class KeyFinder, class ValueFinder>
    auto toMap(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class KeyFinder, class ValueFinder>
    auto toMapByIndex(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class F>
    auto toUnorderedMap(const F &func) const;

    template<class F>
    auto toUnorderedMapByIndex(const F &func) const;

    template<class KeyFinder, class ValueFinder>
    auto toUnorderedMap(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class KeyFinder, class ValueFinder>
    auto toUnorderedMapByIndex(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    [[nodiscard]] std::multiset<T> toMultiSet() const;

    template<class F>
    auto toMultiSet(const F &func) const;

    template<class F>
    auto toMultisetByIndex(const F &func) const;

    [[nodiscard]] std::unordered_multiset<T> toUnorderedMultiSet() const;

    template<class F>
    auto toUnorderedMultiSet(const F &func) const;

    template<class F>
    auto toUnorderedMultiSetByIndex(const F &func) const;

    template<class F>
    auto toMultiMap(const F &func) const;

    template<class F>
    auto toMultiMapByIndex(const F &func) const;

    template<class KeyFinder, class ValueFinder>
    auto toMultiMap(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class KeyFinder, class ValueFinder>
    auto toMultiMapByIndex(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class F>
    auto toUnorderedMultiMap(const F &func) const;

    template<class F>
    auto toUnorderedMultiMapByIndex(const F &func) const;

    template<class KeyFinder, class ValueFinder>
    auto toUnorderedMultiMap(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    template<class KeyFinder, class ValueFinder>
    auto toUnorderedMultiMapByIndex(const KeyFinder &keyFunc, const ValueFinder &valueFunc) const;

    [[nodiscard]] constexpr node_ptr<T> eval() const;
//  [[nodiscard]] LazySeq<T> broadcastSkipHelper() const;

    [[nodiscard]] constexpr T first() const;

    [[nodiscard]] constexpr LazySeq<T> rest() const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr T first(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr T firstByIndex(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> rest(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> rest(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> restByIndex(const Func &pred) const;

    [[nodiscard]] constexpr T itemAt(wide_size_t index) const;

    constexpr wide_size_t indexOf(const T &item, wide_size_t index = 0);

    [[nodiscard]] constexpr LazySeq<indexed_t<T>> getIndexed() const;

    [[nodiscard]] constexpr bool isEmpty() const;

    [[nodiscard]] constexpr explicit operator bool() const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> filter(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> filter(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> filterByIndex(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> remove(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> remove(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> removeByIndex(const Func &pred) const;

    template<class Func, class R = typename ResType<Func, node_ptr<T>>::first_type>
    constexpr LazySeq<R> mapByNode(const Func &f) const;

    template<class Func, class = void, class R = decltype(std::declval<ResType<Func, node_ptr<T>>>()->first)>
    [[nodiscard]] constexpr LazySeq<R> mapByNode(const Func &f) const;

    template<class Func, class R = ResType<Func, T>>
    constexpr LazySeq<R> map(const Func &func) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    constexpr LazySeq<R> mapByIndex(const Func &func) const;

    template<class Func>
    constexpr auto mapMany(const Func &func) const;

    template<class Func>
    constexpr auto mapManyByIndex(const Func &func) const;

    template<class Mapper, class Predicate,
            class = when_is_function<Mapper, T, T>,
            class = when_is_predicate<Predicate, T>>
    constexpr LazySeq<T> mapIf(const Mapper &func, const Predicate &pred) const;

    template<class Mapper, class Predicate,
            class = when_is_function<Mapper, T, indexed_t<T>>,
            class = when_is_predicate<Predicate, indexed_t<T>>>
    constexpr LazySeq<T> mapIfByIndex(const Mapper &func, const Predicate &pred) const;

    constexpr void foreach() const;

    template<class Func>
    constexpr void foreach(const Func &func) const;

    template<class Func>
    constexpr void foreachByIndex(const Func &func) const;

    template<class Func, class = when_is_function<Func, T, T, T>>
    T reduce(const Func &func, T &&defaultValue = T()) const;

    template<class R, class Func, class = when_is_function<Func, R, R, T>>
    R reduce(const R &init, const Func &func) const;

    [[nodiscard]] constexpr LazySeq<T> concat(const LazySeq<T> &other) const;

    [[nodiscard]] constexpr LazySeq<T> repeat(wide_size_t count) const;

    template<class R>
    constexpr LazySeq<std::pair<T, R>> match(const LazySeq<R> &other) const;

    template<class S, class Func, class R = ResType<Func, std::pair<T, S>>>
    constexpr LazySeq<R> match(const LazySeq<S> &other, const Func &func) const;

    template<class S, class Func, class R = ResType<Func, indexed_t<std::pair<T, S>>>>
    constexpr LazySeq<R> matchByIndex(const LazySeq<S> &other, const Func &func) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr bool every(const Func &pred) const;

    [[nodiscard]] constexpr bool every(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr bool everyByIndex(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr bool any(const Func &pred) const;

    [[nodiscard]] constexpr bool any(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr bool anyByIndex(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr bool none(const Func &pred) const;

    [[nodiscard]] constexpr bool none(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr bool noneByIndex(const Func &pred) const;

    [[nodiscard]] constexpr bool contains(const T &value) const;

    [[nodiscard]] constexpr LazySeq<T> take(wide_size_t count = 1) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> takeWhile(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> takeWhile(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> takeWhileByIndex(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> skip(wide_size_t count = 1) const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> skipWhile(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> skipWhile(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> skipWhileByIndex(const Func &pred) const;

    [[nodiscard]] wide_size_t count() const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] wide_size_t count(const Func &pred) const;

    [[nodiscard]] wide_size_t count(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] wide_size_t countByIndex(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> emplaceFront(const T &value) const;

    [[nodiscard]] constexpr LazySeq<T> emplaceBack(const T &value) const;

    [[nodiscard]] T sum() const;

    template<class Func, class R = ResType<Func, T>>
    R sum(const Func &f) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    R sumByIndex(const Func &f) const;

    [[nodiscard]] T average() const;

    template<class Func, class R = ResType<Func, T>>
    R average(const Func &f) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    R averageByIndex(const Func &f) const;

    [[nodiscard]] T subtract() const;

    template<class Func, class R = ResType<Func, T>>
    R subtract(const Func &f) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    R subtractByIndex(const Func &f) const;

    [[nodiscard]] T multiply() const;

    template<class Func, class R = ResType<Func, T>>
    R multiply(const Func &f) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    R multiplyByIndex(const Func &f) const;

    [[nodiscard]] T divide() const;

    template<class Func, class R = ResType<Func, T>>
    R divide(const Func &f) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    R divideByIndex(const Func &f) const;

    template<class Func, class = when_is_comparer<Func, T>>
    [[nodiscard]] T min(const Func &comp) const;

    [[nodiscard]] T min() const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    T min(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = void, class = ResType<Mapper, T>>
    T min(const Mapper &f) const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, indexed_t<T>>, class = when_is_comparer<Comparer, R>>
    T minByIndex(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = ResType<Mapper, indexed_t<T>>>
    T minByIndex(const Mapper &f) const;

    template<class Func, class = when_is_comparer<Func, T>>
    [[nodiscard]] T max(const Func &comp) const;

    [[nodiscard]] T max() const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    T max(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = void, class = ResType<Mapper, T>>
    T max(const Mapper &f) const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, indexed_t<T>>, class = when_is_comparer<Comparer, R>>
    T maxByIndex(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = ResType<Mapper, indexed_t<T>>>
    T maxByIndex(const Mapper &f) const;

    template<class Func, class = when_is_comparer<Func, T>>
    [[nodiscard]] std::pair<T, T> minMax(const Func &comp) const;

    [[nodiscard]] std::pair<T, T> minMax() const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    std::pair<T, T> minMax(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = void, class = ResType<Mapper, T>>
    std::pair<T, T> minMax(const Mapper &f) const;

    template<class Mapper, class Comparer, class R = ResType<Mapper, indexed_t<T>>, class = when_is_comparer<Comparer, R>>
    std::pair<T, T> minMaxByIndex(const Mapper &f, const Comparer &comp) const;

    template<class Mapper, class = ResType<Mapper, indexed_t<T>>>
    std::pair<T, T> minMaxByIndex(const Mapper &f) const;

    [[nodiscard]] std::string toString(const std::string &separator = " ") const;

    template<class Func, class = ResType<Func, T>>
    std::string toString(const Func &f, const std::string &separator = " ") const;

    template<class Func, class = ResType<Func, indexed_t<T>>>
    std::string toStringByIndex(const Func &f, const std::string &separator = " ") const;

    [[nodiscard]] T last() const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] T last(const Func &pred) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] T lastByIndex(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> butLast() const;

    template<class Func, class = when_is_predicate<Func, T>>
    [[nodiscard]] constexpr LazySeq<T> butLast(const Func &pred) const;

    [[nodiscard]] constexpr LazySeq<T> butLast(const T &item) const;

    template<class Func, class = when_is_predicate<Func, indexed_t<T>>>
    [[nodiscard]] constexpr LazySeq<T> butLastByIndex(const Func &pred) const;

    [[nodiscard]] virtual ReversedLazySeq<T> reverse() const;

    template<class Func, class R = ResType<Func, T>>
    constexpr ReversedLazySeq<R> reverse(const Func &func) const;

    template<class Func, class R = ResType<Func, indexed_t<T>>>
    constexpr ReversedLazySeq<R> reverseByIndex(const Func &func) const;

    template<class R>
    constexpr LazySeq<R> castTo() const;

    template<class R>
    constexpr LazySeq<R> staticCastTo() const;

    template<class R>
    constexpr LazySeq<R> reinterpretCastTo() const;

    template<class R>
    constexpr LazySeq<R> constCastTo() const;

    template<class R>
    constexpr LazySeq<R> dynamicCastTo() const;

    LazySeq<T> &operator=(const LazySeq<T> &) = default;

    LazySeq<T> &operator=(LazySeq<T> &&) noexcept = default;

    template<class R>
    bool operator==(const LazySeq<R> &other) const;

    template<class R>
    bool operator!=(const LazySeq<R> &other) const;

    template<class R>
    bool operator<(const LazySeq<R> &other) const;

    template<class R>
    bool operator>(const LazySeq<R> &other) const;

    template<class R>
    bool operator<=(const LazySeq<R> &other) const;

    template<class R>
    bool operator>=(const LazySeq<R> &other) const;

    constexpr LazySeq<T> &operator+=(const LazySeq<T> &other);

    constexpr LazySeq<T> &operator+=(T &&item);

    constexpr LazySeq<T> &operator*=(wide_size_t count);

    template<class R>
    constexpr LazySeq<std::pair<T, R>> operator*(const LazySeq<R> &other) const;

    template<wide_size_t n>
    [[nodiscard]] constexpr auto pow() const;

    [[nodiscard]] constexpr LazySeq<T> distinct() const;

    [[nodiscard]] constexpr LazySeq<T> except(const LazySeq<T> &other) const;

    [[nodiscard]] constexpr LazySeq<T> intersect(const LazySeq<T> &other) const;

    [[nodiscard]] constexpr LazySeq<T> unite(const LazySeq<T> &other) const;

    [[nodiscard]] LazySeq<LazySeq<T>> groupBy(wide_size_t portion) const;

    template<class Func, class F = ResType<Func, T>>
    LazySeq<std::pair<F, LazySeq<T>>> groupBy(const Func &keyFinder) const;

    template<class KeyFinder, class Mapper, class F = ResType<KeyFinder, T>, class R = ResType<Mapper, LazySeq<T>>>
    LazySeq<std::pair<F, R>> groupBy(const KeyFinder &keyFinder,
                                     const Mapper &seqFunc) const;

    template<class KeyFinder, class ValueFunc, class Mapper,
            class F = ResType<KeyFinder, T>,
            class S = ResType<ValueFunc, T>,
            class R = ResType<Mapper, LazySeq<S>>>
    LazySeq<std::pair<F, R>> groupBy(const KeyFinder &keyFinder,
                                     const ValueFunc &valueFunc,
                                     const Mapper &seqFunc) const;

    template<class KeyFinder, class ValueFunc,
            class F = ResType<KeyFinder, T>,
            class S = ResType<ValueFunc, T>,
            class = void>
    LazySeq<std::pair<F, LazySeq<S>>> groupBy(const KeyFinder &keyFinder,
                                              const ValueFunc &valueFunc) const;

    template<class Func, class F = ResType<Func, indexed_t<T>>>
    LazySeq<std::pair<F, LazySeq<T>>> groupByIndexBy(const Func &keyFinder) const;

    template<class KeyFinder, class ValueFunc,
            class F = ResType<KeyFinder, indexed_t<T>>,
            class S = ResType<ValueFunc, indexed_t<T>>,
            class = void>
    LazySeq<std::pair<F, LazySeq<S>>> groupByIndexBy(const KeyFinder &keyFinder,
                                                     const ValueFunc &valueFunc) const;

    template<class KeyFinder, class Mapper,
            class F = ResType<KeyFinder, indexed_t<T>>,
            class R = ResType<Mapper, LazySeq<indexed_t<T>>>>
    LazySeq<std::pair<F, R>> groupByIndexBy(const KeyFinder &keyFinder,
                                            const Mapper &seqFunc) const;

    template<class KeyFinder, class ValueFunc, class Mapper,
            class F = ResType<KeyFinder, indexed_t<T>>,
            class S = ResType<ValueFunc, indexed_t<T>>,
            class R = ResType<Mapper, LazySeq<S>>>
    LazySeq<std::pair<F, R>> groupByIndexBy(const KeyFinder &keyFinder,
                                            const ValueFunc &valueFunc,
                                            const Mapper &seqFunc) const;

    template<class Comparer, class = when_is_comparer<Comparer, T>>
    [[nodiscard]] constexpr OrderedLazySeq<T> orderByItselfWith(const Comparer &comp) const;

    [[nodiscard]] constexpr OrderedLazySeq<T> orderByItself() const;

    template<class Comparer, class Mapper, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<T> orderBy(const Mapper &func, const Comparer &comp) const;

    template<class Mapper, class R = ResType<Mapper, T>>
    constexpr OrderedLazySeq<T> orderBy(const Mapper &func) const;

    template<class Comparer, class = when_is_comparer<Comparer, T>>
    [[nodiscard]] constexpr OrderedLazySeq<T> orderByDescendingByItselfWith(const Comparer &comp) const;

    [[nodiscard]] constexpr OrderedLazySeq<T> orderByDescendingByItself() const;

    template<class Comparer, class Mapper, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<T> orderByDescendingBy(const Mapper &func, const Comparer &comp) const;

    template<class Mapper, class R = ResType<Mapper, T>>
    constexpr OrderedLazySeq<T> orderByDescendingBy(const Mapper &func) const;

    /*template<class... Args>
    constexpr OrderedLazySeq<T> orderBy(const Args &... args) const {
      return makeOrdered().thenBy(args...);
    }

    template<class... Args>
    constexpr OrderedLazySeq<T> orderByDescending(const Args &... args) const {
      return makeOrdered().thenByDescending(args...);
    }

    template<class... Args>
    constexpr OrderedLazySeq<T> orderByIndexBy(const Args &... args) const {
      return getIndexed().orderBy(args...).map([](auto &&pair) { return pair.second; });
    }

    template<class... Args>
    constexpr OrderedLazySeq<T> orderByDescendingByIndexBy(const Args &... args) const {
      return getIndexed().orderByDescending(args...).map([](auto &&pair) { return pair.second; });
    }*/

    /*template<class Mapper, class = ResType<Mapper, T>, class = void>
    constexpr OrderedLazySeq<T> orderBy(const Mapper &func) const;
    template<class Mapper, class Comparer, class R = ResType<Mapper, indexed_t<T>>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<indexed_t<T>> orderByIndexBy(const Mapper &func,
                                                          const Comparer &comp) const;
    template<class Mapper, class = ResType<Mapper, indexed_t<T>>>
    constexpr OrderedLazySeq<indexed_t<T>> orderByIndexBy(const Mapper &func) const;
    template<class Mapper, class Comparer, class R = ResType<Mapper, T>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<T> orderByDescending(const Mapper &func,
                                                  const Comparer &comp) const;
    template<class Mapper, class R = ResType<Mapper, T>, class = void>
    constexpr OrderedLazySeq<T> orderByDescending(const Mapper &func) const;
    template<class Mapper, class Comparer, class R = ResType<Mapper, indexed_t<T>>, class = when_is_comparer<Comparer, R>>
    constexpr OrderedLazySeq<indexed_t<T>> orderByDescendingByIndexBy(const Mapper &func,
                                                                      const Comparer &com) const;
    template<class Mapper, class = ResType<Mapper, indexed_t<T>>>
    constexpr OrderedLazySeq<indexed_t<T>> orderByDescendingByIndexBy(const Mapper &func) const;*/

    [[nodiscard]] constexpr bool hasSpecialSkipHelper() const;

    [[nodiscard]] auto applySkipHelper(wide_size_t count) const;

private:
    static const wide_size_t BUCKET_SIZE_FOR_STD_SORT_CALL = 1024;

    fabric<T> evaluator_;
    skip_helper_t skipHelper_ = nullptr;

    static void broadcastSkipHelper(node_ptr<T> &evaluated, skip_helper_t skipper, wide_size_t i = 0);

    template<class Func, class = when_is_comparer<Func, T>>
    static auto partition(typename std::vector<T>::iterator begin,
                          typename std::vector<T>::iterator end, const Func &comp);

    template<class Func, class = when_is_comparer<Func, T>>
    static void separateMore(const DataController<T> &dataController, const Func &comp);

};

namespace std {
    template<class T>
    struct hash<LazySeq<T>> {
        size_t operator()(const LazySeq<T> &seq) const {
            const size_t k = 65537; // Constant for polynomial hash
            return seq.LazySeq<T>::template reduce<size_t>(0, [k](size_t hash, const T &cur) {
                return hash * k + std::hash<T>()(cur);
            });
        }
    };
}

template<class T>
class LazyIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    constexpr LazyIterator<T>(const LazyIterator<T> &other) = default;

    constexpr explicit LazyIterator<T>(const LazySeq<T> &lazySeq);

    /*virtual*/ ~LazyIterator() noexcept = default;

    constexpr T operator*() const;

    constexpr bool operator!=(const LazyIterator<T> &other) const;

    constexpr bool operator==(const LazyIterator<T> &other) const;

    LazyIterator<T> &operator=(const LazyIterator<T> &other) noexcept = default;

    constexpr T operator->() const;

    constexpr LazyIterator<T> &operator++();

    LazyIterator<T> operator++(int);

    LazyIterator<T> operator+(wide_size_t count) const;

    LazyIterator<T> operator+=(wide_size_t count);

private:
    node_ptr<T> evaluated;
};

template<class Func>
constexpr static auto descendingComparer(const Func &comp);

template<class Iter, class... Args>
LazySeq(Iter begin, Iter end, Args... captured) -> LazySeq<typename std::iterator_traits<Iter>::value_type>;

template<class LazySeq>
constexpr auto operator+(const LazySeq &a, const LazySeq &b);

template<class T>
constexpr LazySeq<T> operator+(const LazySeq<T> &a, T &&item);

template<class T>
constexpr LazySeq<T> operator*(const LazySeq<T> &a, wide_size_t count);

template<class Func>
constexpr auto negate(const Func &pred);
//TODO replace with std::...

template<class T>
constexpr auto dividesBy(const T &n);

template<class T>
constexpr bool even(const T &item);

template<class T>
constexpr bool odd(const T &item);

template<class T>
constexpr auto constantly(const T &item);

template<class F, class... Args>
constexpr decltype(auto) partial(const F &f, const Args &... args);

template<class T>
constexpr T increment(T obj);

template<class T>
constexpr T decrement(T obj);

template<class T>
constexpr auto isEqualTo(const T &item);

template<class T>
constexpr auto isNotEqualTo(const T &item);

template<class T>
constexpr auto isLessThan(const T &item);

template<class T>
constexpr auto isGreaterThan(const T &item);

template<class T>
constexpr auto isNotLessThan(const T &item);

template<class T>
constexpr auto isNotGreaterThan(const T &item);

template<class T>
constexpr LazySeq<T> operator*(wide_size_t count, const LazySeq<T> &seq);

template<class T>
constexpr LazySeq<T> operator+(T &&item, const LazySeq<T> &seq);

template<class T>
constexpr LazySeq<T> range(const T &start, wide_size_t count);

template<class T>
constexpr LazySeq<T> infiniteRange(const T &start);

template<class T>
constexpr LazySeq<T> join(const LazySeq<LazySeq<T>> &seq);

template<class Container>
constexpr auto join(const LazySeq<Container> &seq);

template<class Container>
constexpr auto join(const ReversedLazySeq<Container> &seq);

template<class Container>
auto makeLazy(Container &&container);

template<class T>
constexpr auto square(const T &seq);

template<class T>
std::ostream &operator<<(std::ostream &out, const LazySeq<T> &seq);

template<class Container>
LazySeq<size_t> indexesOf(const Container &container);

template<class T>
LazySeq<wide_size_t> indexesOf(const LazySeq<T> &seq);

template<class T>
constexpr LazySeq<std::pair<wide_size_t, T>> indexed(const LazySeq<T> &seq);

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<K> keys(const LazySeq<std::pair<K, V>> &seq);

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<V> values(const LazySeq<std::pair<K, V>> &seq);

LazySeq<wide_size_t> indexes(wide_size_t start = 0);

LazySeq<natural_t> naturalNumbers();

LazySeq<integer_t> integerNumbers();

LazySeq<rational_t> positiveRationalNumbers();

LazySeq<rational_t> rationalNumbers();

template<class T = natural_t>
constexpr LazySeq<T> powersByFixedBase(const T &base);

template<class T = natural_t>
constexpr T fibonacci(const T &n);

template<class T = natural_t>
constexpr LazySeq<T> fibonacciSeq();

template<class T = natural_t>
constexpr LazySeq<T> factorials();

template<class T = natural_t>
constexpr LazySeq<T> powersByFixedExponent(const T &exponent);

template<class T>
constexpr LazySeq<T> identitySeq(const T &item);

inline wide_size_t getRandomIndex() {
    static std::mt19937_64 gen(static_cast<unsigned long>(std::chrono::system_clock::now().time_since_epoch().count()));
    return gen();
}

inline wide_size_t getRandomIndex(wide_size_t max) {
    return getRandomIndex() % max;
}

inline wide_size_t getRandomIndex(wide_size_t min, wide_size_t max) {
    return min + getRandomIndex(max - min);
}

template<class... Args>
LazySeq<wide_size_t> randomNumbers(Args... args);

#include "LazySeq.hpp"

#endif //FUNCTIONAL_LAZY_SEQ_H
