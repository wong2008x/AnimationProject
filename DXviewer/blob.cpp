
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

	void load_fbx_model(const char* path, std::vector<simpleVert>& vert, std::vector<uint32_t>& indicies)
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

		load.close();

	}

	void load_fbx_model_skinned(const char* path, std::vector<skinnedVert>& vert, std::vector<uint32_t>& indicies)
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
		load.read((char*)&vertSize, sizeof(uint32_t));
		vert.resize(vertSize);
		load.read((char*)vert.data(), sizeof(skinnedVert) * vertSize);
		load.read((char*)&indexSize, sizeof(uint32_t));
		indicies.resize(indexSize);
		load.read((char*)indicies.data(), sizeof(uint32_t) * indexSize);

		load.close();
	}
	
	
	void load_animation(const char* path, anim_clip& myAnim)
	{
		std::fstream load{ path, std::ios_base::in | std::ios_base::binary };

		assert(load.is_open());

		if (!load.is_open())
		{
			assert(false);
			return;
		}
		uint32_t size;
		load.read((char*)&myAnim.duration, sizeof(double));
		load.read((char*)&size, sizeof(uint32_t));
		myAnim.frames.resize(size);
		for (uint32_t i = 0; i < size; i++)
		{
			uint32_t jointSize;
			load.read((char*)&jointSize, sizeof(uint32_t));
			myAnim.frames[i].joints.resize(jointSize);
			load.read((char*)myAnim.frames[i].joints.data(), sizeof(joint) * myAnim.frames[i].joints.size());
			for (size_t j = 0; j < myAnim.frames[i].joints.size(); j++)
			{
				//myAnim.frames[i].joints[j] .global_xform = myAnim.frames[i].joints[j].global_xform * XMMatrixTranslation(5, 0, 0) * XMMatrixRotationY(XMConvertToRadians(180));
				//myAnim.frames[i].joints[j].global_xform = XMMatrixInverse(nullptr, myAnim.frames[i].joints[j].global_xform);
			}
			load.read((char*)&myAnim.frames[i].time, sizeof(double));
		}
		load.close();
	}

	void load_pose(const char* path, std::vector<joint>& poseJoint, std::vector<joint>& invPoseJoints)
	{
		std::fstream load{ path, std::ios_base::in | std::ios_base::binary };

		assert(load.is_open());

		if (!load.is_open())
		{
			assert(false);
			return;
		}
		uint32_t size;
		load.read((char*)&size, sizeof(uint32_t));
		poseJoint.resize(size);
		invPoseJoints.resize(size);
		load.read((char*)poseJoint.data(), sizeof(joint) * poseJoint.size());

		for (size_t i = 0; i < poseJoint.size(); i++)
		{
			invPoseJoints[i].global_xform = XMMatrixInverse(nullptr, poseJoint[i].global_xform);
			invPoseJoints[i].parent_index = poseJoint[i].parent_index;

			//poseJoint[i].global_xform = poseJoint[i].global_xform * XMMatrixTranslation(-5, 0, 0) * XMMatrixRotationY(XMConvertToRadians(180));
		}


		load.close();
	}


	//void load_fbx_model(const char* path, std::vector<simpleVert>& vert, std::vector<uint32_t>& indicies)
	//{

	//	std::fstream load{ path, std::ios_base::in | std::ios_base::binary };

	//	assert(load.is_open());

	//	if (!load.is_open())
	//	{
	//		assert(false);
	//		return;
	//	}

	//	uint32_t vertSize;
	//	uint32_t indexSize;
	//	load.read((char*)&vertSize, sizeof(uint32_t));
	//	vert.resize(vertSize);
	//	load.read((char*)vert.data(), sizeof(simpleVert) * vertSize);
	//	load.read((char*)&indexSize, sizeof(uint32_t));
	//	indicies.resize(indexSize);
	//	load.read((char*)indicies.data(), sizeof(uint32_t) * indexSize);



	//	load.close();

	//}
}
