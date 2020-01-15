
//
// Created by zhele on 06.06.2019.
//

#include "LazySeq.h"

namespace {
    template<class T, class = int>
    struct adder {
        static const bool hasPlus = false;

        static T invoke(T item, wide_size_t count) {
            while (count) {
                ++item;
                --count;
            }
            return item;
        }
    };

    template<class T>
    struct adder<T, decltype(std::declval<T>() + std::declval<wide_size_t>(), 0)> {
        static const bool hasPlus = true;

        static T invoke(const T &item, wide_size_t count) {
            return item + count;
        }
    };

    template<class T/*, class = typename std::enable_if<std::is_nothrow_move_constructible_v<T>>::type*/>
    constexpr void reassign(std::optional<T> &data, std::optional<T> &&newData) noexcept {
        if (newData.has_value()) {
            data.emplace(std::forward<T>(*newData));
        } else {
            data.reset();
        }
    }
}

template<class T>
constexpr node_ptr<T> LazySeq<T>::eval() const {
    return broadcastSkipHelper(evaluator_(), skipHelper_);
}

//template<class T>
//LazySeq<T> LazySeq<T>::broadcastSkipHelper() const {
//  if (hasSpecialSkipHelper()) {
//    auto indexed = getIndexed();
//    return values(indexed.mapByNode(
//        [indexed](node_ptr<indexed_t<T>>& &evaled) {
//          return std::pair{evaled->first, evaled->second.setSkipHelper(
//              [indexed, offset = evaled->first.first](wide_size_t count) {
//                return indexed.applySkipHelper(count <= WIDE_SIZE_T_MAX - offset ? count + offset : WIDE_SIZE_T_MAX);
//              })};
//        }));
//  } else {
//    return *this;
//  }
//}

template<class T>
constexpr bool LazySeq<T>::isEmpty() const {
    return !eval();
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::filter(const Func &pred) const {
    return mapByNode([pred = pred](node_ptr<T> &&pair) -> node_ptr<T> {
        for (; pair.has_value() && !pred(pair->first); pair = pair->second.eval());
        if (pair.has_value()) {
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
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::filterByIndex(const Func &pred) const {
    return values(getIndexed().filter(pred));
}

template<class T>
template<class Func, class R>
constexpr LazySeq<R> LazySeq<T>::map(const Func &func) const {
    return mapByNode([func = func](node_ptr<T> &&pair) -> node<R> {
        return {func(std::move(pair->first)), pair->second.map(func)};
    }).setSkipHelper(
            hasSpecialSkipHelper()
            ? [func = func, *this](wide_size_t count) {
                auto[notSkippedYet, rest] = applySkipHelper(count);
                return std::pair{notSkippedYet, rest.map(func)};
            }
            : typename LazySeq<R>::skip_helper_t());
}

template<class T>
template<class Func, class R>
constexpr LazySeq<R> LazySeq<T>::mapByIndex(const Func &func) const {
    return getIndexed().map(func);
}

template<class T>
template<class Iter, class... Args>
constexpr LazySeq<T>::LazySeq(Iter first, Iter last, Args... captured)
        : LazySeq(
        [first, last, captured...]() -> node_ptr<T> {
            if (first != last) {
                auto next = first;
                return std::pair{*first, LazySeq(++next, last, captured...)};
            } else {
                return std::nullopt;
            }
        }) {
    if constexpr (std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>) {
        skipHelper_ = [first, last, captured...](wide_size_t count) {
            if (auto dist = std::distance(first, last); dist > count) {
                return std::pair{0ull, LazySeq(first + count, last, captured...)};
            } else {
                return std::pair{count - dist, LazySeq<T>()};
            }
        };
    }
}

/* not following because of expensive virtual calls (then they while be replaced with statically polymorphic seqs)*/
/*   : LazySeq((std::is_same_v<typename std::iterator_traits<Iter>::iterator_category, std::random_access_iterator_tag>
               ? range(first, (wide_size_t) std::distance(first, last))
               : infiniteRange(first).takeWhile([last, captured...](Iter iter) { return iter != last; }))
                  .map([captured...](Iter iter) { return *iter; })
) {}*/

template<class T>
template<class Func, class>
T LazySeq<T>::reduce(const Func &func, T &&defaultValue) const {
    auto pair = eval().value_or(node<T>{defaultValue, LazySeq<T>()});
    return pair.second.reduce(pair.first, func);
}

template<class T>
template<class R, class Func, class>
R LazySeq<T>::reduce(const R &init, const Func &func) const {
    std::optional<R> res = init;
    for (const auto &item: *this) {
        res.emplace(func(std::move(*res), item));
    }
    return *res;
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::castTo() const {
    return map([](const T &a) { return (R) a; });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::staticCastTo() const {
    return map([](const T &a) { return static_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::reinterpretCastTo() const {
    return map([](const T &a) { return reinterpret_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::constCastTo() const {
    return map([](const T &a) { return const_cast<R>(a); });
}

template<class T>
template<class R>
constexpr LazySeq<R> LazySeq<T>::dynamicCastTo() const {
    return map([](const T &a) { return dynamic_cast<R>(a); });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::concat(const LazySeq<T> &other) const {
    return LazySeq<T>([*this, other]() -> node_ptr<T> {
        node_ptr<T> pair = eval();
        return pair.has_value() ? (pair->second = pair->second.concat(other), pair) : other.eval();
    }).setSkipHelper(
            hasSpecialSkipHelper() || other.hasSpecialSkipHelper()
            ? [*this, other](wide_size_t count) {
                auto[thisToBeSkipped, thisRest] = applySkipHelper(count);
                return thisToBeSkipped ? other.applySkipHelper(thisToBeSkipped) : std::pair{thisToBeSkipped,
                                                                                            thisRest + other};
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
    return seq.mapMany([](const auto &container) { return makeLazy(std::move(container)); });
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
    if (&evaluator_ == &other.evaluator_) {
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
    if (evaluated.has_value()) {
        reassign(evaluated, evaluated->second.eval());
    }
    return *this;
}

template<class T>
LazyIterator<T> LazyIterator<T>::operator++(int) {
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
    return evaluated.has_value() == other.evaluated.has_value();
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
    return count && evaluated ? LazyIterator<T>(evaluated->second.skip(count - 1)) : *this;
}

template<class T>
LazyIterator<T> LazyIterator<T>::operator+=(wide_size_t count) {
    return reassign(evaluated, (*this + count).evaluated), *this;
}

//template<class T>
//template<class R>
//constexpr LazySeq<R> LazySeq<T>::mapByNode(const std::function<node<R>(node_ptr<T>)> &f) const {
//  returnmapByNode([f = f](node_ptr<T>&& node1) -> node_ptr<R> {
//    return std::optional<node<R>>(f(node1));
//  });
//}

template<class T>
template<class Func, class R>
constexpr LazySeq<R> LazySeq<T>::mapByNode(const Func &f) const {
    return mapByNode<Func, void, R>(f);
}

template<class T>
template<class Func, class, class R>
constexpr LazySeq<R> LazySeq<T>::mapByNode(const Func &f) const {
    return LazySeq<R>([*this, f = f]() -> node_ptr<R> {
        node_ptr<T> old = eval();
        return old.has_value() ? f(std::move(old)) : node_ptr<R>();
    });
}

template<class T>
LazySeq<T>::LazySeq(std::initializer_list<T> list)
        : LazySeq<T>(makeLazy(std::move(std::vector<T>(list)))) {}

template<class T>
constexpr LazySeq<T>::LazySeq(const node<T> &node1) : LazySeq(std::make_optional<node<T>>(node1)) {}

template<class T>
constexpr LazySeq<T>::LazySeq(const node_ptr<T> &nodePtr) : LazySeq<T>(constantly(nodePtr)) {}

template<class T>
constexpr LazySeq<T>::LazySeq() : LazySeq(node_ptr<T>()) {}

template<class T>
constexpr LazySeq<T>::LazySeq(fabric<T> evaluator, skip_helper_t skipHelper)
        : evaluator_(std::move(evaluator)), skipHelper_(std::move(skipHelper)) {}

template<class T>
constexpr LazySeq<T> LazySeq<T>::repeat(wide_size_t count) const {
    return range<wide_size_t>(0, count).mapMany(constantly(*this))
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
                return std::pair{(wide_size_t) 0,
                                 (skipCount % bucketCount != 0 ? skip(skipCount % bucketCount) : LazySeq<T>())
                                 + repeat(resFullCount / bucketCount)};
            });
    /*.broadcastSkipHelper();*/
}

template<class T>
constexpr LazySeq<T> operator*(const LazySeq<T> &a, wide_size_t count) {
    return a.repeat(count);
}

template<class T>
template<class R>
constexpr LazySeq<std::pair<T, R>> LazySeq<T>::operator*(const LazySeq<R> &other) const {
    return mapMany([other](const T &first) -> LazySeq<std::pair<T, R>> {
        return other.map(
                [first](const R &second) -> std::pair<T, R> { return {first, second}; }
        );
    });
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::every(const Func &pred) const {
    return none(negate(pred));
}

template<class T>
constexpr bool LazySeq<T>::every(const T &item) const {
    return every(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::everyByIndex(const Func &pred) const {
    return getIndexed().every(pred);
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::any(const Func &pred) const {
    return !none(pred);
}

template<class T>
constexpr bool LazySeq<T>::any(const T &item) const {
    return any(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::anyByIndex(const Func &pred) const {
    return !noneByIndex(pred);
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::none(const Func &pred) const {
    return filter(pred).isEmpty();
}

template<class T>
constexpr bool LazySeq<T>::none(const T &item) const {
    return none(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr bool LazySeq<T>::noneByIndex(const Func &pred) const {
    return filterByIndex(pred).isEmpty();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::take(wide_size_t count) const {
    return mapByNode([count](node_ptr<T> &&pair) -> node_ptr<T> {
        return count ? std::make_optional<node<T>>(pair->first, pair->second.take(count - 1))
                     : node_ptr<T>();
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
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::takeWhile(const Func &pred) const {
    return mapByNode([pred = pred](node_ptr<T> &&pair) -> node_ptr<T> {
        return pred(pair->first) ? std::make_optional<node<T>>(pair->first, pair->second.takeWhile(pred))
                                 : node_ptr<T>();
    });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::takeWhile(const T &item) const {
    return takeWhile(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::takeWhileByIndex(const Func &pred) const {
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
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::skipWhile(const Func &pred) const {
    return mapByNode([pred = pred](node_ptr<T> &&pair) -> node_ptr<T> {
        for (; pair.has_value() && pred(pair->first); pair = pair->second.eval());
        return pair;
    });
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::skipWhile(const T &item) const {
    return skipWhile(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::skipWhileByIndex(const Func &pred) const {
    return values(getIndexed().skipWhile(pred));
}

template<class T>
wide_size_t LazySeq<T>::count() const {
    return WIDE_SIZE_T_MAX - applySkipHelper(WIDE_SIZE_T_MAX).first;
}

template<class T>
template<class Func, class>
wide_size_t LazySeq<T>::count(const Func &pred) const {
    return filter(pred).count();
}

template<class T>
wide_size_t LazySeq<T>::count(const T &item) const {
    return count(isEqualTo(item));
}

template<class T>
template<class Func, class>
wide_size_t LazySeq<T>::countByIndex(const Func &pred) const {
    return getIndexed().count(pred);
}

template<class T>
T LazySeq<T>::sum() const {
    return reduce(std::plus<T>());
}

template<class T>
template<class Func, class R>
R LazySeq<T>::sum(const Func &f) const {
    return map(f).sum();
}

template<class T>
template<class Func, class R>
R LazySeq<T>::sumByIndex(const Func &f) const {
    return mapByIndex(f).sum();
}

template<class T>
T LazySeq<T>::subtract() const {
    return reduce(std::minus<T>());
}

template<class T>
template<class Func, class R>
R LazySeq<T>::subtract(const Func &f) const {
    return map(f).subtract();
}

template<class T>
template<class Func, class R>
R LazySeq<T>::subtractByIndex(const Func &f) const {
    return mapByIndex(f).subtract();
}

template<class T>
T LazySeq<T>::multiply() const {
    return reduce(std::multiplies<T>(), static_cast<T>(1));
}

template<class T>
template<class Func, class R>
R LazySeq<T>::multiply(const Func &f) const {
    return map(f).multiply();
}

template<class T>
template<class Func, class R>
R LazySeq<T>::multiplyByIndex(const Func &f) const {
    return mapByIndex(f).multiply();
}

template<class T>
T LazySeq<T>::divide() const {
    return reduce(std::divides<T>(), static_cast<T>(1));
}

template<class T>
template<class Func, class R>
R LazySeq<T>::divide(const Func &f) const {
    return map(f).divide();
}

template<class T>
template<class Func, class R>
R LazySeq<T>::divideByIndex(const Func &f) const {
    return mapByIndex(f).divide();
}

template<class T>
template<class Func, class>
T LazySeq<T>::min(const Func &comp) const {
    return reduce([&comp](const T &a, const T &b) { return comp(a, b) ? a : b; });
}

template<class T>
T LazySeq<T>::min() const {
    return min(std::less<T>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
T LazySeq<T>::min(const Mapper &f, const Comparer &comp) const {
    return map([&f](const T &item) { return std::pair{item, f(item)}; })
            .min([&comp](const auto &pair1, const auto &pair2) { return comp(pair1.second, pair2.second); }).first;
}

template<class T>
template<class Mapper, class, class R>
T LazySeq<T>::min(const Mapper &f) const {
    return min(f, std::less<R>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
T LazySeq<T>::minByIndex(const Mapper &f, const Comparer &comp) const {
    return getIndexed().min(f, comp).second;
}

template<class T>
template<class Mapper, class R>
T LazySeq<T>::minByIndex(const Mapper &f) const {
    return minByIndex(f, std::less<R>());
}

template<class Func>
constexpr auto descendingComparer(const Func &comp) {
    return [comp = comp](const auto &a, const auto &b) { return comp(b, a); };
}

template<class T>
template<class Func, class>
T LazySeq<T>::max(const Func &comp) const {
    return min(descendingComparer(comp));
}

template<class T>
T LazySeq<T>::max() const {
    return max(std::less<T>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
T LazySeq<T>::max(const Mapper &f, const Comparer &comp) const {
    return min(f, descendingComparer(comp));
}

template<class T>
template<class Mapper, class, class R>
T LazySeq<T>::max(const Mapper &f) const {
    return max(f, std::less<R>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
T LazySeq<T>::maxByIndex(const Mapper &f, const Comparer &comp) const {
    return getIndexed().max(f, comp).second;
}

template<class T>
template<class Mapper, class R>
T LazySeq<T>::maxByIndex(const Mapper &f) const {
    return maxByIndex(f, std::less<R>());
}

template<class T>
template<class Func, class>
std::pair<T, T> LazySeq<T>::minMax(const Func &comp) const {
    auto firstNode = eval();
    return firstNode->second.reduce(
            std::pair{firstNode->first, firstNode->first},
            [&comp](const auto &pair, const T &cur) {
                return std::pair{comp(cur, pair.first) ? cur : pair.first,
                                 comp(pair.first, cur) ? cur : pair.second};
            });
}

template<class T>
std::pair<T, T> LazySeq<T>::minMax() const {
    return minMax(std::less<T>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
std::pair<T, T> LazySeq<T>::minMax(const Mapper &f, const Comparer &comp) const {
    auto ans = map([&f](const T &item) { return std::pair{item, f(item)}; })
            .minMax([&comp](const auto &pair1, const auto &pair2) { return comp(pair1.second, pair2.second); });
    return std::pair{ans.first.first, ans.second.first};
}

template<class T>
template<class Mapper, class, class R>
std::pair<T, T> LazySeq<T>::minMax(const Mapper &f) const {
    return minMax(f, std::less<R>());
}

template<class T>
template<class Mapper, class Comparer, class, class>
std::pair<T, T> LazySeq<T>::minMaxByIndex(const Mapper &f, const Comparer &comp) const {
    auto res = getIndexed().minMax(f, comp);
    return std::pair{res.first.second, res.second.second};
}

template<class T>
template<class Mapper, class R>
std::pair<T, T> LazySeq<T>::minMaxByIndex(const Mapper &f) const {
    return minMaxByIndex(f, std::less<R>());
}

template<class T>
std::string LazySeq<T>::toString(const std::string &separator) const {
    std::stringstream stream;
    auto pair = eval();
    if (!pair.has_value()) {
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
    if (hasSpecialSkipHelper()) {
        auto cnt = count();
        return cnt ? itemAt(cnt - 1) : T();
    } else {
        return reduce([](const T &, const T &b) -> T { return b; });
    }
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLast() const {
    return LazySeq<T>([*this] {
        auto first = eval();
        if (!first.has_value()) {
            return LazySeq<T>();
        }
        return keys(LazySeq<std::pair<T, node_ptr<T>>>(
                std::pair{first->first, first->second.eval()},
                [](const auto &pair) {
                    auto restNode = pair.second;
                    return std::pair{restNode->first, restNode->second.eval()};
                }).takeWhile([](const auto &pair) { return pair.second.has_value(); }));
    })
            .setSkipHelper(
                    hasSpecialSkipHelper()
                    ? [*this](wide_size_t count) {
                        auto[toBeSkipped, rest] = applySkipHelper(count);
                        auto node = rest.eval();
                        return node ? std::pair{(wide_size_t) 0, LazySeq(node).butLast()}
                                    : std::pair{toBeSkipped < WIDE_SIZE_T_MAX ? toBeSkipped + 1 : WIDE_SIZE_T_MAX,
                                                LazySeq<T>()};
                    }
                    : skip_helper_t())

        /*.broadcastSkipHelper()*/;
}

template<class T>
template<class R>
constexpr LazySeq<std::pair<T, R>> LazySeq<T>::match(const LazySeq<R> &other) const {
    return mapByNode([other](node_ptr<T> &&pair) -> node_ptr<std::pair<T, R>> {
        auto otherPair = other.eval();
        return otherPair ? std::make_optional<node<std::pair<T, R>>>(std::pair{pair->first, otherPair->first},
                                                                     pair->second.match(otherPair->second))
                         : node_ptr<std::pair<T, R>>();
    }).setSkipHelper(
            hasSpecialSkipHelper() || other.hasSpecialSkipHelper()
            ? [*this, other](wide_size_t count) {
                auto[thisToBeSkipped, thisRest] = hasSpecialSkipHelper() ? applySkipHelper(count) : std::pair{count,
                                                                                                              *this};
                auto[otherToBeSkipped, otherRest] = other.hasSpecialSkipHelper() ? other.applySkipHelper(count)
                                                                                 : std::pair{count, other};
                return std::pair{std::max(thisToBeSkipped, otherToBeSkipped), thisRest.match(otherRest)};
            }
            : typename LazySeq<std::pair<T, R>>::skip_helper_t());
}

template<class T>
template<class Func, class>
std::string LazySeq<T>::toString(const Func &f, const std::string &separator) const {
    return map(f).toString(separator);
}

template<class T>
template<class Func, class>
std::string LazySeq<T>::toStringByIndex(const Func &f, const std::string &separator) const {
    return getIndexed().toString(f, separator);
}

template<class T>
constexpr LazySeq<T> operator*(wide_size_t count, const LazySeq<T> &seq) {
    return seq * count;
}

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
                                                                     const std::function<V(
                                                                             indexed_t<T>)> &valueFunc) const {
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
    return toContainer<Container, std::pair<K, V>>([keyFunc = keyFunc, valueFunc = valueFunc](const T &item) {
        return std::pair{keyFunc(item), valueFunc(item)};
    });
}

template<class T>
template<class Container, class K, class V>
auto LazySeq<T>::toContainerByIndex(const std::function<K(indexed_t<T>)> &keyFunc,
                                    const std::function<V(indexed_t<T>)> &valueFunc) const {
    return toContainerByIndex<Container, std::pair<K, V>>([keyFunc = keyFunc,
                                                                  valueFunc = valueFunc](const indexed_t<T> &item) {
        return std::pair{keyFunc(item), valueFunc(item)};
    });
}

template<class T>
T LazySeq<T>::average() const {
    auto sum = map([](const T &item) { return std::pair{std::move(item), (wide_size_t) 1}; })
            .reduce([](const auto &a, const auto &b) { return std::pair{a.first + b.first, a.second + b.second}; });
    return sum.first / sum.second;
}

template<class T>
template<class Func, class R>
R LazySeq<T>::average(const Func &func) const {
    return map(func).average();
}

template<class T>
template<class Func, class R>
R LazySeq<T>::averageByIndex(const Func &func) const {
    return mapByIndex(func).average();
}

template<class T>
ReversedLazySeq<T> LazySeq<T>::reverse() const {
    return ReversedLazySeq(*this);
}

template<class T>
template<class Func, class R>
constexpr ReversedLazySeq<R> LazySeq<T>::reverse(const Func &func) const {
    return map(func).reverse();
}

template<class T>
template<class Func, class R>
constexpr ReversedLazySeq<R> LazySeq<T>::reverseByIndex(const Func &func) const {
    return getIndexed().reverse(func);
}

template<class T>
template<class Func, class>
T LazySeq<T>::last(const Func &pred) const {
    return filter(pred).last();
}

template<class T>
template<class Func, class>
T LazySeq<T>::lastByIndex(const Func &pred) const {
    return filterByIndex(pred).last();
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::butLast(const Func &pred) const {
    return filter(pred).butLast();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::butLast(const T &item) const {
    return butLast(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::butLastByIndex(const Func &pred) const {
    return filterByIndex(pred).butLast();
}

template<class T>
template<class Func, class>
constexpr T LazySeq<T>::first(const Func &pred) const {
    return filter(pred).first();
}

template<class T>
template<class Func, class>
constexpr T LazySeq<T>::firstByIndex(const Func &pred) const {
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
template<class Func>
constexpr auto LazySeq<T>::mapMany(const Func &func) const {
    return join(map(func));
}

template<class T>
template<class Func>
constexpr auto LazySeq<T>::mapManyByIndex(const Func &func) const {
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
template<class S, class Func, class R>
constexpr LazySeq<R> LazySeq<T>::match(const LazySeq<S> &other, const Func &func) const {
    return match(other).map(func);
}

template<class T>
template<class S, class Func, class R>
constexpr LazySeq<R> LazySeq<T>::matchByIndex(const LazySeq<S> &other, const Func &func) const {
    return match(other).mapByIndex(func);
}

template<class T>
LazySeq<LazySeq<T>> LazySeq<T>::groupBy(wide_size_t portion) const {
    return LazySeq<LazySeq<T>>(
            [*this, portion] {
                auto evaled = eval();
                if (!evaled.has_value()) {
                    return node_ptr<LazySeq<T>>();
                }
                auto curGroup = portion ? LazySeq(std::pair{evaled->first, evaled->second.take(portion - 1)})
                                        : LazySeq<T>();
                auto otherGroups = (portion ? evaled->second.skip(portion - 1) : *this).groupBy(portion);
                return std::make_optional<node<LazySeq<T>>>(curGroup, otherGroups);
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
template<class Func, class F>
LazySeq<std::pair<F, LazySeq<T>>> LazySeq<T>::groupBy(const Func &keyFinder) const {
    std::unordered_map<F, std::vector<T>> map;
    for (const auto &item: *this) {
        map[keyFinder(item)].emplace_back(item);
    }
    return makeLazy(std::move(map)).map(
            [](const auto &pair) { return std::pair{pair.first, makeLazy(std::move(pair.second))}; });
}

template<class T>
template<class KeyFinder, class ValueFunc, class Mapper, class F, class S, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupBy(const KeyFinder &keyFinder,
                                             const ValueFunc &valueFunc,
                                             const Mapper &seqFunc) const {
    return groupBy(keyFinder).map([valueFunc = valueFunc, seqFunc = seqFunc](const auto &pair) {
        return std::pair{pair.first, seqFunc(pair.second.map(valueFunc))};
    });
}

template<class T>
template<class KeyFinder, class ValueFunc, class F, class S, class>
LazySeq<std::pair<F, LazySeq<S>>> LazySeq<T>::groupBy(const KeyFinder &keyFinder,
                                                      const ValueFunc &valueFunc) const {
    return groupBy(keyFinder, valueFunc, identity<LazySeq<S>>);
}

template<class T>
template<class KeyFinder, class Mapper, class F, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupBy(const KeyFinder &keyFinder,
                                             const Mapper &seqFunc) const {
    return groupBy(keyFinder, identity<T>, seqFunc);
}

template<class T>
template<class KeyFinder, class ValueFunc, class Mapper, class F, class S, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupByIndexBy(const KeyFinder &keyFinder,
                                                    const ValueFunc &valueFunc,
                                                    const Mapper &seqFunc) const {
    return getIndexed().groupBy(keyFinder, valueFunc, seqFunc);
}

template<class T>
template<class KeyFinder, class Mapper, class F, class R>
LazySeq<std::pair<F, R>> LazySeq<T>::groupByIndexBy(const KeyFinder &keyFinder,
                                                    const Mapper &seqFunc) const {
    return getIndexed().groupBy(keyFinder, identity<indexed_t<T>>, seqFunc);
}

template<class T>
template<class KeyFinder, class ValueFunc, class F, class R, class>
LazySeq<std::pair<F, LazySeq<R>>> LazySeq<T>::groupByIndexBy(const KeyFinder &keyFinder,
                                                             const ValueFunc &valueFunc) const {
    return getIndexed().groupBy(keyFinder, valueFunc, identity<LazySeq<R>>);
}

template<class T>
template<class KeyFinder, class F>
LazySeq<std::pair<F, LazySeq<T>>> LazySeq<T>::groupByIndexBy(const KeyFinder &keyFinder) const {
    return getIndexed().groupBy(keyFinder, [](const indexed_t<T> &item) { return item.second; });
}

/*template<class T>
template<class Comparer, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const Comparer &comp) const {
  return makeOrdered().thenBy(comp);
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy() const {
  return orderBy(std::less<T>());
}

template<class T>
template<class Comp, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending(const Comp &comp) const {
  return makeOrdered().thenByDescending(comp);
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending() const {
  return orderByDescending(std::less<T>());
}

template<class T>
template<class R, class Func, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const std::function<R(T)> &func, const Func &comp) const {
  return makeOrdered().thenBy(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const std::function<R(T)> &func) const {
  return orderBy(func, std::less<R>());
}

template<class T>
template<class R, class Func, class>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                                  const Func &comp) const {
  return getIndexed().orderBy(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByIndexBy(const std::function<R(indexed_t<T>)> &func) const {
  return orderByIndexBy(func, std::less<R>());
}

template<class T>
template<class R, class Func, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending(const std::function<R(T)> &func,
                                                          const Func &comp) const {
  return makeOrdered().thenByDescending(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescending(const std::function<R(T)> &func) const {
  return orderByDescending(func, std::less<R>());
}

template<class T>
template<class R, class Func, class>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByDescendingByIndexBy(const std::function<R(indexed_t<T>)> &func,
                                                                              const Func &comp) const {
  return getIndexed().orderByDescending(func, comp);
}

template<class T>
template<class R>
constexpr OrderedLazySeq<indexed_t<T>> LazySeq<T>::orderByDescendingByIndexBy(const std::function<R(indexed_t<T>)> &func) const {
  return orderByDescendingByIndexBy(func, std::less<R>());
}*/

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::rest(const Func &pred) const {
    return filter(pred).rest();
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::restByIndex(const Func &pred) const {
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
template<class Mapper, class Predicate, class, class>
constexpr LazySeq<T> LazySeq<T>::mapIf(const Mapper &func, const Predicate &pred) const {
    return map([func = func, pred = pred](const T &a) { return pred(a) ? func(a) : a; });
}

template<class T>
template<class Mapper, class Predicate, class, class>
constexpr LazySeq<T> LazySeq<T>::mapIfByIndex(const Mapper &func, const Predicate &pred) const {
    return mapByIndex([func = func, pred = pred](const indexed_t<T> &pair) {
        return pred(pair) ? func(pair) : pair.second;
    });
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::remove(const Func &pred) const {
    return filter(negate(pred));
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::remove(const T &item) const {
    return remove(isEqualTo(item));
}

template<class T>
template<class Func, class>
constexpr LazySeq<T> LazySeq<T>::removeByIndex(const Func &pred) const {
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
template<class Func, class>
constexpr LazySeq<T>::LazySeq(const T &initializer, const Func &next)
        : LazySeq(node<T>{initializer, LazySeq(
        [next = next, initializer] { return LazySeq<T>(next(initializer), next).eval(); }
)}) {}

template<class T>
constexpr LazySeq<T>::LazySeq(wide_size_t count, const T &value) : LazySeq(count * LazySeq{value}) {}

template<class T>
template<class R>
bool LazySeq<T>::operator<(const LazySeq<R> &other) const {
    if (&evaluator_ == &other.evaluator_) {
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
constexpr void LazySeq<T>::foreach() const {
    for (const auto &item: *this);
}

template<class T>
template<class Func>
constexpr void LazySeq<T>::foreach(const Func &func) const {
    for (const auto &item: *this) {
        func(item);
    }
}

template<class T>
template<class Func>
constexpr void LazySeq<T>::foreachByIndex(const Func &func) const {
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
                    .map([](const auto &pair) { return std::tuple_cat(pair.first, pair.second); });
        }
    };

    template<class T>
    struct Pow<T, 1> {
        constexpr static auto invoke(const LazySeq<T> &seq) {
            return seq.map(std::make_tuple<T>);
        }
    };

    template<class T>
    struct Pow<T, 0> {
        constexpr static auto invoke(const LazySeq<T> &seq) {
            return LazySeq<std::tuple<>>();
        }
    };
}

template<class T>
template<wide_size_t n>
constexpr auto LazySeq<T>::pow() const {
    return Pow<T, n>::invoke(*this);
}

template<class T>
template<class Func, class>
constexpr LazySeq<T>::LazySeq(const Func &generator)
        : LazySeq(LazySeq([generator = generator] { return generator().eval(); })
                          .setSkipHelper([generator](wide_size_t count) { return generator().applySkipHelper(count); })

        /*.broadcastSkipHelper()*/) {}

template<class T>
constexpr LazySeq<T>::operator bool() const {
    return !isEmpty();
}

template<class T>
constexpr LazySeq<T> LazySeq<T>::setSkipHelper(const skip_helper_t &specialSkip) const {
    return LazySeq<T>(evaluator_, specialSkip);
}

template<class T>
constexpr bool LazySeq<T>::hasSpecialSkipHelper() const {
    return static_cast<bool>(skipHelper_);
}

template<class T>
auto LazySeq<T>::applySkipHelper(wide_size_t count) const {
    auto seq = *this;
    while (count) {
        if (seq.hasSpecialSkipHelper())
            return seq.skipHelper_(count);
        auto node = seq.eval();
        if (!node.has_value())
            return std::pair{count, LazySeq<T>()};
        seq = node->second;
        --count;
    }
    return std::pair{count, seq};
}

template<class T>
node_ptr<T> LazySeq<T>::broadcastSkipHelper(node_ptr<T> &&evaluated, LazySeq::skip_helper_t skipper, wide_size_t i) {
    if (skipper && evaluated.has_value() && !evaluated->second.hasSpecialSkipHelper())
        evaluated->second = LazySeq<T>(
                [i, skipper = skipper, next_ = evaluated->second] {
                    return broadcastSkipHelper(next_.evaluator_(), skipper, i + 1);
                },
                [i, skipper = skipper](wide_size_t count) { return skipper(i + count); });
    return evaluated;
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByItself() const {
    return orderByItselfWith(std::less<T>());
}

template<class T>
template<class Comparer, class Mapper, class R, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const Mapper &func, const Comparer &comp) const {
    return orderByItselfWith([comp = comp, func = func](const T &a, const T &b) { return comp(func(a), func(b)); });
}

template<class T>
template<class Mapper, class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderBy(const Mapper &func) const {
    return orderBy(func, std::less<R>());
}

template<class T>
template<class Comparer, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescendingByItselfWith(const Comparer &comp) const {
    return orderByItselfWith(descendingComparer(comp));
}

template<class T>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescendingByItself() const {
    return orderByDescendingByItselfWith(std::less<T>());
}

template<class T>
template<class Comparer, class Mapper, class R, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescendingBy(const Mapper &func, const Comparer &comp) const {
    return orderBy(func, descendingComparer(comp));
}

template<class T>
template<class Mapper, class R>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByDescendingBy(const Mapper &func) const {
    return orderByDescendingBy(func, std::less<R>());
}

template<class T>
template<class Comparer, class>
constexpr OrderedLazySeq<T> LazySeq<T>::orderByItselfWith(const Comparer &comp) const {
    return {SliceHolder<T>(toVector(), [comp = comp](const auto &cont) { separateMore(cont, comp); }).toLazySeq(),
            comp};
}

template<class T>
template<class Func, class>
auto LazySeq<T>::partition(typename std::vector<T>::iterator begin,
                           typename std::vector<T>::iterator end, const Func &comp) {
    auto pivot = *(begin + getRandomIndex((size_t) (end - begin)));
    auto border1 = std::partition(begin, end, [comp = comp, pivot](const auto &item) { return comp(item, pivot); });
    auto border2 = std::find_if_not(border1, end, isEqualTo(std::move(pivot)));
    return std::tuple{std::pair{begin, border1}, std::pair{border1, border2}, std::pair{border2, end}};
}

template<class T>
//template<bool stable>
template<class Func, class>
void LazySeq<T>::separateMore(const DataController<T> &dataController, const Func &comp) {
    if (const auto &lock = dataController.getGuard()) {
        auto[begin, end] = dataController.iterators();
        if (dataController.size() <= BUCKET_SIZE_FOR_STD_SORT_CALL) {
            if (dataController.size() > 1) {
                std::stable_sort(begin, end, comp);
            }
            dataController.setReady();
        } else {
            auto[less, equal, greater] = partition(begin, end, comp);
            auto data = dataController.data();
            auto lazySeparator = [comp](const auto &controller) { separateMore(controller, comp); };
            dataController.setSubHolders(lock,
                                         SliceHolder<T>(data, less.first, less.second, lazySeparator),
                                         SliceHolder<T>(data, equal.first, equal.second),
                                         SliceHolder<T>(data, greater.first, greater.second, lazySeparator));
        }
    }
}

template<class T>
constexpr LazySeq<T> range(const T &start, wide_size_t count) {
    if (count) {
        auto seq = LazySeq<T>([start, count] {
            T copy(start);
            return std::make_optional(std::pair{start, range(++copy, count - 1)});
        });
        if constexpr (adder<T>::hasPlus) {
            return seq.setSkipHelper([start, count](wide_size_t new_count) {
                return new_count >= count ? std::pair{new_count - count, LazySeq<T>()}
                                          : std::pair{0ull,
                                                      range(adder<T>::invoke(start, new_count), count - new_count)};
            });
        }
        return seq;
    } else {
        return LazySeq<T>();
    }
    //not infiniteRange(start).take(count);
    //because it is to slow yet because of additional virtual calls.
}

template<class T>
constexpr LazySeq<T> infiniteRange(const T &start) {
    return adder<T>::hasPlus
           ? LazySeq<T>(std::pair{start, LazySeq<T>([start] { return infiniteRange(increment(start)); })})
                   .setSkipHelper(
                           [start](wide_size_t count) {
                               return std::pair{(wide_size_t) 0, infiniteRange(adder<T>::invoke(start, count))};
                           })
           : LazySeq<T>(start, increment<T>);
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

template<class Func>
constexpr auto negate(const Func &pred) {
    return [pred = pred](const auto &... args) { return !pred(args...); };
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
        return seq.mapMany(
                [](wide_size_t index) { return LazySeq{(integer_t) index, -(integer_t) index}; });
    };
    return mapper(naturalNumbers())
            .setSkipHelper([mapper](wide_size_t count) {
                return std::pair{(wide_size_t) 0, mapper(naturalNumbers().skip(count / 2)).skip(count % 2)};
            })

                    /*.broadcastSkipHelper()*/
            .emplaceFront(0);
}

//Lazy bfs of Calkin-Wilf tree
//https://ru.wikipedia.org/wiki/Дерево_Калкина_—_Уилфа
//https://en.wikipedia.org/wiki/Calkin–Wilf_tree
LazySeq<rational_t> positiveRationalNumbers() {
    auto nextGenerator = [](rational_t q) -> rational_t {
        return {q.second, q.first / q.second * q.second * 2 - q.first + q.second};
    };
    return LazySeq<rational_t>(rational_t{1, 1}, nextGenerator)
            .setSkipHelper(
                    [nextGenerator](wide_size_t count) {
                        std::function<natural_t(wide_size_t)> fusc;
                        fusc = [&fusc](wide_size_t count) {
                            return count <= 1 ? count
                                              : count % 2 == 0
                                                ? fusc(count / 2)
                                                : fusc(count / 2) + fusc(count / 2 + 1);
                        };
                        return std::pair{(wide_size_t) 0,
                                         LazySeq<rational_t>(rational_t{fusc(count + 1), fusc(count + 2)},
                                                             nextGenerator)};
                    })

        /*.broadcastSkipHelper()*/;
}

LazySeq<rational_t> rationalNumbers() {
    auto mapper = [](const auto &seq) {
        return seq.mapMany([](rational_t r) { return LazySeq{r, rational_t{-r.first, r.second}}; });
    };
    return mapper(positiveRationalNumbers())
            .setSkipHelper([mapper](wide_size_t count) {
                return std::pair{(wide_size_t) 0, mapper(positiveRationalNumbers().skip(count / 2)).skip(count % 2)};
            })
                    /*.broadcastSkipHelper()*/
            .emplaceFront(rational_t{0, 1});
}

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<K> keys(const LazySeq<std::pair<K, V>> &seq) {
    return seq.map([](const auto &pair) { return pair.first; });
}

template<template<class> class LazySeq, class K, class V>
constexpr LazySeq<V> values(const LazySeq<std::pair<K, V>> &seq) {
    return seq.map([](const auto &pair) { return pair.second; });
}

template<class T>
constexpr auto isEqualTo(const T &item) {
    return partial(std::equal_to<T>(), item);
}

template<class T>
constexpr auto isNotEqualTo(const T &item) {
    return negate(isEqualTo(item));
}

template<class T>
constexpr auto isLessThan(const T &item) {
    return [item](const T &other) { return other < item; };
}

template<class T>
constexpr auto isGreaterThan(const T &item) {
    return partial(std::less<T>(), item);
}

template<class T>
constexpr auto isNotLessThan(const T &item) {
    return negate(isLessThan(item));
}

template<class T>
constexpr auto isNotGreaterThan(const T &item) {
    return negate(isGreaterThan(item));
}

template<class T>
constexpr auto dividesBy(const T &n) {
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
    return n <= 2
           ? (n + 1) / 2
           : n % 2 == 0 ? square(fibonacci(n / 2 + 1)) - square(fibonacci(n / 2 - 1))
                        : square(fibonacci(n / 2 + 1)) + square(fibonacci(n / 2));
}

template<class T>
constexpr LazySeq<T> fibonacciSeq() {
    auto seq = [](const std::pair<T, T> &start) {
        return LazySeq<std::pair<T, T>>(start,
                                        [](const auto &pair) {
                                            return std::pair{pair.second, pair.first + pair.second};
                                        });
    };
    return keys(seq({(T) 0, (T) 1})
                        .setSkipHelper(
                                [seq](wide_size_t index) {
                                    return std::pair{(wide_size_t) 0,
                                                     seq(std::pair<T, T>{fibonacci(index), fibonacci(index + 1)})};
                                }
                        )

            /*.broadcastSkipHelper()*/);
}

template<class T>
constexpr LazySeq<T> powersByFixedExponent(const T &exponent) {
    return infiniteRange<T>(0)
            .map([exponent](const T &base) { return LazySeq<T>{base}.repeat(exponent).multiply(); });
}

template<class T>
constexpr LazySeq<T> identitySeq(const T &item) {
    return infiniteRange(0).map(constantly(item));
}

template<class... Args>
LazySeq<wide_size_t> randomNumbers(Args... args) {
    return infiniteRange(0).map([args...](wide_size_t) { return getRandomIndex(args...); });
}

template<class F, class... Args>
constexpr decltype(auto) partial(const F &f, const Args &... args) {
    return [f = f, args...](const auto &... otherArgs) { return f(args..., otherArgs...); };
}

