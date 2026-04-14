/*****************************************************************************
* @author  : windsPx                                                         *
* @date    :                                                                 *
* @file    :                                                                 *
* @brief   :                                                                 *
*----------------------------------------------------------------------------*
* Date        | Version   | Author         | Description                     *
*----------------------------------------------------------------------------*
*             |           |  windsPx              |                          *
*****************************************************************************/
#pragma once

#include <utility>
#include <tuple>

//基于std::tuple实现的简单AOP
//定义切面对象时 可以不要求存在pre和next两个函数
namespace Aop
{
	template<typename T, typename... Args>
	struct has_member_preDispose
	{
		template<typename AP>
		static auto check(int) -> decltype(std::declval<AP>().preDispose(std::declval<Args>()...), std::true_type());
		template<typename AP>
		static std::false_type check(...);
		enum { value = std::is_same<decltype(check<T>(0)), std::true_type>::value };
	};

	template<typename T, typename... Args>
	struct has_member_nextDispose
	{
		template<typename AP>
		static auto check(int) -> decltype(std::declval<AP>().nextDispose(std::declval<Args>()...), std::true_type());
		template<typename AP>
		static std::false_type check(...);
		enum { value = std::is_same<decltype(check<T>(0)), std::true_type>::value };
	};

	//前置切面 ↓
	//存在前置切面定义
	template <typename AP, typename... Args>
	typename std::enable_if<has_member_preDispose<AP, Args...>::value>::type _pre_dispose(AP&& asp, Args&&... params)
	{
		asp.preDispose(std::forward<Args>(params)...);
	}

	//不存在前置切面定义
	template <typename AP, typename... Args>
	typename std::enable_if<!has_member_preDispose<AP, Args...>::value>::type _pre_dispose(AP&& asp, Args&&... params)
	{
	}

	template <int index, typename APS, typename... Args>
	struct CPreDispose
	{
		static void preDispose(APS&& aspects, Args&&... params)
		{
			CPreDispose<index - 1, APS, Args... >::preDispose(std::forward<APS>(aspects), std::forward<Args>(params)...);
			_pre_dispose(std::get<index - 1>(std::forward<APS>(aspects)), std::forward<Args>(params)...);
		}
	};

	template <typename APS, typename... Args>
	struct CPreDispose<1, APS, Args...>
	{
		static void preDispose(APS&& aspects, Args&&... params)
		{
			_pre_dispose(std::get<0>(std::forward<APS>(aspects)), std::forward<Args>(params)...);
		}
	};
	//前置切面 ↑
	///////////////////////////////////////////////////////////////////
	//后置切面 ↓
	//存在后置切面定义
	template <typename AP, typename... Args>
	typename std::enable_if<has_member_nextDispose<AP, Args...>::value>::type _next_dispose(AP&& asp, Args&&... params)
	{
		asp.nextDispose(std::forward<Args>(params)...);
	}

	//不存在后置切面定义
	template <typename AP, typename... Args>
	typename std::enable_if<!has_member_nextDispose<AP, Args...>::value>::type _next_dispose(AP&& asp, Args&&... params)
	{
	}

	template <int index, typename APS, typename... Args>
	struct CNextDispose
	{
		static void nextDispose(APS&& aspects, Args&&... params)
		{
			CNextDispose<index - 1, APS, Args... >::nextDispose(std::forward<APS>(aspects), std::forward<Args>(params)...);
			_next_dispose(std::get<index - 1>(std::forward<APS>(aspects)), std::forward<Args>(params)...);
		}
	};

	template <typename APS, typename... Args>
	struct CNextDispose<1, APS, Args...>
	{
		static void nextDispose(APS&& aspects, Args&&... params)
		{
			_next_dispose(std::get<0>(std::forward<APS>(aspects)), std::forward<Args>(params)...);
		}
	};
	//后置切面 ↑
	//////////////////////////////////////////////////////////////////////////
	//执行
	template <int index, typename APS, typename... Args>
	void pre_dispose(APS&& aspects, Args&&... params)
	{
		CPreDispose<index, APS, Args... >::preDispose(std::forward<APS>(aspects), std::forward<Args>(params)...);
	}

	template <int index, typename APS, typename... Args>
	void next_dispose(APS&& aspects, Args&&... params)
	{
		CNextDispose<index, APS, Args... >::nextDispose(std::forward<APS>(aspects), std::forward<Args>(params)...);
	}

	//////////////////////////////////////////////////////
	//AP定义的class需要具有想用的函数参数
	template <typename Fun, typename... AP>
	class AspectOP
	{
		//! 被切的执行体
		Fun m_fun;
		//! 切面执行体
		using ASPECTS = std::tuple<AP...>;
		ASPECTS m_aps;
	public:
		AspectOP(Fun&& ofun) : m_fun(std::forward<Fun>(ofun))
		{
			m_aps = std::make_tuple(AP()...);
		}
	public:
		template <typename... Args>
		void dispose(Args&&... params)
		{
			pre_dispose<sizeof...(AP)>(std::forward<ASPECTS>(m_aps), std::forward<Args>(params)...);
			m_fun(std::forward<Args>(params)...);
			next_dispose<std::tuple_size<ASPECTS>::value>(std::forward<ASPECTS>(m_aps), std::forward<Args>(params)...);
		}
	};
	//无切面
	template <typename Fun>
	class AspectOP<Fun>
	{
		//! 被切的执行体
		Fun m_fun;
	public:
		AspectOP(Fun&& ofun) : m_fun(std::forward<Fun>(ofun)) { }
	public:
		template <typename... Args>
		void dispose(Args&&... params)
		{
			m_fun(std::forward<Args>(params)...);
		}
	};

	class Dispose
	{
	public:
		template <typename... AP, typename Fun, typename... Args>
		static void run(Fun&& ofun, Args&&... params)
		{
			AspectOP<Fun, AP...> _aspect(std::forward<Fun>(ofun));
			_aspect.dispose(std::forward<Args>(params)...);
		}
	};
}