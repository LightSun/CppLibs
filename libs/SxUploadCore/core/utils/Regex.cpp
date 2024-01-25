#include <memory.h>
#include <sstream>
#include "utils/Regex.h"
#include "pcre.h"

using namespace h7;

Regex::Regex(CString pat){
    const char* pErrMsg;
    int nOffset = 0;
    m_pcre_pat = pcre_compile(pat.c_str(), 0, &pErrMsg, &nOffset, NULL);
    if(m_pcre_pat == nullptr){
        m_error = pErrMsg;
    }
}
Regex::~Regex(){
    if(m_pcre_pat){
        pcre_free(m_pcre_pat);
    }
}

bool Regex::match(CString pText)const{
    //int* ovector = (int*)&m_ovector[0];
    return pcre_exec(m_pcre_pat, NULL, pText.c_str(), pText.length(), 0,
                   0, (int*)&m_ovector[0], OVER_COUNT) > 0;
}

String Regex::replaceAll(CString pText,CString target) const{
    int ovector[OVER_COUNT];
    //the match count
    int rc;
    int exec_offset = 0;
    std::stringstream ss;
    do{
           rc = pcre_exec(m_pcre_pat, NULL, pText.c_str(), pText.length(), exec_offset,
                           0, ovector, OVER_COUNT);

           if( rc > 0 ) {
               //ovector[rc-1] is match position
               if(ovector[rc-1] > 0){
                    ss << pText.substr(exec_offset, ovector[rc-1] - exec_offset);
               }
               ss << target;
               // update offset
               exec_offset = ovector[1];
           }else{
               break;
           }
    }while( true );
    if(exec_offset != (int)pText.length()){
        ss << pText.substr(exec_offset, pText.length() - exec_offset);
    }
    return ss.str();
}

void Regex::extractStrs(CString pText, IColumn<String>* ret)const{
    int ovector[OVER_COUNT];
    int rc; //the match count
    int exec_offset = 0;
    const char *captured_string;
    do{
           //match start from exec_offset.
           rc = pcre_exec(m_pcre_pat, NULL, pText.c_str(), pText.length(), exec_offset,
                           0, ovector, OVER_COUNT);

           if( rc > 0 ) {
               //get match result
               //ovector[rc-1] is match position
               pcre_get_substring(pText.c_str(), ovector, rc, 0, &captured_string);
               //printf("captured string : %s\n", captured_string);
               ret->add_cons(captured_string);
               // set offset
               exec_offset = ovector[1];
           }else{
               break;
           }
    }while( true );
}

void Regex::split(CString pText, IColumn<String>* ret)const{
    int ovector[OVER_COUNT];
    int rc; //the match count
    int exec_offset = 0;
    int lastEnd = 0;
    do{
           //match start from exec_offset.
           rc = pcre_exec(m_pcre_pat, NULL, pText.c_str(), pText.length(), exec_offset,
                           0, ovector, OVER_COUNT);
           if( rc > 0 ) {
               if(ovector[rc-1] == 0){
                    ret->add_cons("");
               }else{
                    ret->add_cons(pText.c_str() + lastEnd, ovector[rc-1] - lastEnd);
               }
               //lastEnd += strlen(captured_string);
               lastEnd = exec_offset = ovector[1];
           }else{
               break;
           }
    }while( true );
    if(lastEnd == (int)pText.length()){
        ret->add_cons("");
    }else{
        ret->add_cons(pText.c_str() + lastEnd, pText.length() - lastEnd);
    }
}

void Regex::groups(CString pText,IColumn<String>* ret)const{
    int ovector[OVER_COUNT];
    int rc; //the match count
    rc = pcre_exec(m_pcre_pat, NULL, pText.c_str(), pText.length(), 0,
                   0, ovector, OVER_COUNT);
    if( rc > 0 ) {
       for(int i = 0; i < rc; i++) {
            //char *substring_start = pText.c_str() + ovector[2*i];
            //int substring_length = ovector[2*i+1] - ovector[2*i];
            ret->add_cons(pText.c_str() + ovector[2*i], ovector[2*i+1] - ovector[2*i]);
       }
    }
}
