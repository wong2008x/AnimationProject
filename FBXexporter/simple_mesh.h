#pragma once
#include <fbxsdk.h>
#include <directxmath.h>
#include <array>

namespace end
{
	class alignas(8) float2 : std::array<float, 2> {};

	class alignas(16) float4 : std::array<float, 4> {};

	struct simple_vert
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT3 tangent;
		bool operator==(const simple_vert& n) const
		{
			if ((pos.x != n.pos.x) ||
				(pos.y != n.pos.y) ||
				(pos.z != n.pos.z) ||
				(tex.x != n.tex.x) ||
				(tex.y != n.tex.y) ||
				(norm.x != n.norm.x) ||
				(norm.y != n.norm.y) ||
				(norm.z != n.norm.z))
			{
				return false;
			}
			else
				return true;
		}
	};

	struct simple_mesh
	{
		uint32_t vert_count = 0;
		uint32_t index_count = 0;
		simple_vert* verts = nullptr;
		uint32_t* indices = nullptr;
	};
	struct skinned_vert
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT4 indices;
		DirectX::XMFLOAT4 weights;

		DirectX::XMFLOAT4 tangents;
		//DirectX::XMFLOAT4 binormals;

	};

	struct fbx_Joint
	{
		FbxNode* node;
		int parent_index;
	};

	struct joint 
	{ 
		DirectX::XMMATRIX global_xform;
		int parent_index; 
	};
	struct fnv1a
	{
	
		private:
			uint64_t fnv1a_hash(const uint8_t* bytes, size_t count) const
			{
				const uint64_t FNV_offset_basis = 0xcbf29ce484222325;
				const uint64_t FNV_prime = 0x100000001b3;

				uint64_t hash = FNV_offset_basis;

				for (size_t i = 0; i < count; ++i)
					hash = (hash ^ bytes[i]) * FNV_prime;

				return hash;
			}

		public: 
			template<typename T>
			uint64_t operator()(const T& t) const
		{
				return fnv1a_hash((const uint8_t*)&t, sizeof(T));
		}

	};
}
