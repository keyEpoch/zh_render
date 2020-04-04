#include "geometry.h"

template <> template <> vec<3,int>  ::vec(const vec<3,float> &v) : x(int(v.x+.5f)),y(int(v.y+.5f)),z(int(v.z+.5f)) {}
template <> template <> vec<3,float>::vec(const vec<3,int> &v)   : x(v.x),y(v.y),z(v.z) {}
template <> template <> vec<2,int>  ::vec(const vec<2,float> &v) : x(int(v.x+.5f)),y(int(v.y+.5f)) {}
template <> template <> vec<2,float>::vec(const vec<2,int> &v)   : x(v.x),y(v.y) {}



/*
code_1:
#include <iostream>

template<class T1, class T2> struct foo
{
  void doStuff() { std::cout << "generic foo "; }
};

template<class T1>
struct foo<T1, int>
{
 void doStuff() { std::cout << "specific foo with T2=int"; }
};




code_2:
template<class T1> struct bar
{
  void doStuff() { std::cout << "generic bar"; }
};

template<>
struct bar<int>
{
 void doStuff() { std::cout << "specific bar with T1=int"; }
};

为什么要像上面那样有
template<> template<> ***
看上面两段代码就知道了

带有<>说明就是模版，就需要关键字 template<>

以这个为例：
template <> template <> vec<3,int>  ::vec(const vec<3,float> &v) : x(int(v.x+.5f)),y(int(v.y+.5f)),z(int(v.z+.5f)) {}
第一个template<> 是用来修饰 vec<3, int> 的
第二个template<> 是用来修饰 vec<3, float> 的
*/