#ifndef H_STRING_H
#define H_STRING_H

typedef struct hstring hstring;

//latter will opt.
hstring* hstring_append(hstring* owner, const char *str);
hstring* hstring_appendf(hstring* owner, const char *format, ...);
char* hstring_tostring(hstring* owner);

hstring* hstring_new();
void hstring_delete(hstring* owner);


#endif // H_STRING_H
