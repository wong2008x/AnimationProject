#pragma once
#ifdef FBXEXPORTER_EXPORTS
#define FBXEXPORTER_API __declspec(dllexport)
#else
#define FBXEXPORTER_API __declspec(dllimport)
#endif


extern "C" FBXEXPORTER_API int get_scene_poly_count(const char* fbx_file_path);

extern "C" FBXEXPORTER_API int export_simple_mesh(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
extern "C" FBXEXPORTER_API int export_material(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
extern "C" FBXEXPORTER_API int export_bindpose(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
extern "C" FBXEXPORTER_API int export_animation(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
//extern "C" FBXEXPORTER_API void LoadAnimations(const char** pchFilepath, int nAnimationCount);
//
//extern "C" FBXEXPORTER_API int export_inversed_simple_mesh(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
//extern "C" FBXEXPORTER_API int export_inversed_skinned_mesh(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
//extern "C" FBXEXPORTER_API int export_inversed_bindpose(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);
//
//extern "C" FBXEXPORTER_API int export_inversed_animation(const char* fbx_file_path, const char* output_file_path, const char* mesh_name = nullptr);