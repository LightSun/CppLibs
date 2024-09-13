#pragma once

#include <vector>

namespace h7 {

//placement new: from google chromium
/**
 //example
void foo() {
  // std::string析构函数不会被调用，即便出了foo的scope
  NoDestructor<std::string> s("Hello world!");
}
 */
template <typename T>
class NoDestructor {
 public:
  // Not constexpr; just write static constexpr T x = ...; if the value should
  // be a constexpr.
  template <typename... Args>
  explicit NoDestructor(Args&&... args) {
    new (storage_) T(std::forward<Args>(args)...);
  }

  // Allows copy and move construction of the contained type, to allow
  // construction from an initializer list, e.g. for std::vector.
  explicit NoDestructor(const T& x) { new (storage_) T(x); }
  explicit NoDestructor(T&& x) { new (storage_) T(std::move(x)); }

  NoDestructor(const NoDestructor&) = delete;
  NoDestructor& operator=(const NoDestructor&) = delete;

  ~NoDestructor() = default;

  const T& operator*() const { return *get(); }
  T& operator*() { return *get(); }

  const T* operator->() const { return get(); }
  T* operator->() { return get(); }

  const T* get() const { return reinterpret_cast<const T*>(storage_); }
  T* get() { return reinterpret_cast<T*>(storage_); }

 private:
  alignas(T) char storage_[sizeof(T)];
};
}
