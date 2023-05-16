#ifndef PHUSIS_THREADLOCAL_HXX
#define PHUSIS_THREADLOCAL_HXX

#include "fw.hxx"

namespace sys
{
	class __threadlocal
	{
	private:
		tss_t _id;

	public:
		explicit __threadlocal() noexcept;
		~__threadlocal() noexcept;

	public:
		 void* get();
		 void set(void* val);
	};

	template<typename T>
	class threadlocal
	{
	private:
		__threadlocal _val;

	public:
		threadlocal();

	public:
		threadlocal(const sys::threadlocal<T>&) = delete;
		sys::threadlocal<T>& operator=(const sys::threadlocal<T>&) = delete;

	public:
		explicit operator T&() noexcept;
		sys::threadlocal<T>& operator=(const T& val) noexcept;
	};
}

template<typename T>
sys::threadlocal<T>::threadlocal() : _val()
{
}

template<typename T>
sys::threadlocal<T>::operator T&() noexcept
{
	return *static_cast<T*>(_val.get());
}

template<typename T>
sys::threadlocal<T>& sys::threadlocal<T>::operator=(const T& val) noexcept
{
	_val.set(&val);
	return *this;
}

#endif //PHUSIS_THREADLOCAL_HXX
