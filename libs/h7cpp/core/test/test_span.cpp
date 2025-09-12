#include "core/utils/Span.h"

using namespace h7;

void test_span(){
    {
        int a[] = {1,2,3,4,5};

        Span<int> span(a, 5);
        auto b = span;
        auto v = b.back();
        printf("v = %d\n", v);
        Span<int> c(span);
        v = c.back();
        printf("v = %d\n", v);
    }
    {
        std::vector<int> a = {1,2,3,4,5};

        Span<int> span(a);
        auto v = span.back();
        printf("v = %d\n", v);
    }
    {
        std::initializer_list<int> a = {1,2,3,4,5};

        Span<int> span(a);
        auto v = span.back();
        printf("v = %d\n", v);
        for(auto& v : span){
            printf("v = %d\n", v);
        }
    }
    {
        std::vector<int> a = {1,2,3,4,5};
        std::vector<int> b = {6,7,8,9,10};
        Spans<int> spans({a,b});
        auto v = spans[5];
        printf("v = %d\n", v);

        v = spans[3];
        printf("v = %d\n", v);
        for(auto& v: spans){
            printf("v = %d\n", v);
        }
        spans.remove_prefix(5);
        printf("spans >> size = %ld\n", spans.size());
        spans = Spans<int>({a,b});
        spans.remove_prefix(6);
        printf("spans >> size = %ld\n", spans.size());
        //
        spans = Spans<int>({a,b});
        spans.remove_suffix(5);
        printf("spans >> size = %ld\n", spans.size());

        spans = Spans<int>({a,b});
        spans.remove_suffix(3);
        printf("spans >> size = %ld\n", spans.size());

        spans = Spans<int>({a,b});
        spans.remove_suffix(6);
        printf("spans >> size = %ld\n", spans.size());
    }
    {
        printf("Spans >> test_first ... \n");
        std::vector<int> a = {1,2,3,4,5};
        std::vector<int> b = {6,7,8,9,10};
        Spans<int> spans({a,b});
        auto spans2 = spans.first(6);
        spans2.print("test_first1");
        spans2 = spans.last(6);
        spans2.print("test_first2");
    }
    {
        printf("Spans >> test_sub ... \n");
        std::vector<int> a = {1,2,3,4,5};
        std::vector<int> b = {6,7,8,9,10};
        Spans<int> spans({a,b});
        auto spans2 = spans.subspan(0, 6);
        spans2.print("test_sub1");
        spans2 = spans.subspan(2, 6);
        spans2.print("test_sub2");
    }
}
