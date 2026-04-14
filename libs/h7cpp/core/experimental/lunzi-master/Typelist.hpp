#pragma once

#include <type_traits>

namespace Px
{
	template <bool...>
	struct tagpack;

	//Types必须是 base on std::integral_constant<bool, value> or std::is_same<type1, type2>
	//与运算 全部都能找到
	template <typename... Types>
	struct and_all : std::is_same<tagpack<true, Types::value...>, tagpack<Types::value..., true> > {};

	template <typename Type>
	struct not_not : std::integral_constant<bool, !Type::value>
	{};

	//或运算 只要有1个能找到
	template <typename... Types>
	struct or_all : not_not <std::is_same<tagpack<false, Types::value...>, tagpack<Types::value..., false>>> {};

	template <typename... T> //必须有这个申明 A
	struct typeList;

	struct nullType {};

	//只是类型相关 并不需要定义实际的 应用数据结构 测试时需要看具体类型操作是否正确 打开注释
	/*
	template <typename T>
	struct typeList<T> : nullType
	{
	         typedef T Type;
	         enum { value = 0 };
	         int nV;
	         typeList() { nV = value; }
	};
	 
	template <typename T, typename... Types>
	struct typeList<T, Types... > : typeList<Types...>        //typeList<T, Types... >建立在申明A上 否则编译不过
	{
	         typedef T Type;
	         enum { value = typeList<Types...>::value + 1 };
	         int nV;
	         typeList() { nV = value; }
	};
	*/

	//判断type是否在typeList中
	template <typename T, typename Typelist>
	struct is_type_in_typeList;

	template <typename T, typename... Ts>
	struct is_type_in_typeList<T, typeList<Ts...> > : or_all<std::is_same<T, Ts>... > {};

	//type在typeList中index
	template <typename... Types>
	struct type_index_in_typeList;

	template <typename T>
	struct type_index_in_typeList<T, typeList<> >
	{
		enum { value = std::false_type::value - 1 };
	};

	template <typename T, typename... Types>
	struct type_index_in_typeList<T, typeList<T, Types...>>
	{
		enum { value = 0 };
	};

	template <typename T, typename U, typename... Types>
	struct type_index_in_typeList<T, typeList<U, Types...> >
	{
		enum { value = !is_type_in_typeList<T, typeList<U, Types...> >::value ? -1 : 1 + type_index_in_typeList<T, typeList<Types...> >::value };
	};

	//根据Idx获取type
	template<int Idx, typename Typelist>
	struct type_by_index;

	template <int Idx>
	struct type_by_index<Idx, typeList<> > //idx超过list的最大长度 - 1 则编译器赋值时直接报错
	{
		typedef nullType Type;
	};

	template<typename T, typename... Types>
	struct type_by_index<0, typeList<T, Types...> >
	{
		typedef T Type;
	};

	template <int Idx, typename T, typename... Types >
	struct type_by_index<Idx, typeList<T, Types...> > : type_by_index<Idx - 1, typeList<Types...> >
	{
	};

	//连接2个typelist
	template<typename T, typename U>
	struct typeList_concat;

	template<typename... Ts, typename... Us>
	struct typeList_concat<typeList<Ts...>, typeList<Us...>>
	{
		using type = typeList<Ts..., Us...>;
	};

	//2个types去重合并
	template<typename T, typename U>
	struct type_all_types;

	template<typename... Types>
	struct type_all_types<typeList<>, typeList<Types...> >
	{
		typedef typeList<Types...> Type;
	};

	//如 type_inter_types<typeList<long long , char, double>, typeList<int, float, bool> >::Type
	//是 typeList<double, char, long long, int, float, bool> 前三个是倒置的
	template <typename T, typename... Types, typename... Uypes>
	struct type_all_types<typeList<T, Types... >, typeList<Uypes...> >
	{
		using Type = std::conditional_t
			<
			!is_type_in_typeList<typename T, typename typeList<Uypes...> >::value,
			typename type_all_types
			<
			typeList<Types... >,
			typename typeList_concat
			<
			typeList<T>,
			typename typeList<Uypes... > >::type
			>::Type,
			typename type_all_types<typeList<Types... >, typeList<Uypes...> >::Type
			>;
	};

	//2个types求交
	template <typename T, typename U>
	struct type_intersection_types;

	template<typename... Uypes>
	struct type_intersection_types<typeList<>, typeList<Uypes...>>
	{
		using Type = typeList<>;
	};

	template <typename T, typename... Types, typename... Uypes>
	struct type_intersection_types<typeList<T, Types...>, typeList<Uypes...> >
	{
		using Type = std::conditional_t
			<
			is_type_in_typeList<T, typeList<Uypes...>>::value,
			typename typeList_concat
			<
			typeList<T>,
			typename type_intersection_types<typeList<Types...>, typeList<Uypes...>>::Type
			>::type,
			typename type_intersection_types<typeList<Types...>, typeList<Uypes...>>::Type >;
	};

	//从typelist中减除types 只要U中存在的就删掉
	template<typename T, typename U>
	struct types_sub_types;

	template<typename... Types>
	struct types_sub_types<typeList<>, typeList<Types...> >
	{
		typedef typeList<> Type;
	};

	//等价于 typeList<T, Types...> - typeList<Uypes...>
	template <typename T, typename... Types, typename... Uypes>
	struct types_sub_types<typeList<T, Types...>, typeList<Uypes...> >
	{
		using Type = std::conditional_t
			<
			is_type_in_typeList<T, typeList<Uypes...>>::value,
			typename types_sub_types<typeList<Types...>, typeList<Uypes...>>::Type,
			typename typeList_concat
			<
			typeList<T>,
			typename types_sub_types<typeList<Types...>, typeList<Uypes...>>::Type
			>::type
			>;
	};
}
