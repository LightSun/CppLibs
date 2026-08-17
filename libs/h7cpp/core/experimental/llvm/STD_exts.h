#pragma once

#include "core/experimental/llvm/iterator.h"
#include <functional>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace h7 {
namespace detail {

using std::declval;

// We have to alias this since inlining the actual type at the usage site
// in the parameter list of iterator_facade_base<> below ICEs MSVC 2017.
template<typename... Iters> struct ZipTupleType {
    using type = std::tuple<decltype(*declval<Iters>())...>;
};

template <typename ZipType, typename ReferenceTupleType, typename... Iters>
using zip_traits = iterator_facade_base<
    ZipType,
    std::common_type_t<
        std::bidirectional_iterator_tag,
        typename std::iterator_traits<Iters>::iterator_category...>,
    // ^ TODO: Implement random access methods.
    ReferenceTupleType,
    typename std::iterator_traits<
        std::tuple_element_t<0, std::tuple<Iters...>>>::difference_type,
    // ^ FIXME: This follows boost::make_zip_iterator's assumption that all
    // inner iterators have the same difference_type. It would fail if, for
    // instance, the second field's difference_type were non-numeric while the
    // first is.
    ReferenceTupleType *, ReferenceTupleType>;

template <typename ZipType, typename ReferenceTupleType, typename... Iters>
struct zip_common : public zip_traits<ZipType, ReferenceTupleType, Iters...> {
    using Base = zip_traits<ZipType, ReferenceTupleType, Iters...>;
    using IndexSequence = std::index_sequence_for<Iters...>;
    using value_type = typename Base::value_type;

    std::tuple<Iters...> iterators;

protected:
    template <size_t... Ns> value_type deref(std::index_sequence<Ns...>) const {
        return value_type(*std::get<Ns>(iterators)...);
    }

    template <size_t... Ns> void tup_inc(std::index_sequence<Ns...>) {
        (++std::get<Ns>(iterators), ...);
    }

    template <size_t... Ns> void tup_dec(std::index_sequence<Ns...>) {
        (--std::get<Ns>(iterators), ...);
    }

    template <size_t... Ns>
    bool test_all_equals(const zip_common &other,
                         std::index_sequence<Ns...>) const {
        return ((std::get<Ns>(this->iterators) == std::get<Ns>(other.iterators)) &&
                ...);
    }

public:
    zip_common(Iters &&... ts) : iterators(std::forward<Iters>(ts)...) {}

    value_type operator*() const { return deref(IndexSequence{}); }

    ZipType &operator++() {
        tup_inc(IndexSequence{});
        return static_cast<ZipType &>(*this);
    }

    ZipType &operator--() {
        static_assert(Base::IsBidirectional,
                      "All inner iterators must be at least bidirectional.");
        tup_dec(IndexSequence{});
        return static_cast<ZipType &>(*this);
    }

    /// Return true if all the iterator are matching `other`'s iterators.
    bool all_equals(zip_common &other) {
        return test_all_equals(other, IndexSequence{});
    }
};

template <typename... Iters>
struct zip_first : zip_common<zip_first<Iters...>,
                              typename ZipTupleType<Iters...>::type, Iters...> {
    using zip_common<zip_first, typename ZipTupleType<Iters...>::type,
                     Iters...>::zip_common;

    bool operator==(const zip_first &other) const {
        return std::get<0>(this->iterators) == std::get<0>(other.iterators);
    }
};

template <typename... Iters>
struct zip_shortest
    : zip_common<zip_shortest<Iters...>, typename ZipTupleType<Iters...>::type,
                 Iters...> {
    using zip_common<zip_shortest, typename ZipTupleType<Iters...>::type,
                     Iters...>::zip_common;

    bool operator==(const zip_shortest &other) const {
        return any_iterator_equals(other, std::index_sequence_for<Iters...>{});
    }

private:
    template <size_t... Ns>
    bool any_iterator_equals(const zip_shortest &other,
                             std::index_sequence<Ns...>) const {
        return ((std::get<Ns>(this->iterators) == std::get<Ns>(other.iterators)) ||
                ...);
    }
};

/// Helper to obtain the iterator types for the tuple storage within `zippy`.
template <template <typename...> class ItType, typename TupleStorageType,
         typename IndexSequence>
struct ZippyIteratorTuple;

/// Partial specialization for non-const tuple storage.
template <template <typename...> class ItType, typename... Args,
         std::size_t... Ns>
struct ZippyIteratorTuple<ItType, std::tuple<Args...>,
                          std::index_sequence<Ns...>> {
    using type = ItType<decltype(adl_begin(
        std::get<Ns>(declval<std::tuple<Args...> &>())))...>;
};

/// Partial specialization for const tuple storage.
template <template <typename...> class ItType, typename... Args,
         std::size_t... Ns>
struct ZippyIteratorTuple<ItType, const std::tuple<Args...>,
                          std::index_sequence<Ns...>> {
    using type = ItType<decltype(adl_begin(
        std::get<Ns>(declval<const std::tuple<Args...> &>())))...>;
};

template <template <typename...> class ItType, typename... Args> class zippy {
private:
    std::tuple<Args...> storage;
    using IndexSequence = std::index_sequence_for<Args...>;

public:
    using iterator = typename ZippyIteratorTuple<ItType, decltype(storage),
                                                 IndexSequence>::type;
    using const_iterator =
        typename ZippyIteratorTuple<ItType, const decltype(storage),
                                    IndexSequence>::type;
    using iterator_category = typename iterator::iterator_category;
    using value_type = typename iterator::value_type;
    using difference_type = typename iterator::difference_type;
    using pointer = typename iterator::pointer;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;

    zippy(Args &&...args) : storage(std::forward<Args>(args)...) {}

    const_iterator begin() const { return begin_impl(IndexSequence{}); }
    iterator begin() { return begin_impl(IndexSequence{}); }
    const_iterator end() const { return end_impl(IndexSequence{}); }
    iterator end() { return end_impl(IndexSequence{}); }

private:
    template <size_t... Ns>
    const_iterator begin_impl(std::index_sequence<Ns...>) const {
        return const_iterator(adl_begin(std::get<Ns>(storage))...);
    }
    template <size_t... Ns> iterator begin_impl(std::index_sequence<Ns...>) {
        return iterator(adl_begin(std::get<Ns>(storage))...);
    }

    template <size_t... Ns>
    const_iterator end_impl(std::index_sequence<Ns...>) const {
        return const_iterator(adl_end(std::get<Ns>(storage))...);
    }
    template <size_t... Ns> iterator end_impl(std::index_sequence<Ns...>) {
        return iterator(adl_end(std::get<Ns>(storage))...);
    }
};
}

}
