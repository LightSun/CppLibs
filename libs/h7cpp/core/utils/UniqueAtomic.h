#ifndef UNIQUEATOMIC_H
#define UNIQUEATOMIC_H

#include <atomic>

namespace h7 {

   template<typename T>
   class UniqueAtomic{
   public:
     typedef T mutex_type;

     explicit UniqueAtomic(mutex_type& __m)
     : _M_device(std::__addressof(__m)), _M_owns(false)
     {
   lock();
   _M_owns = true;
     }

     ~UniqueAtomic()
     {
   if (_M_owns)
     unlock();
     }

     UniqueAtomic(const UniqueAtomic&) = delete;
     UniqueAtomic& operator=(const UniqueAtomic&) = delete;

     UniqueAtomic(UniqueAtomic&& __u) noexcept
     : _M_device(__u._M_device), _M_owns(__u._M_owns)
     {
   __u._M_device = 0;
   __u._M_owns = false;
     }

     UniqueAtomic& operator=(UniqueAtomic&& __u) noexcept
     {
   if(_M_owns)
     unlock();

   UniqueAtomic(std::move(__u)).swap(*this);

   __u._M_device = 0;
   __u._M_owns = false;

   return *this;
     }

     void
     lock()
     {
        bool excep = false;
        do {
        } while (!_M_device->compare_exchange_weak(excep, true));
       _M_owns = true;
     }

     void
     unlock()
     {
       if (_M_owns){
          bool excep = true;
           _M_device->compare_exchange_strong(excep, false);
           _M_owns = false;
       }
     }

     void
     swap(UniqueAtomic& __u) noexcept
     {
   std::swap(_M_device, __u._M_device);
   std::swap(_M_owns, __u._M_owns);
     }

     mutex_type*
     release() noexcept
     {
   mutex_type* __ret = _M_device;
   _M_device = 0;
   _M_owns = false;
   return __ret;
     }

     bool
     owns_lock() const noexcept
     { return _M_owns; }

     explicit operator bool() const noexcept
     { return owns_lock(); }

     mutex_type*
     mutex() const noexcept
     { return _M_device; }

   private:
     mutex_type*	_M_device;
     bool		_M_owns; // XXX use atomic_bool
   };

 template<typename _Mutex>
   inline void
   swap(UniqueAtomic<_Mutex>& __x, UniqueAtomic<_Mutex>& __y) noexcept
   { __x.swap(__y); }

   typedef UniqueAtomic<std::atomic<bool>> UniqueBool;
}

#endif // UNIQUEATOMIC_H
