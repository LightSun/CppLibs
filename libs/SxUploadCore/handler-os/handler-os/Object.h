#pragma once

#include <string>
#include <functional>

namespace h7_handler_os{

typedef std::function<void(const std::string& tag, void* ptr)> Func_free;
typedef std::function<void*(const std::string& tag, void* ptr,std::string* out_tag)> Func_copy;

struct Object{
    using String = std::string;
    using CString = const std::string&;

    std::string tag;
    void* ptr {nullptr};
    Func_free func_free {nullptr};
    Func_copy func_cpy {nullptr};

    typedef std::function<bool(const Object* o1, const Object* o2)> Func_EQ;
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
        return (T*)(ptr);
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
        if(ptr && func_free){
            func_free(tag, ptr);
        }
        ptr = nullptr;
        func_free = nullptr;
        func_cpy = nullptr;
        func_eq = nullptr;
    }

    void copyFrom(Object* src){
        if(src && src->ptr != nullptr){
            this->tag = src->tag;
            this->func_cpy = src->func_cpy;
            this->func_free = src->func_free;
            this->func_eq = src->func_eq;
            if(src->func_cpy != nullptr){
                this->ptr = src->func_cpy(src->tag, src->ptr, &tag);
            }else{
                this->ptr = src->ptr;
                //avoid free twice or more.
                src->func_free = nullptr;
            }
        }else{
            this->ptr = nullptr;
            this->func_free = nullptr;
            this->func_cpy = nullptr;
            this->func_eq = nullptr;
        }
    }

    static Object make(CString tag, void* ptr,
                       Func_free func_free = nullptr){
        return makeCopyEqual(tag, ptr, func_free, nullptr, nullptr);
    }
    static Object makeCopy(CString tag, void* ptr,
                       Func_free func_free,
                       Func_copy func_cpy){
        return makeCopyEqual(tag, ptr, func_free, func_cpy, nullptr);
    }
    static Object makeEqual(CString tag, void* ptr,
                       Func_free func_free,
                       Func_EQ func_eq){
        return makeCopyEqual(tag, ptr, func_free, nullptr, func_eq);
    }
    static Object makeCopyEqual(CString tag, void* ptr,
                       Func_free func_free,
                       Func_copy func_cpy,
                       Func_EQ func_eq){
        Object obj;
        obj.tag = tag;
        obj.ptr = ptr;
        obj.func_free = func_free;
        obj.func_cpy = func_cpy;
        obj.func_eq = func_eq;
        return obj;
    }
};

}
