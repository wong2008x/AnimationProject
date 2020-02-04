#pragma once
#include <directxmath.h>
#include <array>

namespace end
{
	class alignas(8) float2 : std::array<float, 2> {};

	class alignas(16) float4 : std::array<float, 4> {};

	struct simple_vert
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT3 norm;
		DirectX::XMFLOAT2 tex;
		DirectX::XMFLOAT4 tangent;
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
	struct Hash
	{
		size_t operator()(const skinned_vert v) const
		{
			size_t pos = std::hash<float>()(v.pos.x) ^ std::hash<float>()(v.pos.y) ^ std::hash<float>()(v.pos.z) ^ std::hash<float>()(v.pos.w);
			size_t norm = std::hash<float>()(v.norm.x) ^ std::hash<float>()(v.norm.y) ^ std::hash<float>()(v.norm.z);
			size_t tex = std::hash<float>()(v.tex.x) ^ std::hash<float>()(v.tex.y);
			return (pos ^ norm ^ tex);
		}

		size_t operator()(const simple_vert v) const
		{
			size_t pos = std::hash<float>()(v.pos.x) ^ std::hash<float>()(v.pos.y) ^ std::hash<float>()(v.pos.z) ^ std::hash<float>()(v.pos.w);
			size_t norm = std::hash<float>()(v.norm.x) ^ std::hash<float>()(v.norm.y) ^ std::hash<float>()(v.norm.z);
			size_t tex = std::hash<float>()(v.tex.x) ^ std::hash<float>()(v.tex.y);
			return (pos ^ norm ^ tex);
		}
	};
}
