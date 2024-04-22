#pragma once

#include <random>
#include <string>

namespace h7 {
    class Random{
    public:
        using String = std::string;

        static int nextInt(int min, int max){
            std::random_device rd;
            std::mt19937 gen(rd()); //gen是一个使用rd()作种子初始化的标准梅森旋转算法的随机数发生器
            std::uniform_int_distribution<> distrib(min, max);//[0,255]
            return distrib(gen);
        }
        static String nextStr(int len){
            std::random_device rd;
            std::mt19937 gen(rd()); //gen是一个使用rd()作种子初始化的标准梅森旋转算法的随机数发生器
            std::uniform_int_distribution<> distrib(0, 255);
            String str;
            str.resize(len);
            for(int i = 0 ; i < len ; ++i){
                 ((char*)str.data())[i] = distrib(gen);
            }
            return str;
        }
    };
}
