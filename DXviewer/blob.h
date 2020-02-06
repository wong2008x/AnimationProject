#pragma once

#include <cstdint>
#include <vector>
#include "math_types.h"
#include "pch.h"

namespace end
{
	using binary_blob_t = std::vector<uint8_t>;

	binary_blob_t load_binary_blob(const char* path);

	void load_fbx_model(ID3D11Device* mdevice,ID3D11Buffer* vertBuffer,const char* path, std::vector<simpleVert>& vert, std::vector<uint32_t>& indicies);
}