#pragma once

#include <vector>
#include <type_traits>
#include <iostream>
#include <algorithm>
#include "common/span_internal.h"

#define ASSERT_ERROR(expr)\
    if(!(expr)){\
        abort();\
    }

#define ASSERT_ERROR_X(expr, m)\
    if(!(expr)){\
        std::cout << m << std::endl;\
        abort();\
    }

namespace h7 {

/** //many comes from google absl::Span. but add some useful methods.
 * span, never holder the real data. just hold the data ptr.
 */
template<typename T>
class Span{
public:
    constexpr static auto npos = static_cast<size_t>(-1);
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;
private:
    // Used to determine whether a Span can be constructed from a container of
    // type C.
    template <typename C>
    using EnableIfConvertibleFrom =
        std::enable_if_t<!std::is_same_v<Span, std::remove_reference_t<C>> &&
                         span_internal::HasData<T, C>::value &&
                         span_internal::HasSize<C>::value>;

    // Used to SFINAE-enable a function when the slice elements are const.
    template <typename U>
    using EnableIfValueIsConst =
        typename std::enable_if<std::is_const<T>::value, U>::type;

    // Used to SFINAE-enable a function when the slice elements are mutable.
    template <typename U>
    using EnableIfValueIsMutable =
        typename std::enable_if<!std::is_const<T>::value, U>::type;

public:
    constexpr Span() noexcept : Span(nullptr, 0) {}
    constexpr Span(pointer array,size_type length) noexcept
        : ptr_(array), len_(length) {}

    // Implicit conversion constructors
    template <size_t N>
    constexpr Span(T(  // NOLINT(google-explicit-constructor)
        &a)[N]) noexcept
        : Span(a, N) {}

    // Explicit reference constructor for a mutable `Span<T>` type. Can be
    // replaced with MakeSpan() to infer the type parameter.
    template <typename V, typename = EnableIfConvertibleFrom<V>,
             typename = EnableIfValueIsMutable<V>,
             typename = span_internal::EnableIfNotIsView<V>>
    /*explicit*/ Span(
        V& v) noexcept  // NOLINT(runtime/references)
        : Span(span_internal::GetData(v), v.size()) {}

    // Implicit reference constructor for a read-only `Span<const T>` type
    template <typename V, typename = EnableIfConvertibleFrom<V>,
             typename = EnableIfValueIsConst<V>,
             typename = span_internal::EnableIfNotIsView<V>>
    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr Span(const V& v) noexcept
        : Span(span_internal::GetData(v), v.size()) {}

    // Overloads of the above two functions that are only enabled for view types.
    // This is so we can drop the ABSL_ATTRIBUTE_LIFETIME_BOUND annotation. These
    // overloads must be made unique by using a different template parameter list
    // (hence the = 0 for the IsView enabler).
    template <typename V, typename = EnableIfConvertibleFrom<V>,
             typename = EnableIfValueIsMutable<V>,
             span_internal::EnableIfIsView<V> = 0>
    /*explicit*/ Span(V& v) noexcept  // NOLINT(runtime/references)
        : Span(span_internal::GetData(v), v.size()) {}

    template <typename V, typename = EnableIfConvertibleFrom<V>,
             typename = EnableIfValueIsConst<V>,
             span_internal::EnableIfIsView<V> = 0>
    constexpr Span(const V& v) noexcept  // NOLINT(google-explicit-constructor)
        : Span(span_internal::GetData(v), v.size()) {}

    // Implicit constructor from an initializer list, making it possible to pass a
    // brace-enclosed initializer list to a function expecting a `Span`. Such
    // spans constructed from an initializer list must be of type `Span<const T>`.
    //
    //   void Process(absl::Span<const int> x);
    //   Process({1, 2, 3});
    //
    // Note that as always the array referenced by the span must outlive the span.
    // Since an initializer list constructor acts as if it is fed a temporary
    // array (cf. C++ standard [dcl.init.list]/5), it's safe to use this
    // constructor only when the `std::initializer_list` itself outlives the span.
    // In order to meet this requirement it's sufficient to ensure that neither
    // the span nor a copy of it is used outside of the expression in which it's
    // created:
    //
    //   // Assume that this function uses the array directly, not retaining any
    //   // copy of the span or pointer to any of its elements.
    //   void Process(absl::Span<const int> ints);
    //
    //   // Okay: the std::initializer_list<int> will reference a temporary array
    //   // that isn't destroyed until after the call to Process returns.
    //   Process({ 17, 19 });
    //
    //   // Not okay: the storage used by the std::initializer_list<int> is not
    //   // allowed to be referenced after the first line.
    //   absl::Span<const int> ints = { 17, 19 };
    //   Process(ints);
    //
    //   // Not okay for the same reason as above: even when the elements of the
    //   // initializer list expression are not temporaries the underlying array
    //   // is, so the initializer list must still outlive the span.
    //   const int foo = 17;
    //   absl::Span<const int> ints = { foo };
    //   Process(ints);
    //
    template <typename LazyT = T,
             typename = EnableIfValueIsConst<LazyT>>
    Span(std::initializer_list<value_type> v) noexcept  // NOLINT(runtime/explicit)
        : Span(v.begin(), v.size()) {}

    Span(const Span<T>& span){
        ptr_ = span.ptr_;
        len_ = span.len_;
    }
    Span(Span<T>& span){
        ptr_ = span.ptr_;
        len_ = span.len_;
    }
    Span& operator=(const Span<T>& span){
        ptr_ = span.ptr_;
        len_ = span.len_;
        return *this;
    }
    Span& operator=(Span<T>& span){
        ptr_ = span.ptr_;
        len_ = span.len_;
        return *this;
    }


    constexpr pointer data() const noexcept { return ptr_; }

    // Span::size()
    //
    // Returns the size of this span.
    constexpr size_type size() const noexcept { return len_; }

    // like java.size()
    constexpr int jsize() const noexcept { return len_; }

    // Span::length()
    //
    // Returns the length (size) of this span.
    constexpr size_type length() const noexcept { return size(); }

    // Span::empty()
    //
    // Returns a boolean indicating whether or not this span is considered empty.
    constexpr bool empty() const noexcept { return size() == 0; }

    // Span::operator[]
    //
    // Returns a reference to the i'th element of this span.
    reference operator[](size_type i) const {
        ASSERT_ERROR_X(i < size(), "index out if range. index = "
                                       << i << ",size = " << size());
        return ptr_[i];
    }

    // Span::at()
    //
    // Returns a reference to the i'th element of this span.
    reference at(size_type i) const {
        ASSERT_ERROR_X(i < size(), "index out if range. index = "
                                       << i << ",size = " << size());
        return *(data() + i);
    }

    // Span::front()
    //
    // Returns a reference to the first element of this span. The span must not
    // be empty.
    reference front() const {
        ASSERT_ERROR(size() > 0);
        return *data();
    }

    // Span::back()
    //
    // Returns a reference to the last element of this span. The span must not
    // be empty.
    reference back() const {
        ASSERT_ERROR(size() > 0);
        return *(data() + size() - 1);
    }

    // Span::begin()
    //
    // Returns an iterator pointing to the first element of this span, or `end()`
    // if the span is empty.
    constexpr iterator begin() const noexcept { return data(); }

    // Span::cbegin()
    //
    // Returns a const iterator pointing to the first element of this span, or
    // `end()` if the span is empty.
    constexpr const_iterator cbegin() const noexcept { return begin(); }

    // Span::end()
    //
    // Returns an iterator pointing just beyond the last element at the
    // end of this span. This iterator acts as a placeholder; attempting to
    // access it results in undefined behavior.
    constexpr iterator end() const noexcept { return data() + size(); }

    // Span::cend()
    //
    // Returns a const iterator pointing just beyond the last element at the
    // end of this span. This iterator acts as a placeholder; attempting to
    // access it results in undefined behavior.
    constexpr const_iterator cend() const noexcept { return end(); }

    // Span::rbegin()
    //
    // Returns a reverse iterator pointing to the last element at the end of this
    // span, or `rend()` if the span is empty.
    constexpr reverse_iterator rbegin() const noexcept {
        return reverse_iterator(end());
    }

    // Span::crbegin()
    //
    // Returns a const reverse iterator pointing to the last element at the end of
    // this span, or `crend()` if the span is empty.
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    // Span::rend()
    //
    // Returns a reverse iterator pointing just before the first element
    // at the beginning of this span. This pointer acts as a placeholder;
    // attempting to access its element results in undefined behavior.
    constexpr reverse_iterator rend() const noexcept {
        return reverse_iterator(begin());
    }

    // Span::crend()
    //
    // Returns a reverse const iterator pointing just before the first element
    // at the beginning of this span. This pointer acts as a placeholder;
    // attempting to access its element results in undefined behavior.
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    // Span mutations

    // Span::remove_prefix()
    //
    // Removes the first `n` elements from the span.
    void remove_prefix(size_type n) {
        ASSERT_ERROR(size() >= n);
        ptr_ += n;
        len_ -= n;
    }

    // Span::remove_suffix()
    //
    // Removes the last `n` elements from the span.
    void remove_suffix(size_type n) {
        ASSERT_ERROR(size() >= n);
        len_ -= n;
    }

    // Span::subspan()
    //
    // Returns a `Span` starting at element `pos` and of length `len`. Both `pos`
    // and `len` are of type `size_type` and thus non-negative. Parameter `pos`
    // must be <= size(). Any `len` value that points past the end of the span
    // will be trimmed to at most size() - `pos`. A default `len` value of `npos`
    // ensures the returned subspan continues until the end of the span.
    //
    // Examples:
    //
    //   std::vector<int> vec = {10, 11, 12, 13};
    //   absl::MakeSpan(vec).subspan(1, 2);  // {11, 12}
    //   absl::MakeSpan(vec).subspan(2, 8);  // {12, 13}
    //   absl::MakeSpan(vec).subspan(1);     // {11, 12, 13}
    //   absl::MakeSpan(vec).subspan(4);     // {}
    //   absl::MakeSpan(vec).subspan(5);     // throws std::out_of_range
    constexpr Span subspan(size_type pos = 0, size_type len = npos) const {
        ASSERT_ERROR_X(pos < size(), "pos out if range. pos = "
                                       << pos << ",size = " << size());
        return Span(data() + pos, std::min(size() - pos, len));
    }

    // Span::first()
    //
    // Returns a `Span` containing first `len` elements. Parameter `len` is of
    // type `size_type` and thus non-negative. `len` value must be <= size().
    //
    // Examples:
    //
    //   std::vector<int> vec = {10, 11, 12, 13};
    //   absl::MakeSpan(vec).first(1);  // {10}
    //   absl::MakeSpan(vec).first(3);  // {10, 11, 12}
    //   absl::MakeSpan(vec).first(5);  // throws std::out_of_range
    constexpr Span first(size_type len) const {
        ASSERT_ERROR(len <= size());
        return Span(data(), len);
    }

    // Span::last()
    //
    // Returns a `Span` containing last `len` elements. Parameter `len` is of
    // type `size_type` and thus non-negative. `len` value must be <= size().
    //
    // Examples:
    //
    //   std::vector<int> vec = {10, 11, 12, 13};
    //   absl::MakeSpan(vec).last(1);  // {13}
    //   absl::MakeSpan(vec).last(3);  // {11, 12, 13}
    //   absl::MakeSpan(vec).last(5);  // throws std::out_of_range
    constexpr Span last(size_type len) const {
        ASSERT_ERROR(len <= size());
        return Span(size() - len + data(), len);
    }

    std::vector<T> toVector()const{
        std::vector<T> vec(ptr_, ptr_ + len_);
        return vec;
    }

    void print(const std::string& tag)const{
        std::cout << "--- Span: " << tag << " ---"<< std::endl;
        for(auto& v: *this){
            std::cout << v << std::endl;
        }
    }
private:
    template<typename>
    friend class Spans;
    value_type* ptr_;
    size_type len_;
};

template<typename T, typename MockSpans>
class SpansIterator : public std::iterator<std::input_iterator_tag, T> {
public:
    using value_type = std::remove_cv_t<T>;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;

    SpansIterator(const MockSpans* spans, size_t index_ = 0)
        :spans_((MockSpans*)spans), index_(index_){
    }
    T& operator*() const{
        return ((*spans_)[index_]);
    }
    //prefix ++
    SpansIterator& operator++(){
        index_++;
        return *this;
    }
    //suffix ++
    SpansIterator operator++(int){
        SpansIterator tmp(spans_, index_);
        ++(*this);
        return tmp;
    }
    SpansIterator& operator+=(int v){
        index_ += v;
        return *this;
    }
    SpansIterator& operator-=(int v){
        if(index_ >= v){
            index_ -= v;
        }else{
            index_ = 0;
        }
        return *this;
    }
    bool operator==(const SpansIterator& other) const{
        return index_ == other.index_;
    }
    bool operator!=(const SpansIterator& other) const{
        return index_ != other.index_;
    }

private:
    MockSpans* spans_;
    size_t index_ {0};
};

template<typename T>
class Spans{
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = SpansIterator<T, Spans<T>>;
    using const_iterator = const iterator;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = size_t;

    Spans(const std::vector<Span<std::remove_cv_t<T>>>& spans):spans_(spans){
        compute0();
    }

    constexpr size_type size() const noexcept { return len_; }

    constexpr int jsize() const noexcept { return len_; }

    constexpr size_type length() const noexcept { return size(); }

    constexpr bool empty() const noexcept { return size() == 0; }
    //
    reference front() const {
        return spans_.front().front();
    }
    reference back() const {
        return spans_.back().back();
    }

    //
    constexpr iterator begin() const noexcept { return iterator(this); }

    constexpr const_iterator cbegin() const noexcept { return begin(); }

    constexpr iterator end() const noexcept { return iterator(this, size()); }

    constexpr const_iterator cend() const noexcept { return end(); }

    constexpr reverse_iterator rbegin() const noexcept {
        return reverse_iterator(end());
    }
    constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }

    constexpr reverse_iterator rend() const noexcept {
        return reverse_iterator(begin());
    }
    constexpr const_reverse_iterator crend() const noexcept { return rend(); }

    //
    reference operator[](size_type index) const {
        ASSERT_ERROR_X(index < size(), "index out if range. index = "
                                       << index << ",size = " << size());
        const int cnt = spans_.size();
        for(int i = 0 ; i < cnt - 1; ++i){
            if(index < span_offsets_[i + 1]){
                return spans_[i][index - span_offsets_[i]];
            }
        }
        return spans_[cnt-1][index - span_offsets_[cnt-1]];
    }
    reference at(size_type index) const {
        return operator[](index);
    }

    void remove_prefix(size_type n) noexcept {
        if(n > size()){
            n = size();
        }
        std::vector<int> rm_spanIds;
        int cnt = spans_.size();
        for(int i = 0 ; i < cnt && n > 0; ++i){
            if(n < spans_[i].size()){
                spans_[i].remove_prefix(n);
                n = 0;
            }else if(n == spans_[i].size()){
                rm_spanIds.push_back(i);
                n = 0;
            }else{
                rm_spanIds.push_back(i);
                n -= spans_[i].size();
            }
        }
        cnt = rm_spanIds.size();
        for(int i = cnt - 1; i >= 0 ; --i){
            spans_.erase(spans_.begin() + rm_spanIds[i]);
        }
        compute0();
    }
    void remove_suffix(size_type n) noexcept{
        if(n > size()){
            n = size();
        }
        std::vector<int> rm_spanIds;
        int cnt = spans_.size();
        for(int i = cnt - 1; i >= 0 && n > 0; --i){
            if(spans_[i].size() >= n){
                spans_[i].remove_suffix(n);
                n = 0;
            }else{
                n -= spans_[i].size();
                rm_spanIds.push_back(i);
            }
        }
        for(auto& id : rm_spanIds){
            spans_.erase(spans_.begin() + id);
        }
        compute0();
    }
    constexpr Spans subspan(size_type pos = 0, size_type len = Span<T>::npos) const {
        ASSERT_ERROR_X(pos < size(), "pos out if range. pos = "
                                         << pos << ",size = " << size());
        if(pos == 0){
            return first(len);
        }else{
            std::vector<Span<T>> kspans;
            {
                auto& span = spans_[0];
                auto size = span.size();
                int headLen = std::min(size - pos, len);
                kspans.push_back(span.subspan(pos, headLen));
                len -= headLen;
            }
            int cnt = spans_.size();
            for(int i = 1 ; i < cnt && len > 0; ++i){
                auto sc = spans_[i].size();
                if(sc <= len){
                    kspans.push_back(spans_[i]);
                    len -= sc;
                }else{
                    kspans.push_back(spans_[i].first(len));
                    len = 0;
                }
            }
            return Spans(kspans);
        }
    }

    constexpr Spans first(size_type len) const {
        ASSERT_ERROR(len <= size());
        std::vector<Span<T>> kspans;
        int cnt = spans_.size();
        for(int i = 0 ; i < cnt && len > 0; ++i){
            auto sc = spans_[i].size();
            if(sc <= len){
                kspans.push_back(spans_[i]);
                len -= sc;
            }else{
                kspans.push_back(spans_[i].first(len));
                len = 0;
            }
        }
        return Spans(kspans);
    }
    constexpr Spans last(size_type len) const {
        ASSERT_ERROR(len <= size());
        std::vector<Span<T>> kspans;
        int cnt = spans_.size();
        for(int i = cnt - 1 ; i >= 0 && len > 0; --i){
             auto sc = spans_[i].size();
             if(sc <= len){
                 kspans.push_back(spans_[i]);
                 len -= sc;
             }else{
                 kspans.push_back(spans_[i].last(len));
                 len = 0;
             }
        }
        std::reverse(kspans.begin(), kspans.end());
        return Spans(kspans);
    }

    std::vector<T> toVector()const{
        std::vector<T> vec;
        vec.reserve(size());
        for(auto& sp: spans_){
            auto svec = sp.toVector();
            vec.insert(vec.end(), svec.begin(), svec.end());
        }
        return vec;
    }

    void print(const std::string& tag)const{
        std::cout << "--- Spans: " << tag << " ---"<< std::endl;
        for(auto& v: *this){
            std::cout << v << std::endl;
        }
    }

private:
    void compute0(){
        len_ = 0;
        span_offsets_.clear();
        for(auto& s: spans_){
            span_offsets_.push_back(len_);
            len_ += s.size();
        }
    }

private:
    std::vector<Span<T>> spans_;
    std::vector<size_type> span_offsets_;
    size_type len_;
};
}
