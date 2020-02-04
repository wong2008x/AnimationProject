#pragma once

#include <cstdint>

namespace end
{
	// 'detail' is a name often give to namespaces that
	//	wrap up the details of an implementation.
	//  Any 'detail' namespace should be treated as private
	//	and not accessed directly.
	namespace detail
	{
		uint64_t fnv1a_hash(const uint8_t* bytes, size_t count)
		{
			const uint64_t FNV_offset_basis = 0xcbf29ce484222325;
			const uint64_t FNV_prime = 0x100000001b3;

			uint64_t hash = FNV_offset_basis;

			for (size_t i = 0; i < count; ++i)
				hash = (hash ^ bytes[i]) * FNV_prime;

			return hash;
		}
	}


	// Calculate the fnv1a hash for some object of type T
	// WARNING: This hashing will include padding
	template<typename T>
	uint64_t fnv1a(const T& t)
	{
		return detail::fnv1a_hash((const uint8_t*)&t, sizeof(T));
	}

	// Calculate the fnv1a hash for an array of constant characters
	template<size_t N>
	uint64_t fnv1a(const char(&arr)[N])
	{
		return detail::fnv1a_hash((const uint8_t*)arr, N - 1);
	}
}