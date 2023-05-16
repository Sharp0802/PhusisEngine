
#include "sys/threadlocal.hxx"

static void __dtor(void*)
{
}

sys::__threadlocal::__threadlocal() noexcept
{
	_id = 0;
	tss_create(&_id, __dtor);
}

sys::__threadlocal::~__threadlocal() noexcept
{
	tss_delete(_id);
}

void* sys::__threadlocal::get()
{
	return tss_get(_id);
}

void sys::__threadlocal::set(void* val)
{
	tss_set(_id, val);
}
