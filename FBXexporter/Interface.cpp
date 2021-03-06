#pragma once
#include "pch.h" 
#include "InterFace.h"
#include "simple_mesh.h"
#include "e_material.h"

#include <fbxsdk.h>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string>

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif


using namespace DirectX;
FbxManager* create_and_import(const char* fbx_file_path, FbxScene*& lScene)
{
	// Initialize the SDK manager. This object handles all our memory management.
	FbxManager* lSdkManager = FbxManager::Create();
	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);
	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(fbx_file_path, -1, lSdkManager->GetIOSettings()))
	{
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf(std::string(std::string(fbx_file_path) + std::string("\n")).c_str());
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		return nullptr;
	}
	// Create a new scene so that it can be populated by the imported file.
	lScene = FbxScene::Create(lSdkManager, "imported_scene");
	// Import the contents of the file into the scene.
	lImporter->Import(lScene);
	lImporter->Destroy();
	return lSdkManager;
}

FBXEXPORTER_API int get_scene_poly_count(const char* fbx_file_path)
{
	int result = -1;
	// Scene pointer, set by call to create_and_import
	FbxScene* scene = nullptr;
	// Create the FbxManager and import the scene from file
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);
	// Check if manager creation failed
	if (sdk_manager == nullptr)
		return result;
	//If the scene was imported...
	if (scene != nullptr)
	{
		//No errors to report, so start polygon count at 0
		result = 0;
		// Get the count of geometry objects in the scene
		int geo_count = scene->GetGeometryCount();
		for (int i = 0; i < geo_count; ++i)
		{
			//Get geometry number 'i'
			FbxGeometry* geo = scene->GetGeometry(i);
			// If it's not a mesh, skip it
			// Geometries might be some other type like nurbs
			if (geo->GetAttributeType() != FbxNodeAttribute::eMesh)
				continue;
			// Found a mesh, add its polygon count to the result
			FbxMesh* mesh = (FbxMesh*)geo;
			result += mesh->GetPolygonCount();
		}
	}
	//Destroy the manager
	sdk_manager->Destroy();
	//Return the polygon count for the scene
	return result;
}

FBXEXPORTER_API int export_simple_mesh(const char* fbx_file_path, const char* output_file_path, const char* mesh_name)
{

	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	if (!scene)
		return -1;

	FbxNode* childNode = nullptr;
	int childCount = scene->GetRootNode()->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		childNode = scene->GetRootNode()->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();
		if (mesh)
		{
				mesh_name = mesh->GetName();
				//std::vector<end::simple_vert> simpleMesh;
				//int poly_vert ;
				//int point_index;
								//int poly_count = mesh->GetPolygonCount();
				//int* polygon_verts = mesh->GetPolygonVertices();
				//const FbxVector4* control_points = mesh->GetControlPoints();
				//// getting normals from fbx
				//FbxArray<FbxVector4> normalsVec;
				//mesh->GetPolygonVertexNormals(normalsVec);
				//FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = NULL;
				//mesh->GetTextureUV(&uvVertices);
				//FbxLayerElementArrayTemplate<FbxVector4>* fbxTangents;
				//mesh->GetTangents(&fbxTangents);
				//for (size_t j = 0; j < poly_count; j++)
				//{

				//	for (size_t v = 0; v < mesh->GetPolygonSize(j); v++)
				//	{
				//		end::simple_vert tempVert;
				//		poly_vert = j * 3 + v;
				//		point_index = polygon_verts[poly_vert];
				//		FbxVector4 position = control_points[point_index];
				//		tempVert.pos = DirectX::XMFLOAT3{
				//			(float)position.mData[0],
				//			(float)position.mData[1],
				//			(float)position.mData[2] };

				//		FbxVector2 uv = uvVertices->GetAt(mesh->GetTextureUVIndex(j, v));
				//		FbxVector4 tangent = fbxTangents->GetAt(point_index);

				//		tempVert.tex = DirectX::XMFLOAT2((float)uv.mData[0], 1 - (float)uv.mData[1]);
				//		tempVert.norm = DirectX::XMFLOAT3{ (float)normalsVec[point_index].mData[0], (float)normalsVec[point_index].mData[1], (float)normalsVec[point_index].mData[2] };
				//		tempVert.tangent = DirectX::XMFLOAT3{ (float)tangent.mData[0], (float)tangent.mData[1], (float)tangent.mData[2] };
				//		simpleMesh.push_back(tempVert);
				//	}
				//  
				//}


				end::simple_mesh simpleMesh;
					// getting vertices and count from fbx
				simpleMesh.vert_count = mesh->GetControlPointsCount();
				simpleMesh.verts = new end::simple_vert[simpleMesh.vert_count];
				for (uint32_t j = 0; j < simpleMesh.vert_count; j++)
				{
					simpleMesh.verts[j].pos = DirectX::XMFLOAT3{ (float)mesh->GetControlPointAt(j).mData[0], (float)mesh->GetControlPointAt(j).mData[1], (float)mesh->GetControlPointAt(j).mData[2]};
				}
				// getting indices array and count from fbx
				simpleMesh.index_count = mesh->GetPolygonVertexCount();
				simpleMesh.indices = (uint32_t*)mesh->GetPolygonVertices();

				// getting uv's from fbx
				DirectX::XMFLOAT2* UV = new DirectX::XMFLOAT2[simpleMesh.index_count];
				for (int j = 0; j < mesh->GetPolygonCount(); j++)//polygon(=mostly rectangle) count
				{
					FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = NULL;
					mesh->GetTextureUV(&uvVertices);
					for (int k = 0; k < mesh->GetPolygonSize(j); k++)//retrieves number of vertices in a polygon
					{
						FbxVector2 uv = uvVertices->GetAt(mesh->GetTextureUVIndex(j, k));
						UV[3 * j + k] = DirectX::XMFLOAT2((float)uv.mData[0], 1 - (float)uv.mData[1]);
					}
				}

				FbxLayerElementArrayTemplate<FbxVector4>* fbxTangents;
				mesh->GetTangents(&fbxTangents);


				// getting normals from fbx
				FbxArray<FbxVector4> normalsVec;
				mesh->GetPolygonVertexNormals(normalsVec);

				end::simple_vert* verts2 = new end::simple_vert[simpleMesh.index_count];
				simpleMesh.vert_count = simpleMesh.index_count;
				for (uint32_t j = 0; j < simpleMesh.index_count; j++)
				{
					verts2[j] = simpleMesh.verts[simpleMesh.indices[j]];
					verts2[j].norm = DirectX::XMFLOAT3{ (float)normalsVec[j].mData[0], (float)normalsVec[j].mData[1], (float)normalsVec[j].mData[2] };
					verts2[j].tex = UV[j];


					verts2[j].tangent = {
						(float)fbxTangents->GetAt(j).mData[0],
						(float)fbxTangents->GetAt(j).mData[1],
						(float)fbxTangents->GetAt(j).mData[2],};
				};

				delete[] simpleMesh.verts;
				simpleMesh.verts = verts2;

				unsigned int numIndices = 0;
				std::unordered_map<end::simple_vert, unsigned int, end::fnv1a> uniqueValues;
				std::vector<end::simple_vert> vertices;
				std::vector<unsigned int> indicesVector;

				for (uint32_t j = 0; j < simpleMesh.index_count; j++)
				{
					end::simple_vert v;
					v.norm = simpleMesh.verts[j].norm;
					v.pos = simpleMesh.verts[j].pos;
					v.tex = simpleMesh.verts[j].tex;

					if (uniqueValues.count(v) == 0)
					{
						uniqueValues.insert({ v, numIndices });
						vertices.push_back(v);
						indicesVector.push_back(numIndices++);
					}
					else
						indicesVector.push_back(uniqueValues[v]);
				}


				std::ofstream file(output_file_path, std::ios_base::binary | std::ios_base::trunc);
				if (file.is_open())
				{
					uint32_t size = static_cast<uint32_t>(vertices.size());
					file.write((const char*)&size, sizeof(uint32_t));
					file.write((const char*)vertices.data(), sizeof(end::simple_vert) * size);
					size = static_cast<uint32_t>(indicesVector.size());
					file.write((const char*)&size, sizeof(uint32_t));
					file.write((const char*)indicesVector.data(), sizeof(uint32_t) * size);
				}
				file.close();
				//delete []verts2;
				//delete []UV;
				return 0;
			
		}
	}
	return -1;
}

FBXEXPORTER_API int export_material(const char* fbx_file_path, const char* output_file_path, const char* mesh_name)
{
	std::vector<dev5::material_t> materials;
	std::vector<dev5::file_path_t> paths;
	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	int num_mats = scene->GetMaterialCount();
	for (int m = 0; m < num_mats; ++m)
	{
			dev5::material_t my_mat;
			FbxSurfaceMaterial* mat = scene->GetMaterial(m);
			if (mat->Is<FbxSurfaceLambert>() == false) // Non-standard material, skip for now
				continue;
			FbxSurfaceLambert* lam = (FbxSurfaceLambert*)mat;

			

		// Get emissive property as above
			FbxDouble3 emissive_color = lam->Emissive.Get();
			FbxDouble emissive_factor = lam->EmissiveFactor.Get();
			my_mat[dev5::material_t::EMISSIVE].value[0] = (float)emissive_color.mData[0];
			my_mat[dev5::material_t::EMISSIVE].value[1] = (float)emissive_color.mData[1];
			my_mat[dev5::material_t::EMISSIVE].value[2] = (float)emissive_color.mData[2];
			my_mat[dev5::material_t::EMISSIVE].factor = emissive_factor;
			if (FbxFileTexture* file_texture = lam->Emissive.GetSrcObject<FbxFileTexture>())
			{
				const char* file_name = file_texture->GetRelativeFileName();
				dev5::file_path_t file_path;
				strcpy(file_path.data(), file_name);
				my_mat[dev5::material_t::EMISSIVE].input = paths.size();
				paths.push_back(file_path);
			}

			FbxDouble3 diffuse_color = lam->Diffuse.Get();
			FbxDouble diffuse_factor = lam->DiffuseFactor.Get();
			my_mat[dev5::material_t::DIFFUSE].value[0] = (float)diffuse_color.mData[0];
			my_mat[dev5::material_t::DIFFUSE].value[1] = (float)diffuse_color.mData[1];
			my_mat[dev5::material_t::DIFFUSE].value[2] = (float)diffuse_color.mData[2];
			my_mat[dev5::material_t::DIFFUSE].factor = diffuse_factor;
			if (FbxFileTexture* file_texture = lam->Diffuse.GetSrcObject<FbxFileTexture>())
			{
				const char* file_name = file_texture->GetRelativeFileName();
				dev5::file_path_t file_path;
				strcpy(file_path.data(), file_name);
				my_mat[dev5::material_t::DIFFUSE].input = paths.size();
				paths.push_back(file_path);
			}
		// ...
		if (mat->Is<FbxSurfacePhong>())
		{
			// Get specular related properties...
			FbxSurfacePhong* phong = (FbxSurfacePhong*)mat;
			FbxDouble3 specular_color = phong->Specular.Get();
			FbxDouble specular_factor = phong->SpecularFactor.Get();
			my_mat[dev5::material_t::SPECULAR].value[0] = (float)specular_color.mData[0];
			my_mat[dev5::material_t::SPECULAR].value[1] = (float)specular_color.mData[1];
			my_mat[dev5::material_t::SPECULAR].value[2] = (float)specular_color.mData[2];
			my_mat[dev5::material_t::SPECULAR].factor = specular_factor;
			if (FbxFileTexture* file_texture = phong->Specular.GetSrcObject<FbxFileTexture>())
			{
				const char* file_name = file_texture->GetRelativeFileName();
				dev5::file_path_t file_path;
				strcpy(file_path.data(), file_name);
				my_mat[dev5::material_t::SPECULAR].input = paths.size();
				paths.push_back(file_path);
			}
		}
		// add my_mat materials vector...
		materials.push_back(my_mat);;
	}

	std::ofstream file(output_file_path, std::ios_base::binary | std::ios_base::trunc);
	if (file.is_open())
	{
		uint32_t size = static_cast<uint32_t>(materials.size());
		file.write((const char*)&size, sizeof(uint32_t));
		file.write((const char*)materials.data(), sizeof(dev5::material_t) * size);
		size = static_cast<uint32_t>(paths.size());
		file.write((const char*)&size, sizeof(uint32_t));
		file.write((const char*)paths.data(), sizeof(dev5::file_path_t) * size);
	}
	file.close();
	return num_mats;
 }

FBXEXPORTER_API int export_bindpose(const char* fbx_file_path, const char* output_file_path, const char* mesh_name)
{

	int result = -1;
	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	if (!scene)
		return result;
	FbxNode* childNode = nullptr;
	int childCount = scene->GetRootNode()->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		childNode = scene->GetRootNode()->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();
		if (mesh)
		{
			if (!mesh_name || mesh_name == mesh->GetName())
			{
				int posecount = scene->GetPoseCount();
				FbxPose* pose = scene->GetPose(0);
				FbxMesh* poseMesh = pose->GetNode(0)->GetMesh();
				std::vector<end::fbx_Joint> joints;
				if (pose->IsBindPose())
				{
					posecount = pose->GetCount();
					for (int i = 0; i < posecount; i++)
					{
						FbxSkeleton* skele = pose->GetNode(i)->GetSkeleton();
						if (skele && skele->IsSkeletonRoot())
						{
							end::fbx_Joint joint;
							joint.node = pose->GetNode(i);
							joint.parent_index = -1;
							joints.push_back(joint);
							for (int j = 0; j < joints.size(); j++)
							{
								for (int k = 0; k < joints[j].node->GetChildCount(); k++)
								{
									joint.node = joints[j].node->GetChild(k);
									joint.parent_index = j;
									joints.push_back(joint);
								}
							}
							break;
						}
					}
				}

				std::vector<end::joint> myJoints;
				for (size_t i = 0; i < joints.size(); i++)
				{
					end::joint tempjoint;
					FbxAMatrix mat = joints[i].node->EvaluateGlobalTransform();
					XMMATRIX xmat;
					for (size_t j = 0; j < 4; j++)
						xmat.r[j] = { (float)mat.mData[j].mData[0], (float)mat.mData[j].mData[1], (float)mat.mData[j].mData[2], (float)mat.mData[j].mData[3] };
					//xmat = XMMatrixInverse(nullptr, xmat);
					tempjoint.global_xform = xmat;
					tempjoint.parent_index = joints[i].parent_index;
					myJoints.push_back(tempjoint);
				}

				std::ofstream file(output_file_path, std::ios_base::binary | std::ios_base::trunc);
				if (file.is_open())
				{
					uint32_t size = myJoints.size();
					file.write((const char*)&size, sizeof(uint32_t));
					file.write((const char*)myJoints.data(), sizeof(end::joint) * myJoints.size());
				}
				file.close();
				return 0;
			}
		}
	}
	return result;
}
extern "C" FBXEXPORTER_API int export_animation(const char* fbx_file_path, const char* output_file_path, const char* mesh_name)
{
	int result = -1;
	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	if (!scene)
		return result;
	FbxNode* childNode = nullptr;
	int childCount = scene->GetRootNode()->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		childNode = scene->GetRootNode()->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();
		if (mesh)
		{
			if (!mesh_name || mesh_name == mesh->GetName())
			{
				int posecount = scene->GetPoseCount();
				FbxPose* pose = scene->GetPose(0);
				std::vector<end::fbx_Joint> joints;
				if (pose->IsBindPose())
				{
					posecount = pose->GetCount();
					for (int i = 0; i < posecount; i++)
					{
						FbxSkeleton* skele = pose->GetNode(i)->GetSkeleton();
						if (skele && skele->IsSkeletonRoot())
						{
							end::fbx_Joint joint;
							joint.node = pose->GetNode(i);
							joint.parent_index = -1;
							joints.push_back(joint);
							for (int j = 0; j < joints.size(); j++)
							{
								for (int k = 0; k < joints[j].node->GetChildCount(); k++)
								{
									joint.node = joints[j].node->GetChild(k);
									joint.parent_index = j;
									joints.push_back(joint);
								}
							}
							break;
						}
					}
				}

				FbxAnimStack* animstack = scene->GetCurrentAnimationStack();
				FbxTimeSpan lifespan = animstack->GetLocalTimeSpan();
				FbxTime animTime = lifespan.GetDuration();
				end::anim_clip animClip;
				animClip.duration = animTime.GetSecondDouble();
				FbxLongLong frames = animTime.GetFrameCount(FbxTime::EMode::eFrames24);
				for (FbxLongLong i = 1; i < frames; i++)
				{
					animTime.SetFrame(i, FbxTime::EMode::eFrames24);
					end::keyframe mykeyframe;
					mykeyframe.time = animTime.GetSecondDouble();
					for (size_t j = 0; j < joints.size(); j++)
					{
						FbxAMatrix mat = joints[j].node->EvaluateGlobalTransform(animTime);

						end::joint animTransform;
						for (size_t j = 0; j < 4; j++)
							animTransform.global_xform.r[j] = { (float)mat.mData[j].mData[0], (float)mat.mData[j].mData[1], (float)mat.mData[j].mData[2], (float)mat.mData[j].mData[3] };
						animTransform.parent_index = joints[j].parent_index;

						mykeyframe.joints.push_back(animTransform);
					}
					
					animClip.frames.push_back(mykeyframe);
				}
				std::ofstream file(output_file_path, std::ios_base::binary | std::ios_base::trunc);
				if (file.is_open())
				{
					uint32_t size= animClip.frames.size();

					file.write((const char*)&animClip.duration, sizeof(double));
					file.write((const char*)&size, sizeof(uint32_t));
					for (uint32_t i = 0; i < size; i++)
					{
						uint32_t jointSize = animClip.frames[i].joints.size();
						file.write((const char*)&jointSize,sizeof(uint32_t));
						file.write((const char*)animClip.frames[i].joints.data(), sizeof(end::joint) * animClip.frames[i].joints.size());
						file.write((const char*)&animClip.frames[i].time, sizeof(double));
					}
				}
				file.close();
				return 0;
			}
		}
	}
	return result;
	
}

FBXEXPORTER_API int export_skinned_mesh(const char* fbx_file_path, const char* output_file_path, const char* mesh_name)
{
	int result = -1;
	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	if (!scene)
		return result;
	FbxNode* childNode = nullptr;
	int childCount = scene->GetRootNode()->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		childNode = scene->GetRootNode()->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();
		if (mesh)
		{
			const char* name = mesh->GetName();
			if (!mesh_name || mesh_name == mesh->GetName())
			{

				const int MAX_INFLUENCES = 4;
				using influence_set = std::array<end::influence, MAX_INFLUENCES>;
				std::vector<influence_set> control_point_influences;
				control_point_influences.resize(mesh->GetControlPointsCount());
				std::vector<end::fbx_Joint> joints;

				int posecount = scene->GetPoseCount();
				FbxPose* pose = scene->GetPose(0);
				FbxMesh* poseMesh = pose->GetNode(0)->GetMesh();
				if (pose->IsBindPose())
				{
					posecount = pose->GetCount();
					for (int i = 0; i < posecount; i++)
					{
						FbxSkeleton* skele = pose->GetNode(i)->GetSkeleton();
						if (skele && skele->IsSkeletonRoot())
						{
							end::fbx_Joint joint;
							joint.node = pose->GetNode(i);
							joint.parent_index = -1;
							joints.push_back(joint);
							for (int j = 0; j < joints.size(); j++)
							{
								for (int k = 0; k < joints[j].node->GetChildCount(); k++)
								{
									skele = joints[j].node->GetChild(k)->GetSkeleton();
									if (skele)
									{
										joint.node = joints[j].node->GetChild(k);
										joint.parent_index = j;
										joints.push_back(joint);
									}
								}
							}
							break;
						}
					}

					int deformerCount = poseMesh->GetDeformerCount();
					for (int i = 0; i < deformerCount; i++)
					{
						FbxDeformer* skin = poseMesh->GetDeformer(i);
						if (skin->Is<FbxSkin>())
						{
							FbxSkin* newSkin = (FbxSkin*)skin;
							int clusterCount = newSkin->GetClusterCount();
							for (int j = 0; j < clusterCount; j++)
							{
								FbxCluster* cluster = newSkin->GetCluster(j);
								for (int k = 0; k < joints.size(); k++)
								{
									if (joints[k].node == cluster->GetLink())
									{
										int indexCount = cluster->GetControlPointIndicesCount();
										double* weights = cluster->GetControlPointWeights();
										int* indexArray = cluster->GetControlPointIndices();
										for (int l = 0; l < indexCount; l++)
										{
											end::influence influence_to_add{ k, (float)weights[l] };
											influence_set set = control_point_influences[indexArray[l]];

											int min = -1;
											for (int m = 0; m < MAX_INFLUENCES; m++)
											{
												if (set[m].weight < influence_to_add.weight)
												{
													if (min < 0)
														min = m;
													else if (set[m].weight < set[min].weight)
														min = m;
												}
											}
											if (min > -1)
											{
												set[min] = influence_to_add;
											}
											control_point_influences[indexArray[l]] = set;
										}
									}
								}
							}
						}
					}
				}

				end::skinned_mesh simpleMesh;

				// getting vertices and count from fbx
				simpleMesh.vert_count = mesh->GetControlPointsCount();
				simpleMesh.verts = new end::skinned_vert[simpleMesh.vert_count];
				for (uint32_t j = 0; j < simpleMesh.vert_count; j++)
				{
					simpleMesh.verts[j].pos = DirectX::XMFLOAT3{ (float)mesh->GetControlPointAt(j).mData[0], (float)mesh->GetControlPointAt(j).mData[1], (float)mesh->GetControlPointAt(j).mData[2]};
				}
				// getting indices array and count from fbx
				simpleMesh.index_count = mesh->GetPolygonVertexCount();
				simpleMesh.indices = (uint32_t*)mesh->GetPolygonVertices();

				// getting uv's from fbx
				DirectX::XMFLOAT2* UV = new DirectX::XMFLOAT2[simpleMesh.index_count];
				for (int j = 0; j < mesh->GetPolygonCount(); j++)//polygon(=mostly rectangle) count
				{
					FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = NULL;
					mesh->GetTextureUV(&uvVertices);
					for (int k = 0; k < mesh->GetPolygonSize(j); k++)//retrieves number of vertices in a polygon
					{
						FbxVector2 uv = uvVertices->GetAt(mesh->GetTextureUVIndex(j, k));
						UV[3 * j + k] = DirectX::XMFLOAT2((float)uv.mData[0], 1 - (float)uv.mData[1]);
					}
				}

				// Get tangents and binormals
				FbxLayerElementArrayTemplate<FbxVector4>* fbxTangents;
				//FbxLayerElementArrayTemplate<FbxVector4> *fbxBinormal;
				mesh->GetTangents(&fbxTangents);
				//mesh->GetBinormals(&fbxBinormal);

				// getting normals from fbx
				FbxArray<FbxVector4> normalsVec;
				mesh->GetPolygonVertexNormals(normalsVec);

				end::skinned_vert* verts2 = new end::skinned_vert[simpleMesh.index_count];
				simpleMesh.vert_count = simpleMesh.index_count;
				// re-index
				for (uint32_t j = 0; j < simpleMesh.index_count; j++)
				{
					verts2[j] = simpleMesh.verts[simpleMesh.indices[j]];

					verts2[j].norm = DirectX::XMFLOAT3{ (float)normalsVec[j].mData[0], (float)normalsVec[j].mData[1], (float)normalsVec[j].mData[2] };
					verts2[j].tex = UV[j];

					influence_set set = control_point_influences[simpleMesh.indices[j]];

					verts2[j].indices[0] = set[0].joint;
					verts2[j].indices[1] = set[1].joint;
					verts2[j].indices[2] = set[2].joint;
					verts2[j].indices[3] = set[3].joint;

					//DirectX::XMVECTOR normal{ set[0].weight, set[1].weight, set[2].weight, set[3].weight };
					//normal = DirectX::XMVector4Normalize(normal);
					//verts2[j].weights = { XMVectorGetX(normal), XMVectorGetY(normal), XMVectorGetZ(normal), XMVectorGetW(normal) };

					float sum = set[0].weight + set[1].weight + set[2].weight + set[3].weight;
					verts2[j].weights = XMFLOAT4{ set[0].weight / sum,set[1].weight / sum,set[2].weight / sum,set[3].weight / sum };
					

				};

				delete[] simpleMesh.verts;
				simpleMesh.verts = verts2;

				unsigned int numIndices = 0;
				std::unordered_map<end::skinned_vert, unsigned int, end::fnv1a> uniqueValues;
				std::vector<end::skinned_vert> vertices;
				std::vector<unsigned int> indicesVector;

				for (uint32_t j = 0; j < simpleMesh.index_count; j++)
				{
					end::skinned_vert v = simpleMesh.verts[j];
					if (uniqueValues.count(v) == 0)
					{
						uniqueValues.insert({ v, numIndices });
						vertices.push_back(v);
						indicesVector.push_back(numIndices++);
					}
					else
						indicesVector.push_back(uniqueValues[v]);
				}

				std::ofstream file(output_file_path, std::ios_base::binary | std::ios_base::trunc);
				if (file.is_open())
				{
					int size = (int)vertices.size();
					file.write((const char*)&size, sizeof(int));
					file.write((const char*)vertices.data(), sizeof(end::skinned_vert) * vertices.size());
					size = (int)indicesVector.size();
					file.write((const char*)&size, sizeof(int));
					file.write((const char*)indicesVector.data(), sizeof(int) * indicesVector.size());
				}
				file.close();

				delete[] verts2;
				delete[] UV;
			}
		}
	}
	return 0;
}
