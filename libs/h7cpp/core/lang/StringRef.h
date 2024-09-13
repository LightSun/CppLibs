
#include "common/common.h"

namespace h7 {
    class StringRef{
    public:
        using u32 = unsigned int;
        using Long = long long;
        using ULong = unsigned long long;

        explicit StringRef(char* data, u32 start, u32 len);
        explicit StringRef(char* data);
        explicit StringRef(String* ref);
        //malloc
        StringRef(u32 len, bool resize);
        StringRef(const char* data, u32 start, u32 len);
        StringRef(const char* data);
        StringRef(const String& ref);
        StringRef(const StringRef& ref);
        ~StringRef();

        void clear();
        void clearLength(){m_validLen = 0;}
        String toString();
        void prepareSize(u32 size);
        void resize(u32 size);

        //start: include, end exclude
        StringRef subString(u32 start, u32 end = 0)const;
        bool startsWith(const StringRef& ref)const;
        bool endsWith(const StringRef& ref)const;

        StringRef reverse() const;
        StringRef& toLower();
        StringRef& toUpper();

        int indexOf(char c, int fromIndex = 0)const;
        int lastIndexOf(char c, int fromIndex = 0)const;
        int indexOf(const StringRef& ref, int fromIndex = 0)const;
        int lastIndexOf(const StringRef& ref, int fromIndex = -1)const;

        Long toLong(Long def = 0, int base = 10);
        ULong toULong(ULong def = 0, int base = 10);
        int toInt(int def = 0, int base = 10);
        float toFloat(float def = 0);
        float toDouble(float def = 0);
        bool toBool();

        StringRef& append(const StringRef& ref);
        StringRef& append(const char* v){
            return append(StringRef((char*)v, 0, strlen(v)));
        }
        StringRef& append(const String& ref){
            return append(StringRef((String*)&ref));
        }
        StringRef& append(char c){
            return append(String(1, c));
        }
        StringRef& append(unsigned char c){
            return append(std::to_string(c));
        }
        StringRef& append(unsigned short c){
            return append(std::to_string(c));
        }
        StringRef& append(short c){
            return append(std::to_string(c));
        }
        StringRef& append(int c){
            return append(std::to_string(c));
        }
        StringRef& append(unsigned int c){
            return append(std::to_string(c));
        }
        StringRef& append(long c){
            return append(std::to_string(c));
        }
        StringRef& append(unsigned long c){
            return append(std::to_string(c));
        }
        StringRef& append(float c){
            return append(std::to_string(c));
        }
        StringRef& append(double c){
            return append(std::to_string(c));
        }
        StringRef& append(bool c){
            return append(c ? "true" : "false");
        }
        StringRef& append(long long c){
            return append(std::to_string(c));
        }
        StringRef& append(unsigned long long c){
            return append(std::to_string(c));
        }

        StringRef copy() const{
            return StringRef(*this);
        }
        const char* data() const{
            return m_data;
        }
        char charAt(int index) const{
            return m_data[index];
        }
        bool isEmpty() const{
            return m_validLen == 0;
        }
        int length() const{
            return m_validLen;
        }

        int hashCode();
        bool equals(const StringRef& ref) const;
        bool equalsIgnoreCase(const StringRef& ref) const;

        int compareTo(const StringRef& ref) const;

//----------------------
        StringRef& operator=(const StringRef& ref){
            m_validLen = ref.m_validLen;
            alloc0(m_validLen);
            memcpy(m_data, ref.m_data, m_validLen);
            return *this;
        }
        char& operator[](int i){
            return m_data[i];
        }

    private:
        void alloc0(u32 real_len){
            if(real_len > m_allocLen){
                if(m_data){
                    m_data = (char*)realloc(m_data, real_len);
                }else{
                    m_data = (char*)malloc(real_len);
                }
                m_allocLen = real_len;
            }
        }

    private:
        char* m_data {nullptr};
        unsigned int m_allocLen {0};
        unsigned int m_validLen {0};
        bool m_selfOwned {false};
    };
    //-------------------
    inline StringRef::StringRef(u32 len, bool resize){
        alloc0(len + 1);
        m_data[m_allocLen-1] = '\0';
        m_selfOwned = true;
        if(resize){
            m_validLen = len;
        }
    }
    inline StringRef::StringRef(char* data, u32 start, u32 len){
        m_data = data + start;
        m_validLen = len;
    }
    inline StringRef::StringRef(const char* data, u32 start, u32 len){
        u32 old = m_validLen;
        u32 real_len = strlen(data) - start;
        real_len = HMIN(len, real_len);
        m_validLen += real_len;
        alloc0(m_validLen + 1);
        memcpy(m_data + old, (char*)data + start, real_len);
        m_data[m_validLen] = '\0';
        m_selfOwned = true;
    }
    inline StringRef::StringRef(char* data){
        m_data = data;
        m_validLen = strlen(data);
    }
    inline StringRef::StringRef(const char* data){
        m_validLen = strlen(data);
        alloc0(m_validLen + 1);
        memcpy(m_data, data, m_validLen + 1);
        m_selfOwned = true;
    }
    inline StringRef::StringRef(const StringRef& ref){
        m_validLen = ref.m_validLen;
        alloc0(m_validLen + 1);
        memcpy(m_data, ref.m_data, m_validLen + 1);
        m_selfOwned = true;
    }
    inline StringRef::StringRef(const String& ref){
        m_validLen = ref.length();
        alloc0(m_validLen + 1);
        memcpy(m_data, ref.data(), m_validLen + 1);
        m_selfOwned = true;
    }
    inline StringRef::StringRef(String* ref){
        m_data = (char*)&ref->data()[0];
        m_validLen = ref->length();
    }
    inline StringRef::~StringRef(){
        if(m_data && m_selfOwned){
            free(m_data);
            m_data = nullptr;
        }
    }
    inline String StringRef::toString(){
        auto old = m_data[m_validLen];
        if(old != '\0'){
            m_data[m_validLen] = '\0';
            String ret(m_data, m_validLen);
            m_data[m_validLen] = old;
            return ret;
        }
        return String(m_data, m_validLen);
    }
    inline void StringRef::clear(){
        m_validLen = 0;
        if(m_selfOwned && m_data){
            free(m_data);
            m_data = nullptr;
            m_allocLen = 0;
        }
        alloc0(1);
    }

    inline void StringRef::prepareSize(u32 size){
         alloc0(size + 1);
    }
    inline void StringRef::resize(u32 size){
        prepareSize(size);
        m_validLen = size;
    }
    inline StringRef StringRef::subString(u32 start, u32 end)const{
        if(m_data == nullptr){
            return StringRef((u32)0, false);
        }
        if(end <= 0){
            end = m_validLen;
        }else if(end > m_validLen){
            end = m_validLen;
        }
        int size = end - start;
        return StringRef(m_data, start, size);
    }
    inline bool StringRef::startsWith(const StringRef& ref)const{
        if(m_validLen < ref.m_validLen){
            return false;
        }
        return strncmp(m_data, ref.m_data, ref.m_validLen) == 0;
    }
    inline bool StringRef::endsWith(const StringRef& ref)const{
        if(m_validLen < ref.m_validLen){
            return false;
        }
        // 123  23
        int start = m_validLen - ref.m_validLen;
        return strncmp(m_data + start, ref.m_data, ref.m_validLen) == 0;
    }
    inline StringRef StringRef::reverse()const{
        StringRef s(m_validLen, true);
        for(u32 i = 0 ; i < m_validLen ; ++i){
            s.m_data[m_validLen - 1 - i] = m_data[i];
        }
        return s;
    }
    inline StringRef& StringRef::toLower(){
        for(u32 i = 0 ; i < m_validLen ; ++i){
            if(m_data[i] >= 'A' && m_data[i] <= 'Z'){
                m_data[i] = m_data[i] + 32;
            }
        }
        return *this;
    }
    inline StringRef& StringRef::toUpper(){
        for(u32 i = 0 ; i < m_validLen ; ++i){
            if(m_data[i] >= 'a' && m_data[i] <= 'z'){
                m_data[i] = m_data[i] - 32;
            }
        }
        return *this;
    }
    inline int StringRef::indexOf(char c, int fromIndex)const{
        for(u32 i = fromIndex ; i < m_validLen ; ++i){
            if(m_data[i] == c){
                return i;
            }
        }
        return -1;
    }
    inline int StringRef::lastIndexOf(char c, int fromIndex)const{
        if(fromIndex == -1){
            fromIndex = m_validLen -1;
        }
        for(int i = fromIndex ; i >= 0 ; --i){
            if(m_data[i] == c){
                return i;
            }
        }
        return -1;
    }
    inline int StringRef::indexOf(const StringRef& ref, int fromIndex)const{
        if(fromIndex < 0){
            return -1;
        }
        int len = m_validLen - ref.m_validLen - fromIndex;
        if(len < 0) return -1;
        for(int i = 0 ; i <= len ; ++i){
            if(strncmp(m_data + fromIndex + i, ref.m_data, ref.m_validLen) == 0){
                return fromIndex + i;
            }
        }
        return -1;
    }
    inline int StringRef::lastIndexOf(const StringRef& ref, int fromIndex)const{
        if(fromIndex == -1){
            fromIndex = m_validLen -1;
        }
        return indexOf(ref, fromIndex - ref.m_validLen + 1);
    }
    inline StringRef& StringRef::append(const StringRef& ref){
        int old = m_validLen;
        m_validLen += ref.m_validLen;
        alloc0(m_validLen + 1);
        memcpy(m_data + old, ref.m_data, ref.m_validLen);
        m_data[m_validLen] = '\0';
        return *this;
    }

    //------------------------
    inline StringRef::ULong StringRef::toULong(ULong def, int base){
        auto str = toString();
        try{
               return std::stoull(str, nullptr, base);
           }catch (std::invalid_argument&){
           }catch (std::out_of_range&){
           }catch (...) {
           }
        return def;
    }
    inline StringRef::Long StringRef::toLong(Long def, int base){
        auto str = toString();
        try{
               return std::stoll(str, nullptr, base);
           }catch (std::invalid_argument&){
           }catch (std::out_of_range&){
           }catch (...) {
           }
        return def;
    }
    inline bool StringRef::toBool(){
        return equals("true") || equals("TRUE");
    }
    inline int StringRef::toInt(int def, int base){
        auto str = toString();
        try{
               return std::stoi(str, nullptr, base);
           }catch (std::invalid_argument&){
           }catch (std::out_of_range&){
           }catch (...) {
           }
        return def;
    }
    inline float StringRef::toFloat(float def){
        auto str = toString();
        try{
               return std::stof(str);
           }catch (std::invalid_argument&){
           }catch (std::out_of_range&){
           }catch (...) {
           }
        return def;
    }
    inline float StringRef::toDouble(float def){
        auto str = toString();
        try{
               return std::stod(str);
           }catch (std::invalid_argument&){
           }catch (std::out_of_range&){
           }catch (...) {
           }
        return def;
    }
    inline bool StringRef::equals(const StringRef& ref)const{
        if(m_validLen != ref.m_validLen){
            return false;
        }
        return strncmp(m_data, ref.m_data, m_validLen) == 0;
    }
    inline bool StringRef::equalsIgnoreCase(const StringRef& ref)const{
        if(m_validLen != ref.m_validLen){
            return false;
        }
        for(u32 i = 0 ; i < m_validLen ; ++i){
            int dela = m_data[i] - ref.m_data[i];
            if(dela == 0) continue;
            if(dela == 32){
                if(m_data[i] < 'a' || m_data[i] > 'z'){
                    return false;
                }
            }else if(dela == -32){
                if(m_data[i] < 'A' || m_data[i] > 'Z'){
                    return false;
                }
            }else{
                return false;
            }
        }
        return true;
//        auto s1 = toLower();
//        auto s2 = ref.toLower();
//        return s1.equals(s2);
    }
    inline int StringRef::compareTo(const StringRef& oth)const{
        int len1 = m_validLen;
        int len2 = oth.m_validLen;
        int lim = HMIN(len1, len2);
        const char* v1 = m_data;
        const char* v2 = oth.m_data;

        int k = 0;
        while (k < lim) {
           char c1 = v1[k];
           char c2 = v2[k];
           if (c1 != c2) {
               return c1 - c2;
           }
           k++;
        }
        return len1 - len2;
    }
}
inline bool operator!=(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) != 0;
}
inline bool operator==(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) == 0;
}
inline bool operator<(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) < 0;
}
inline bool operator<=(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) <= 0;
}
inline bool operator>(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) > 0;
}
inline bool operator>=(const h7::StringRef& ref1, const h7::StringRef& ref2){
    return ref1.compareTo(ref2) >= 0;
}
inline h7::StringRef operator+(const h7::StringRef& ref1, const h7::StringRef& ref2){
    auto ret = ref1.copy();
    ret.append(ref2);
    return ret;
}
