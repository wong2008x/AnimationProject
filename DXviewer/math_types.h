#pragma once

#include <array>
#include <cstdint>
#include "pools.h"
#include <cmath>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <vector>

using namespace DirectX;
namespace end
{
	struct simpleVert
	{
		XMFLOAT3 pos;
		XMFLOAT3 norm;
		XMFLOAT2 uv;
		XMFLOAT3 tang;
	};
	struct joint
	{
		XMMATRIX global_xform;
		int parent_index;
	};
	struct lightCons
	{
		XMVECTOR dLightDir;
		XMVECTOR dLightClr;
	};
	struct keyframe
	{
		double time;
		std::vector<XMMATRIX> myjoints;
	};

	struct anim_clip
	{
		double duration;
		std::vector<keyframe> frames;
	};
	struct sphere_t { XMVECTOR center; float radius; }; //Alterative: using sphere_t = float4;

	struct aabb_t
	{
		XMVECTOR min; XMVECTOR max; XMFLOAT4 color;
	}; //Alternative: aabb_t { XMFLOAT3 center; XMFLOAT3 extend; };

	struct plane_t { XMVECTOR normal; float offset; };  //Alterative: using plane_t = float4;

	using frustum_t = std::array<plane_t, 6>;
	struct float2
	{
		float x;
		float y;

		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 2; }
	};

	struct float3
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};

			float2 xy;
		};


		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 3; }

		inline friend float3 operator+(float3 lhs, float3 rhs)
		{
			return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
		}

		inline friend float3 operator-(float3 lhs, float3 rhs)
		{
			return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
		}

		inline friend float3 operator*(float3 lhs, float3 rhs)
		{
			return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
		}
		inline friend float3 operator*(float3 lhs, float rhs)
		{
			return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
		}

		inline friend float3 operator/(float3 lhs, float3 rhs)
		{
			return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
		}
		inline friend float3 operator/(float3 lhs, float rhs)
		{
			return { lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
		}

		inline friend float dot(float3 lhs, float3 rhs)
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

		inline friend float3 cross(float3 lhs, float3 rhs)
		{
			return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x };
		}
		inline friend float3 normalize(float3 _value)
		{
			return (_value / (sqrtf(_value.x * _value.x + _value.y * _value.y + _value.z * _value.z)));
		};

		inline float3& operator+=(float3 rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}

		inline float3& operator-=(float3 rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}

		inline float3& operator*=(float3 rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;

			return *this;
		}

		inline float3& operator/=(float3 rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;

			return *this;
		}

		inline float3& operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;

			return *this;
		}

		inline float3& operator/=(float rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;

			return *this;
		}
	};

	struct float4
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};

			float3 xyz;

			struct
			{
				float2 xy;
				float2 zw;
			};
		};

		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 4; }
	};


	struct alignas(8) float2_a : float2 {};

	struct alignas(16) float3_a : float3 {};

	struct alignas(16) float4_a : float4 {};

	using float4x4 = std::array< float4, 4 >;
	using float4x4_a = std::array< float4_a, 4 >;
}

namespace end
{
	

	struct colored_vertex
	{
		XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
		XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		colored_vertex() = default;
		colored_vertex(const colored_vertex&) = default;

		inline colored_vertex(const XMFLOAT3& p, const XMFLOAT4& c) : pos{ p }, color{ c } {}
		inline colored_vertex(const XMFLOAT3& p, const XMFLOAT3& c) : pos{ p }, color{ c.x, c.y, c.z, 1.0f } {}
		inline colored_vertex(const XMFLOAT3& p, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : pos{ p }, color{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f } {}
	};
	struct particle
	{
		XMFLOAT3 pos;
		XMFLOAT3 prev_pos;
		XMFLOAT4 color;
		XMFLOAT4 velocity;
		XMFLOAT4 acceleration;
		float lifetime;
	};
	

	struct emitter
	{
		XMFLOAT3 spawn_pos;
		XMFLOAT4 spawn_color;
		// indices into the shared_pool
		sorted_pool_t<int16_t, 256> indices;
	}; 
}