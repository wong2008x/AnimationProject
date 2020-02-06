
#include "blob.h"
#include <fstream>

namespace end
{
	binary_blob_t load_binary_blob(const char* path)
	{
		binary_blob_t blob;

		std::fstream file{ path, std::ios_base::in | std::ios_base::binary };

		if (file.is_open())
		{
			file.seekg(0, std::ios_base::end);
			blob.resize(file.tellg());
			file.seekg(0, std::ios_base::beg);

			file.read((char*)blob.data(), blob.size());

			file.close();
		}

		return std::move(blob);
	}

	void load_fbx_model(ID3D11Device* mdevice, ID3D11Buffer* vertBuffer, const char* path, std::vector<simpleVert>& vert, std::vector<uint32_t>& indicies)
	{
		
		std::fstream load{ path, std::ios_base::in | std::ios_base::binary };
		
		assert(load.is_open());
		
		if (!load.is_open())
		{
			assert(false);
			return;
		}
		uint32_t vertSize;
		uint32_t indexSize;
		load.read((char*)&vertSize,sizeof(uint32_t));
		vert.resize(vertSize);
		load.read((char*)vert.data(), sizeof(simpleVert) * vertSize);
		load.read((char*)&indexSize, sizeof(uint32_t));
		indicies.resize(indexSize);
		load.read((char*)indicies.data(), sizeof(uint32_t) * indexSize);

		D3D11_BUFFER_DESC BufferDesc;
		ZeroMemory(&BufferDesc, sizeof(BufferDesc));
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.ByteWidth = sizeof(simpleVert) * vert.size();
		BufferDesc.CPUAccessFlags = NULL;
		BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA VerticesData;
		ZeroMemory(&VerticesData, sizeof(VerticesData));
		VerticesData.pSysMem = vert.data();
		mdevice->CreateBuffer(&BufferDesc, &VerticesData, &vertBuffer);


	}
}