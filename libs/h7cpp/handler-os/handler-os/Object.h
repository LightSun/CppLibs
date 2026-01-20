#pragma once

#include <string>
#include <functional>
#include <memory>

namespace h7_handler_os{

struct Object;
using SpPtr = std::shared_ptr<void>;
using Func_copy= std::function<SpPtr(const std::string& tag,
                                       SpPtr ptr,std::string* out_tag)>;
using Func_EQ = std::function<bool(const Object* o1, const Object* o2)>;

template<typename T>
std::shared_ptr<void> make_shared_void_ptr(T* ptr) {
    return std::shared_ptr<void>(ptr, [](void* p) {
        delete static_cast<T*>(p);
    });
}

struct Object{
    using String = std::string;
    using CString = const std::string&;

    std::string tag;
    SpPtr ptr;
    Func_copy func_cpy {nullptr};
    Func_EQ func_eq {nullptr};

    ~Object(){
        release();
    }
    Object(){}
    Object(const Object& src){
        Object* p = (Object*)&src;
        copyFrom(p);
    }

    template<typename T>
    T* ptrAs()const{
        return (T*)(ptr.get());
    }

    Object& operator=(const Object& src){
        Object* p = (Object*)&src;
        copyFrom(p);
        return *this;
    }
    friend bool operator == (Object const &a, Object const &b){
        return a.ptr == b.ptr || (a.func_eq && a.func_eq(&a, &b));
    }
    bool equals(Object* o, bool allowEq = true){
        if(ptr == o->ptr) return true;
        if(allowEq){
            auto& f_eq = func_eq ? func_eq : o->func_eq;
            if(f_eq && f_eq(this, o)){
                return true;
            }
        }
        return false;
    }
    void release(){
        ptr = nullptr;
        func_cpy = nullptr;
        func_eq = nullptr;
    }

    void copyFrom(Object* src){
        if(src && src->ptr != nullptr){
            this->tag = src->tag;
            this->func_cpy = src->func_cpy;
            this->func_eq = src->func_eq;
            if(src->func_cpy != nullptr){
                this->ptr = src->func_cpy(src->tag, src->ptr, &tag);
            }else{
                this->ptr = src->ptr;
            }
        }else{
            this->ptr = nullptr;
            this->func_cpy = nullptr;
            this->func_eq = nullptr;
        }
    }

    static Object make(CString tag, SpPtr ptr){
        return makeCopyEqual(tag, ptr, nullptr, nullptr);
    }
    static Object makeCopy(CString tag, SpPtr ptr,
                       Func_copy func_cpy){
        return makeCopyEqual(tag, ptr, func_cpy, nullptr);
    }
    static Object makeEqual(CString tag, SpPtr ptr,
                       Func_EQ func_eq){
        return makeCopyEqual(tag, ptr, nullptr, func_eq);
    }
    static Object makeCopyEqual(CString tag, SpPtr ptr,
                       Func_copy func_cpy,
                       Func_EQ func_eq){
        Object obj;
        obj.tag = tag;
        obj.ptr = ptr;
        obj.func_cpy = func_cpy;
        obj.func_eq = func_eq;
        return obj;
    }
};

}
