#pragma once

#include <cstdint>
#include <vector>
#include "math_types.h"
#include "pch.h"

namespace end
{
	using binary_blob_t = std::vector<uint8_t>;


	binary_blob_t load_binary_blob(const char* path);

	void load_fbx_model(const char* path, std::vector<simpleVert>& vert, std::vector<uint32_t>& indicies);

	void load_animation(const char* path, anim_clip& myAnim);

	void load_pose(const char* path, std::vector<joint>& poseJoint, std::vector<joint>& invPoseJoints);

	void load_fbx_model_skinned(const char* path, std::vector<skinnedVert>& vert, std::vector<uint32_t>& indicies);

}