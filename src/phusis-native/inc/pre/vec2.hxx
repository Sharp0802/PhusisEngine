#ifndef PHUSIS_VEC2_HXX
#define PHUSIS_VEC2_HXX

#include "fw.hxx"
#include "type.hxx"

namespace pre
{
	template<typename T>
	struct vec2
	{
	public:
		T x;
		T y;

	public:
		vec2() noexcept;
		explicit vec2(T x, T y) noexcept;
		vec2(const vec2<T>& other) noexcept;

	public:
		vec2<T> operator+(vec2<T> v) noexcept;
		vec2<T> operator-(vec2<T> v) noexcept;
		vec2<T> operator*(T v) noexcept;
		vec2<T> operator/(T v) noexcept;

		vec2<T>& operator=(const vec2<T>& v) noexcept;
	};

	using vec2i = vec2<int32_t>;
	using vec2u = vec2<uint32_t>;
}

template<typename T>
pre::vec2<T>::vec2() noexcept
	: x(pre::type<T>::__default), y(pre::type<T>::__default)
{

}

template<typename T>
pre::vec2<T>::vec2(T x, T y) noexcept
	: x(x), y(y)
{
}

template<typename T>
pre::vec2<T>::vec2(const vec2<T>& other) noexcept
	: x(other.x), y(other.y)
{
}

template<typename T>
pre::vec2<T> pre::vec2<T>::operator+(vec2<T> v) noexcept
{
	return pre::vec2<T>(x + v.x, y + v.y);
}

template<typename T>
pre::vec2<T> pre::vec2<T>::operator-(vec2<T> v) noexcept
{
	return pre::vec2<T>(x - v.x, y - v.y);
}

template<typename T>
pre::vec2<T> pre::vec2<T>::operator*(T v) noexcept
{
	return pre::vec2<T>(x * v, y * v);
}

template<typename T>
pre::vec2<T> pre::vec2<T>::operator/(T v) noexcept
{
	return pre::vec2<T>(x / v, y / v);
}

template<typename T>
pre::vec2<T>& pre::vec2<T>::operator=(const vec2<T>& v) noexcept
{
	this->x = v.x;
	this->y = v.y;
	return *this;
}

#endif //PHUSIS_VEC2_HXX
