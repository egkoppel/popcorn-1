#ifndef _STLP_INTERNAL_UTILITY_H
#define _STLP_INTERNAL_UTILITY_H

#include <stl/config/features.h>

#include <type_traits>

_STLP_BEGIN_NAMESPACE

template<class T> typename tr1::remove_reference<T>::type&& move(T&& t) {
	return static_cast<typename tr1::remove_reference<T>::type&&>(t);
}

template<class T> T&& forward(typename tr1::remove_reference<T>::type& t) {
	return static_cast<T&&>(t);
}

template<class T> T&& forward(typename tr1::remove_reference<T>::type&& t) {
	return static_cast<T&&>(t);
}

_STLP_END_NAMESPACE

#endif