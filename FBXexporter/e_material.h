#pragma once
#include <cstdint>
#include <array>
namespace dev5
{
	using file_path_t = std::array<char, 260>; // Simple max size file path string
	// Simple material definition
	struct material_t
	{
		enum e_component { EMISSIVE = 0, DIFFUSE, SPECULAR, SHININESS, COUNT };
		struct component_t
		{
			float value[3] = { 0.0f, 0.0f, 0.0f };
			float factor = 0.0f;
			int64_t input = -1;
		};
		component_t& operator[](int i) { return components[i]; }
		const component_t& operator[](int i)const { return components[i]; }
	private:
		component_t components[COUNT];
	};
}