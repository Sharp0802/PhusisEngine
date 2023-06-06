
#include "fw.hxx"
#include <cstring>
#include <cxxabi.h>

/// @brief Get mangled name of unmanaged function (for managed assemblies)
/// @param ns name of namespace
/// @param cls name of class
/// @param fn name of function
/// @param dst destination memory of mangled function name
extern "C" DLLEXPORT bool __getfn(
		const char* __restrict ns,
		const char* __restrict cls,
		const char* __restrict fn,
		char* __restrict dst,
		int32_t n);

bool __getfn(const char* ns, const char* cls, const char* fn, char* dst, int32_t n)
{
	size_t l = (ns ? strlen(ns) + 1 : 0) +
			   (cls ? strlen(cls) + 1 : 0) +
			   strlen(fn);

	char* src = static_cast<char*>(calloc(l, sizeof(char)));
	char* buf = static_cast<char*>(calloc(512, sizeof(char)));

	int status;

	if (ns)
	{
		strcat(src, ns);
		strcat(src, "::");
	}
	if (cls)
	{
		strcat(src, cls);
		strcat(src, "::");
	}
	strcat(src, fn);

	buf = abi::__cxa_demangle(src, buf, nullptr, &status);
	if (static_cast<int32_t>(strlen(buf)) > n)
		return false;

	strcpy(dst, buf);

	free(buf);

	return true;
}
