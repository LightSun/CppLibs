#ifndef HREGEX_H
#define HREGEX_H

#include "table/Column.h"

struct real_pcre;

namespace h7 {
//split can't be used by multi thread. or-else. may cause crash.
    class Regex{
    public:
        Regex(CString pat);
        ~Regex();

        String replaceAll(CString str, CString target) const;
        bool match(CString str)const;
        inline sk_sp<IColumn<String>> extractStrs(CString str)const;
        inline sk_sp<IColumn<String>> split(CString str)const;
        inline sk_sp<IColumn<String>> groups(CString str)const;

        void split(CString str, IColumn<String>* out)const;
        void extractStrs(CString str,IColumn<String>* out)const;
        void groups(CString str,IColumn<String>* out)const;

        static inline bool match(CString pat, CString str){
            return Regex(pat).match(str);
        }

        inline String& getErrorMsg(){
            return m_error;
        }
    private:
        static constexpr int OVER_COUNT = 30;
        real_pcre* m_pcre_pat {nullptr};
        String m_error;
        int m_ovector[OVER_COUNT];
    };

    sk_sp<IColumn<String>> Regex::extractStrs(CString str)const{
        sk_sp<IColumn<String>> ret = sk_make_sp<IColumn<String>>();
        extractStrs(str, ret.get());
        return ret;
    }
    sk_sp<IColumn<String>> Regex::split(CString str)const{
        sk_sp<IColumn<String>> ret = sk_make_sp<IColumn<String>>();
        split(str, ret.get());
        return ret;
    }
    sk_sp<IColumn<String>> Regex::groups(CString str)const{
        sk_sp<IColumn<String>> ret = sk_make_sp<IColumn<String>>();
        groups(str, ret.get());
        return ret;
    }
}


#endif // HREGEX_H
