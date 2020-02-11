#pragma once
#include "pch.h" 
#include "InterFace.h"
#include "simple_mesh.h"

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
	std::vector<end::material_t> materials;
	std::vector<end::file_path_t> paths;
	FbxScene* scene = nullptr;
	FbxManager* sdk_manager = create_and_import(fbx_file_path, scene);

	if (!scene)
		return -1;

	//int num_mats = scene->GetMaterialCount();
	//for (int m = 0; m < num_mats; ++m)
	//{
	//	end::material_t my_mat;
	//	FbxSurfaceMaterial* mat = scene->GetMaterial(m);
	//	if (mat->Is<FbxSurfaceLambert>() == false) // Non-standard material, skip for now
	//		continue;
	//	FbxSurfaceLambert* lam = (FbxSurfaceLambert*)mat;
	//	FbxDouble3 diffuse_color = lam->Diffuse.Get();
	//	FbxDouble diffuse_factor = lam->DiffuseFactor.Get();
	//	my_mat[end::material_t::DIFFUSE].value = diffuse_color;
	//	my_mat[end::material_t::DIFFUSE].factor = diffuse_factor;
	//		if (FbxFileTexture* file_texture = lam->Diffuse.GetSrcObject<FbxFileTexture>())
	//		{
	//			const char* file_name = file_texture->GetRelativeFileName();
	//			end::file_path_t file_path;
	//			strcpy(file_path.data(), file_name);
	//			my_mat[end::material_t::DIFFUSE].input = paths.size();
	//			paths.push_back(file_path);
	//		}
	//	// Get emissive property as above
	//	// ...
	//	if (mat->Is<FbxSurfacePhong>())
	//	{
	//		// Get specular related properties...
	//	}
	//	// add my_mat materials vector...
	//}

	FbxNode* childNode = nullptr;
	int childCount = scene->GetRootNode()->GetChildCount();
	for (int i = 0; i < childCount; i++)
	{
		childNode = scene->GetRootNode()->GetChild(i);
		FbxMesh* mesh = childNode->GetMesh();
		if (mesh)
		{
				mesh_name = mesh->GetName();
				end::simple_mesh simpleMesh;

				int colorCount = mesh->GetElementVertexColorCount();
				// getting vertices and count from fbx
				simpleMesh.vert_count = mesh->GetControlPointsCount();
				simpleMesh.verts = new end::simple_vert[simpleMesh.vert_count];
				for (uint32_t j = 0; j < simpleMesh.vert_count; j++)
				{
					simpleMesh.verts[j].pos = DirectX::XMFLOAT3{ 
						(float)mesh->GetControlPointAt(j).mData[0], 
						(float)mesh->GetControlPointAt(j).mData[1],
						(float)mesh->GetControlPointAt(j).mData[2] };
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
						(float)fbxTangents->GetAt(j).mData[2],
					};
				};

				delete[] simpleMesh.verts;
				simpleMesh.verts = verts2;

				unsigned int numIndices = 0;
				std::unordered_map<end::simple_vert, uint64_t,end::fnv1a> uniqueValues;
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
				delete []verts2;
				delete []UV;
				return 0;
			
		}
	}
	return -1;
}

