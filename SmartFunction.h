//
// Created by zhele on 23.11.2019.
//

#ifndef FUNCTIONAL_SMART_FUNCTION_H
#define FUNCTIONAL_SMART_FUNCTION_H

#include <thread>

#include <boost/pool/object_pool.hpp>

template<class T>
struct Counter {
    T data;
    size_t uses = 1;

    explicit Counter(T data) : data(std::move(data)) {}
};

template<class F>
using FunctionalPool = boost::object_pool<Counter<F>>;

template<class F>
FunctionalPool<F> &getPool() {
    thread_local FunctionalPool<F> pool(12);
    return pool;
}

template<class F>
class FunctionHolder {
    std::thread::id owner = std::this_thread::get_id();
    Counter<F> *counter;

public:
    explicit FunctionHolder(Counter<F> *counter) : counter(counter) {}

    FunctionHolder(const FunctionHolder<F> &other)
            : counter(owner == other.owner ? (++other.counter->uses, other.counter)
                                           : (getPool<F>().construct(other.counter->data))) {}

    template<class T>
    explicit FunctionHolder(T functor) : counter(getPool<F>().construct(std::move(functor))) {}

    FunctionHolder &operator=(const FunctionHolder &other) {
        if (this != &other) {
            auto copy = FunctionHolder(other);
            std::swap(owner, copy.owner);
            std::swap(counter, copy.counter);
        }
        return *this;
    }

    template<class... Args>
    auto operator()(Args... args) const { return counter->data(std::forward<Args>(args)...); }

    ~FunctionHolder() {
        if (!--counter->uses) {
            assert(getPool<F>().is_from(counter));
            getPool<F>().destroy(counter);
        }
    }
};

template<class T>
FunctionHolder(T &&holder) -> FunctionHolder<std::remove_cv_t<T>>;

template<class Res, class... Args>
struct VTable {
    void (*copy)(const void *from, void *to);

    void (*destruct)(void *f);

    Res (*invoke)(const void *f, Args...);
};

template<class F, class Res, class... Args>
constexpr VTable<Res, Args...> vTable = {
        [](const void *from, void *to) { new(to) F(*static_cast<F const *>(from)); },
        [](void *f) { static_cast<F *>(f)->~F(); },
        [](const void *f, Args... args) -> Res { return (*static_cast<F const *>(f))(std::forward<Args>(args)...); }
};

template<class F, class Res, class... Args>
constexpr VTable<Res, Args...> fPointerVTable = {
        nullptr,
        nullptr,
        [](const void *f, Args... args) -> Res { return (*static_cast<F const *>(f))(std::forward<Args>(args)...); }
};

template<class T>
struct SmartFunction;

using FunctionHolderExample = FunctionHolder<int>; // All they have the same size and alignment.

constexpr size_t SmallObjectSize = 4 * sizeof(size_t);

constexpr size_t FunctionStorageSize = std::max(SmallObjectSize, sizeof(FunctionHolderExample));

template<class Res, class... Args>
struct SmartFunction<Res(Args...)> {
    std::aligned_storage_t<FunctionStorageSize, alignof(FunctionHolderExample)> data{};
    const VTable<Res, Args...> *curVTable;

    constexpr SmartFunction() : curVTable(nullptr) {}

    constexpr SmartFunction(std::nullptr_t) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
            : SmartFunction() {}

    template<class F>
    explicit SmartFunction(const FunctionHolder<F> &holder) noexcept
            : curVTable(&vTable<FunctionHolder<F>, Res, Args...>) {
        new(&data) FunctionHolder<F>(holder);
    }

    template<class F>
    /*implicit*/ SmartFunction(F functor) // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
    {
        if constexpr (std::is_convertible_v<F, Res(*)(Args...)>) {
            curVTable = &fPointerVTable<F, Res, Args...>;
        } else if constexpr (sizeof(F) <= FunctionStorageSize &&
                             alignof(F) <= alignof(FunctionHolderExample) &&
                             alignof(FunctionHolderExample) % alignof(F) == 0) {
            curVTable = &vTable<F, Res, Args...>;
            new(&data) F(functor);
        } else {
            new(this) SmartFunction<Res(Args...)>(FunctionHolder(std::move(functor)));
        }
    } //change

    SmartFunction(const SmartFunction &other) : curVTable(other.curVTable) {
        if (curVTable && curVTable->copy) {
            curVTable->copy(&other.data, &data);
        }
    }

    ~SmartFunction() {
        if (curVTable && curVTable->destruct) {
            curVTable->destruct(&data);
        }
    }

    SmartFunction &operator=(const SmartFunction &other) {
        if (this != &other) {
            auto copy = SmartFunction(other);
            std::swap(data, copy.data);
            std::swap(curVTable, copy.curVTable);
        }
        return *this;
    }

    Res operator()(Args... args) const {
        if (curVTable && curVTable->invoke) {
            return curVTable->invoke(&data, std::forward<Args>(args)...);
        } else {
            throw std::bad_function_call();
        }
    };

    explicit operator bool() const {
        return static_cast<bool>(curVTable);
    }
};

#endif //FUNCTIONAL_SMART_FUNCTION_H