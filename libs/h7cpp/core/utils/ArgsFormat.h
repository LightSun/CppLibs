#ifndef ARGSFORMAT_H
#define ARGSFORMAT_H

#include "common/common.h"

namespace h7 {
/**
  demo:
  auto af = ArgsFormat("<", ">")
  String fmt = "_<rep_count>_<sigma>";
  ft.parse(fmt);
  ft.getItems()[0].value="6"
  ft.getItems()[1].value="3"
  auto dst = ft.replaceAll(fmt);
  printf("ArgsFormat >> %s -> %s", fmt.data(), dst.data());
     * @brief The ArgsFormat class
     */
    class ArgsFormat{
    public:
        struct Item{
            String name;
            String value;
            int startPos;
        };
        ArgsFormat(CString prefix, CString suffix):m_prefix(prefix),
            m_suffix(suffix){
        }
        void clear(){
            m_items.clear();
        }
        std::vector<Item>& getItems(){
            return m_items;
        }
        void parse(CString fmt){
            m_items.clear();
            int start_pos = 0;
            do{
                start_pos = fmt_parse0(fmt, m_items, start_pos);
            }while (start_pos >= 0);
        }
        /**
         * @brief fmt_replace
         * @param src the format string to replace.
         * @param fmt the format item
         * @return the char count of add by replace
         */
        int replace(String& src, const Item& fmt){
            auto& str = fmt.value;
            //printf("fmt_replace >> name(%s) = %s \n", fmt.name.data(), str.data());
            src = src.replace(fmt.startPos, fmt.name.length()
                              + m_prefix.length() + m_suffix.length(), str);
            return str.length() - (fmt.name.length() + m_prefix.length()
                                   + m_suffix.length());
        }
        /**
         * @brief replaceAll replace all fmt item with value
         * @param src0 the string to replace
         * @return the replace result
         */
        String replaceAll(CString src0){
            String src = src0;
            int delta = 0;
            int size = (int)m_items.size();
            for(int i = 0 ; i < size ; ++i){
                 delta += replace(src, m_items[i]);
                 if(i + 1 < size){
                    m_items[i + 1].startPos += delta;
                 }
            }
            return src;
        }
    private:
        int fmt_parse0(CString fmt, std::vector<Item>& out, int start_pos){
            int index, index2;
            {
                index = fmt.find(m_prefix, start_pos);
                if(index < 0){
                    return -1;
                }
                index2 = fmt.find(m_suffix, index + m_prefix.length());
                if(index2 < 0){
                    return -1;
                }
            }
            Item item;
            item.name = fmt.substr(index + m_prefix.length(),
                                   index2 - index - m_prefix.length());
            item.startPos = index;
            out.push_back(std::move(item));
            return index + item.name.length() + m_prefix.length()
                    + m_suffix.length();
        }
    private:
        String m_prefix;
        String m_suffix;
        std::vector<Item> m_items;
    };
}

#endif // ARGSFORMAT_H
