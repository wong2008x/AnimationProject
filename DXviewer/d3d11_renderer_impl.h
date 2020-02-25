#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dxgi1_2.h>
#include <d3d11_2.h>
#include <random>
#include <ctime>
#include "WICTextureLoader.h"
#include "../FBXexporter/e_material.h"
#include <fstream>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")


#include "renderer.h"
#include "Camera.h"
#include "frustum_culling.h"
#include "view.h"
#include "blob.h"
#include "XTime.h"
#include "shaders/mvp.hlsli"

// NOTE: This header file must *ONLY* be included by renderer.cpp

namespace
{
	template<typename T>
	void safe_release(T* t)
	{ 
		if (t)
			t->Release();
	}
}


namespace end
{
	using namespace DirectX;

	struct renderer_t::impl_t
	{
		// platform/api specific members, functions, etc.
		// Device, swapchain, resource views, states, etc. can be members here
		HWND hwnd;

		ID3D11Device *device = nullptr;
		ID3D11DeviceContext *context = nullptr;
		IDXGISwapChain *swapchain = nullptr;

		ID3D11RenderTargetView*		render_target[VIEW_RENDER_TARGET::COUNT]{};

		ID3D11DepthStencilView*		depthStencilView[VIEW_DEPTH_STENCIL::COUNT]{};

		ID3D11DepthStencilState*	depthStencilState[STATE_DEPTH_STENCIL::COUNT]{};

		ID3D11RasterizerState*		rasterState[STATE_RASTERIZER::COUNT]{};

		ID3D11Buffer*				vertex_buffer[VERTEX_BUFFER::COUNT]{};

		ID3D11Buffer*				index_buffer[INDEX_BUFFER::COUNT]{};
		
		ID3D11InputLayout*			input_layout[INPUT_LAYOUT::COUNT]{};

		ID3D11VertexShader*			vertex_shader[VERTEX_SHADER::COUNT]{};

		ID3D11PixelShader*			pixel_shader[PIXEL_SHADER::COUNT]{};

		ID3D11Buffer*				constant_buffer[CONSTANT_BUFFER::COUNT]{};

		D3D11_VIEWPORT				view_port[VIEWPORT::COUNT]{};

		ID3D11ShaderResourceView* player_texture_resource[TEXTURE_RESOURCE::COUNT]{};

		ID3D11SamplerState* sampler_state[STATE_SAMPLER::COUNT]{};

		std::unique_ptr<DirectX::Mouse> m_pMouse;
		DirectX::Mouse::ButtonStateTracker m_MouseTracker;
		std::unique_ptr<DirectX::Keyboard> m_pKeyboard;
		DirectX::Keyboard::KeyboardStateTracker m_KeyboardTracker;



		float3 startingPos = {0,0,0};
		XTime timer;
		
		XMFLOAT4 colours[3] = { {1,0,0,1},{0,1,0,1},{0,0,1,1} };
		double delta = 0;
		

		lightCons lightingConstant;
		mTransform myTransform;
		Camera myCam;
		frustum_t myFrustum;
		XMMATRIX m_Lookat =XMMatrixIdentity();
		XMMATRIX m_Turnto= XMMatrixIdentity();
		XMMATRIX m_Matrix = XMMatrixIdentity();
		XMMATRIX m_ViewMatrix= XMMatrixIdentity();

		aabb_t myAABBs[5];

		XMVECTOR eyepos = XMVectorSet(0.0f, 15.0f, -15.0f, 1.0f);
		XMVECTOR focus = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		
		int g_mouseX=0,g_mouseY=0;

		float cameraSpeed = 0.5f;
		const XMVECTOR DefaultUp = { 0,1,0,0 };
		const XMVECTOR DefaultForward = { 0,0,1,0 };
		const XMVECTOR DefaultRight = { 1,0,0,0 };

		std::vector<simpleVert> mageVert;
		std::vector<uint32_t> mageIndex;
		std::vector<joint>myJoints;
		std::vector<joint>invJoints;

		std::vector<skinnedVert>mageSkinnedVert;
		std::vector<uint32_t> mageSkinnedIndex;

		anim_clip animClip;
		XTime animTimer;
		int animStep;
		double currentTime = 0;
		bool AnimCtrl = false;
		/* Add more as needed...
		ID3D11SamplerState*			sampler_state[STATE_SAMPLER::COUNT]{};

		ID3D11BlendState*			blend_state[STATE_BLEND::COUNT]{};
		*/

		// Constructor for renderer implementation
		// 
		impl_t(native_handle_type window_handle, view_t& default_view)
		{
			hwnd = (HWND)window_handle;


			srand(time(NULL));

			create_device_and_swapchain();

			create_main_render_target();

			setup_depth_stencil();

			setup_rasterizer();

			create_shaders();

			create_debug_renderer();
			
			create_constant_buffers();

			create_sampler_state();

			MatrixInit();


			load_pose("..//Assets//BattleMageBind.bin",myJoints,invJoints);

			load_animation("..//Assets//BattleMageRun.anim",animClip);

			m_pMouse = std::make_unique<Mouse>();
			m_pKeyboard = std::make_unique<Keyboard>();

			float aspect = view_port[VIEWPORT::DEFAULT].Width / view_port[VIEWPORT::DEFAULT].Height;

			eyepos = XMVectorSet(0.0f, 10.0f, -15.0f, 1.0f);
			focus = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
			up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			m_ViewMatrix =XMMatrixLookAtLH(eyepos, focus, up);
			//default_view.view_mat = (float4x4_a&)XMMatrixInverse(nullptr, m_ViewMatrix);
			default_view.proj_mat = (float4x4_a&)XMMatrixPerspectiveFovLH(3.1415926f / 4.0f, aspect, 0.01f, 100.0f);
			lightingConstant.dLightClr = { 0.7f,0.7f,0.5f,1.0f };
			lightingConstant.dLightDir = { -0.557f,-0.557f,0.557f,1 };

			
			timer.Restart();
			animTimer.Restart();
			animStep = 0;
		}

		void draw_view(view_t& view)
		{
			timer.Signal();
			D3D11_MAPPED_SUBRESOURCE gpuBuffer;
			ZeroMemory(&gpuBuffer, sizeof(gpuBuffer));
			const float4 black{ 0.0f, 0.0f, 0.0f, 1.0f };

			context->OMSetDepthStencilState(depthStencilState[STATE_DEPTH_STENCIL::DEFAULT], 1);
			context->OMSetRenderTargets(1, &render_target[VIEW_RENDER_TARGET::DEFAULT], depthStencilView[VIEW_DEPTH_STENCIL::DEFAULT]);

			context->ClearRenderTargetView(render_target[VIEW_RENDER_TARGET::DEFAULT], black.data());
			context->ClearDepthStencilView(depthStencilView[VIEW_DEPTH_STENCIL::DEFAULT], D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

			context->RSSetState(rasterState[STATE_RASTERIZER::DEFAULT]);
			context->RSSetViewports(1, &view_port[VIEWPORT::DEFAULT]);


			MVP_t mvp;

			mvp.modeling = XMMatrixIdentity();
			mvp.projection = XMMatrixTranspose((XMMATRIX&)view.proj_mat);
			mvp.view = XMMatrixTranspose(m_ViewMatrix);

			UINT colorStrides[] = { sizeof(colored_vertex) };
			UINT coloroffsets[] = { 0 };

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
			context->IASetVertexBuffers(0,1,&vertex_buffer[VERTEX_BUFFER::COLORED_VERTEX],colorStrides,coloroffsets);
			context->VSSetShader(vertex_shader[VERTEX_SHADER::COLORED_VERTEX], nullptr, 0);
			context->PSSetShader(pixel_shader[PIXEL_SHADER::COLORED_VERTEX], nullptr, 0);

			context->IASetInputLayout(input_layout[INPUT_LAYOUT::COLORED_VERTEX]);

			context->VSSetConstantBuffers(0, 1, &constant_buffer[CONSTANT_BUFFER::MVP]);

			//context->UpdateSubresource(constant_buffer[CONSTANT_BUFFER::MVP], 0, NULL, &mvp, 0, 0);
			HRESULT hr = context->Map(constant_buffer[CONSTANT_BUFFER::MVP], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((MVP_t*)(gpuBuffer.pData)) = mvp;
			context->Unmap(constant_buffer[CONSTANT_BUFFER::MVP], 0);

			context->UpdateSubresource(vertex_buffer[VERTEX_BUFFER::COLORED_VERTEX], 0, NULL, debug_renderer::get_line_verts(), 0, 0);

			context->Draw(debug_renderer::get_line_vert_count(), 0);

			debug_renderer::clear_lines();


			//UINT meshStrides[] = { sizeof(simpleVert) };
			//UINT meshoffsets[] = { 0 };

			//mvp.modeling = XMMatrixIdentity()*XMMatrixRotationY(XMConvertToRadians(180));
			//context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			//context->IASetVertexBuffers(0, 1, &vertex_buffer[VERTEX_BUFFER::SIMPLEMESH], meshStrides, meshoffsets);
			//context->IASetIndexBuffer(index_buffer[INDEX_BUFFER::SIMPLEMESH], DXGI_FORMAT_R32_UINT, 0);
			//context->VSSetShader(vertex_shader[VERTEX_SHADER::SIMPLEMESH], nullptr, 0);
			//context->PSSetShader(pixel_shader[PIXEL_SHADER::SIMPLEMESH], nullptr, 0);
			//context->PSSetShaderResources(0,3, player_texture_resource);

			//context->IASetInputLayout(input_layout[INPUT_LAYOUT::SIMPLEMESH]);
			//context->VSSetConstantBuffers(0, 1, &constant_buffer[CONSTANT_BUFFER::MVP]);
			//context->PSSetConstantBuffers(0, 1, &constant_buffer[CONSTANT_BUFFER::LIGHT]);
			//context->PSSetSamplers(0,1,&sampler_state[STATE_SAMPLER::DEFAULT]);
		
			//
			//lightingConstant.dLightDir = XMVector4Transform(lightingConstant.dLightDir, XMMatrixRotationY(XMConvertToRadians(50 * timer.Delta())));

			//
			//context->Map(constant_buffer[CONSTANT_BUFFER::MVP], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			//*((MVP_t*)(gpuBuffer.pData)) = mvp;
			//context->Unmap(constant_buffer[CONSTANT_BUFFER::MVP], 0);

			//ZeroMemory(&gpuBuffer,sizeof(gpuBuffer));
			//hr = context->Map(constant_buffer[CONSTANT_BUFFER::LIGHT], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			//*((lightCons*)(gpuBuffer.pData)) = lightingConstant;
			//context->Unmap(constant_buffer[CONSTANT_BUFFER::LIGHT], 0);

			//context->DrawIndexed(mageIndex.size(), 0, 0);



			UINT skinnedStrides[] = { sizeof(skinnedVert) };
			UINT skinnedoffsets[] = { 0 };

			
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context->IASetVertexBuffers(0, 1, &vertex_buffer[VERTEX_BUFFER::SKINNEDMESH], skinnedStrides, skinnedoffsets);
			context->IASetIndexBuffer(index_buffer[INDEX_BUFFER::SKINNEDMESH], DXGI_FORMAT_R32_UINT, 0);
			context->VSSetShader(vertex_shader[VERTEX_SHADER::SKINNEDMESH], nullptr, 0);

			context->PSSetShader(pixel_shader[PIXEL_SHADER::SIMPLEMESH], nullptr, 0);
			context->PSSetShaderResources(0, 3, player_texture_resource);

			context->IASetInputLayout(input_layout[INPUT_LAYOUT::SKINNEDMESH]);
			context->VSSetConstantBuffers(0, 1, &constant_buffer[CONSTANT_BUFFER::MVP]);
			context->VSSetConstantBuffers(1, 1, &constant_buffer[CONSTANT_BUFFER::TRANSFORM]);
			context->PSSetConstantBuffers(0, 1, &constant_buffer[CONSTANT_BUFFER::LIGHT]);

			context->PSSetSamplers(0, 1, &sampler_state[STATE_SAMPLER::DEFAULT]);


			lightingConstant.dLightDir = XMVector4Transform(lightingConstant.dLightDir, XMMatrixRotationY(XMConvertToRadians(50 * timer.Delta())));


			context->Map(constant_buffer[CONSTANT_BUFFER::MVP], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((MVP_t*)(gpuBuffer.pData)) = mvp;
			context->Unmap(constant_buffer[CONSTANT_BUFFER::MVP], 0);

			mvp.modeling = XMMatrixIdentity()*XMMatrixTranslation(-5,0,0);
			for (size_t i = 0; i < invJoints.size(); i++)	
			{
				myTransform.m[i] = XMMatrixMultiply(invJoints[i].global_xform, animClip.frames[animStep].joints[i].global_xform)*XMMatrixTranslation(-5, 0, 0);
				//myTransform.m[i] = XMMatrixMultiply(XMMatrixInverse(nullptr, myJoints[i].global_xform), animClip.frames[animStep].joints[i].global_xform);
				//myTransform.m[i] = XMMatrixMultiply(invJoints[i].global_xform, myJoints[i].global_xform);
				//myTransform.m[i] = XMMatrixIdentity(); //myJoints[i].global_xform;
				//myTransform.m[i] = invJoints[i].global_xform;
			}

			ZeroMemory(&gpuBuffer, sizeof(gpuBuffer));
			hr = context->Map(constant_buffer[CONSTANT_BUFFER::TRANSFORM], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((mTransform*)(gpuBuffer.pData)) = myTransform;
			context->Unmap(constant_buffer[CONSTANT_BUFFER::TRANSFORM], 0);

			ZeroMemory(&gpuBuffer, sizeof(gpuBuffer));
			hr = context->Map(constant_buffer[CONSTANT_BUFFER::LIGHT], 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
			*((lightCons*)(gpuBuffer.pData)) = lightingConstant;
			context->Unmap(constant_buffer[CONSTANT_BUFFER::LIGHT], 0);



			context->DrawIndexed(mageSkinnedIndex.size(), 0, 0);
			swapchain->Present(1, 0);
		}

		~impl_t()
		{
			// TODO:
			//Clean-up
			//
			// In general, release objects in reverse order of creation

			for (auto& ptr : constant_buffer)
				safe_release(ptr);

			for (auto& ptr : pixel_shader)
				safe_release(ptr);

			for (auto& ptr : vertex_shader)
				safe_release(ptr);

			for (auto& ptr : input_layout)
				safe_release(ptr);

			for (auto& ptr : index_buffer)
				safe_release(ptr);
			
			for (auto& ptr : vertex_buffer)
				safe_release(ptr);

			for (auto& ptr : rasterState)
				safe_release(ptr);

			for (auto& ptr : depthStencilState)
				safe_release(ptr);

			for (auto& ptr : depthStencilView)
				safe_release(ptr);

			for (auto& ptr : render_target)
				safe_release(ptr);

			for (auto& ptr : player_texture_resource)
				safe_release(ptr);

			for (auto& ptr : sampler_state)
				safe_release(ptr);

			safe_release(context);
			safe_release(swapchain);
			safe_release(device);
		}

		void create_device_and_swapchain()
		{
			RECT crect;
			GetClientRect(hwnd, &crect);

			// Setup the viewport
			D3D11_VIEWPORT &vp = view_port[VIEWPORT::DEFAULT];

			vp.Width = (float)crect.right;
			vp.Height = (float)crect.bottom;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;

			// Setup swapchain
			DXGI_SWAP_CHAIN_DESC sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = 1;
			sd.BufferDesc.Width = crect.right;
			sd.BufferDesc.Height = crect.bottom;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hwnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = TRUE;

			D3D_FEATURE_LEVEL  FeatureLevelsSupported;

			const D3D_FEATURE_LEVEL lvl[] = 
			{
				D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
				D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
			};

			UINT createDeviceFlags = 0;

			#ifdef _DEBUG
						createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			#endif

			HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, lvl, _countof(lvl), D3D11_SDK_VERSION, &sd, &swapchain, &device, &FeatureLevelsSupported, &context);

			if (hr == E_INVALIDARG)
			{
				hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, &lvl[1], _countof(lvl) - 1, D3D11_SDK_VERSION, &sd, &swapchain, &device, &FeatureLevelsSupported, &context);
			}

			assert(!FAILED(hr));
		}

		void create_main_render_target()
		{
			ID3D11Texture2D* pBackBuffer;
			// Get a pointer to the back buffer
			HRESULT hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D),
				(LPVOID*)&pBackBuffer);

			assert(!FAILED(hr));

			// Create a render-target view
			device->CreateRenderTargetView(pBackBuffer, NULL,
				&render_target[VIEW_RENDER_TARGET::DEFAULT]);

			pBackBuffer->Release();
		}

		void setup_depth_stencil()
		{
			/* DEPTH_BUFFER */
			D3D11_TEXTURE2D_DESC depthBufferDesc;
			ID3D11Texture2D *depthStencilBuffer;

			ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

			depthBufferDesc.Width = (UINT)view_port[VIEWPORT::DEFAULT].Width;
			depthBufferDesc.Height = (UINT)view_port[VIEWPORT::DEFAULT].Height;
			depthBufferDesc.MipLevels = 1;
			depthBufferDesc.ArraySize = 1;
			depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthBufferDesc.SampleDesc.Count = 1;
			depthBufferDesc.SampleDesc.Quality = 0;
			depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthBufferDesc.CPUAccessFlags = 0;
			depthBufferDesc.MiscFlags = 0;

			HRESULT hr = device->CreateTexture2D(&depthBufferDesc, NULL, &depthStencilBuffer);

			assert(!FAILED(hr));

			/* DEPTH_STENCIL */
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

			ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

			depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			hr = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDesc, &depthStencilView[VIEW_DEPTH_STENCIL::DEFAULT]);
			
			assert(!FAILED(hr));

			depthStencilBuffer->Release();

			/* DEPTH_STENCIL_DESC */
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

			ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

			depthStencilDesc.DepthEnable = true;
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

			hr = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState[STATE_DEPTH_STENCIL::DEFAULT]);

			assert(!FAILED(hr));
		}

		void setup_rasterizer()
		{
			D3D11_RASTERIZER_DESC rasterDesc;

			ZeroMemory(&rasterDesc, sizeof(rasterDesc));

			rasterDesc.AntialiasedLineEnable = true;
			rasterDesc.CullMode = D3D11_CULL_BACK;
			rasterDesc.DepthBias = 0;
			rasterDesc.DepthBiasClamp = 0.0f;
			rasterDesc.DepthClipEnable = false;
			rasterDesc.FillMode = D3D11_FILL_SOLID;
			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.MultisampleEnable = false;
			rasterDesc.ScissorEnable = false;
			rasterDesc.SlopeScaledDepthBias = 0.0f;

			HRESULT hr = device->CreateRasterizerState(&rasterDesc, &rasterState[STATE_RASTERIZER::DEFAULT]);

			assert(!FAILED(hr));
		}

		void create_sampler_state()
		{
			D3D11_SAMPLER_DESC sampDesc = {};
			sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			sampDesc.MinLOD = 0;
			sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

			HRESULT hr = device->CreateSamplerState(&sampDesc, &sampler_state[STATE_SAMPLER::DEFAULT]);
		}
		void create_shaders()
		{


			binary_blob_t vs_clr = load_binary_blob("vs_clrVerts.cso");
			binary_blob_t ps_clr = load_binary_blob("ps_clrVerts.cso");

			HRESULT hr = device->CreateVertexShader(vs_clr.data(), vs_clr.size(), NULL, &vertex_shader[VERTEX_SHADER::COLORED_VERTEX]);

			assert(!FAILED(hr));

			hr = device->CreatePixelShader(ps_clr.data(), ps_clr.size(), NULL, &pixel_shader[PIXEL_SHADER::COLORED_VERTEX]);

			assert(!FAILED(hr));

			D3D11_INPUT_ELEMENT_DESC InputLayout[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};

			device->CreateInputLayout(InputLayout, 2, vs_clr.data(), vs_clr.size(), &input_layout[INPUT_LAYOUT::COLORED_VERTEX]);



			load_fbx_model("..//Assets//BattleMageMesh.bin", mageVert, mageIndex);


			D3D11_BUFFER_DESC BufferDesc;
			ZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			BufferDesc.ByteWidth = sizeof(simpleVert) * mageVert.size();
			BufferDesc.CPUAccessFlags = NULL;
			BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA VerticesData;
			ZeroMemory(&VerticesData, sizeof(VerticesData));
			VerticesData.pSysMem = mageVert.data();
			 hr = device->CreateBuffer(&BufferDesc, &VerticesData, &vertex_buffer[VERTEX_BUFFER::SIMPLEMESH]);

			ZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			BufferDesc.ByteWidth = sizeof(uint32_t) * mageIndex.size();
			BufferDesc.CPUAccessFlags = NULL;
			BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			ZeroMemory(&VerticesData, sizeof(VerticesData));
			VerticesData.pSysMem = mageIndex.data();
			hr = device->CreateBuffer(&BufferDesc, &VerticesData, &index_buffer[INDEX_BUFFER::SIMPLEMESH]);

			binary_blob_t vs_mesh = load_binary_blob("vs_simpleVerts.cso");
			binary_blob_t ps_mesh = load_binary_blob("ps_simpleVerts.cso");

			hr = device->CreateVertexShader(vs_mesh.data(), vs_mesh.size(), NULL, &vertex_shader[VERTEX_SHADER::SIMPLEMESH]);

			load_material("..//Assets//BattleMageMesh.mat",player_texture_resource);
	
			assert(!FAILED(hr));

			hr = device->CreatePixelShader(ps_mesh.data(), ps_mesh.size(), NULL, &pixel_shader[PIXEL_SHADER::SIMPLEMESH]);

			assert(!FAILED(hr));

			D3D11_INPUT_ELEMENT_DESC InputLayout1[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			device->CreateInputLayout(InputLayout1, 4, vs_mesh.data(), vs_mesh.size(), &input_layout[INPUT_LAYOUT::SIMPLEMESH]);


			load_fbx_model_skinned("..//Assets//BattleMageRun.bin", mageSkinnedVert, mageSkinnedIndex);


			ZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			BufferDesc.ByteWidth = sizeof(skinnedVert) * mageSkinnedVert.size();
			BufferDesc.CPUAccessFlags = NULL;
			BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			ZeroMemory(&VerticesData, sizeof(VerticesData));
			VerticesData.pSysMem = mageSkinnedVert.data();
			hr = device->CreateBuffer(&BufferDesc, &VerticesData, &vertex_buffer[VERTEX_BUFFER::SKINNEDMESH]);

			ZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			BufferDesc.ByteWidth = sizeof(uint32_t) * mageSkinnedIndex.size();
			BufferDesc.CPUAccessFlags = NULL;
			BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			ZeroMemory(&VerticesData, sizeof(VerticesData));
			VerticesData.pSysMem = mageSkinnedIndex.data();
			hr = device->CreateBuffer(&BufferDesc, &VerticesData, &index_buffer[INDEX_BUFFER::SKINNEDMESH]);

			binary_blob_t vs_skinnedmesh = load_binary_blob("vs_skinnedMesh.cso");

			hr = device->CreateVertexShader(vs_skinnedmesh.data(), vs_skinnedmesh.size(), NULL, &vertex_shader[VERTEX_SHADER::SKINNEDMESH]);

			D3D11_INPUT_ELEMENT_DESC InputLayout2[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			hr = device->CreateInputLayout(InputLayout2, 5, vs_skinnedmesh.data(), vs_skinnedmesh.size(), &input_layout[INPUT_LAYOUT::SKINNEDMESH]);

			//float3 pos : POSITION;
			//float3 normal : NORMAL;
			//float2 uv : TEXCOORD;
			//float4 weights : BLENDWEIGHTS;
			//int4 indices : BLENDINDICES;

		}
		
		void load_material(const char* path, ID3D11ShaderResourceView** textureResource)
		{
			std::vector<dev5::material_t> materials;
			std::vector<dev5::file_path_t> paths;
			std::fstream load{ path, std::ios_base::in | std::ios_base::binary };

			assert(load.is_open());

			if (!load.is_open())
			{
				assert(false);
				return;
			}

			uint32_t mat_num;
			uint32_t path_num;
			load.read((char*)&mat_num, sizeof(uint32_t));
			materials.resize(mat_num);
			load.read((char*)materials.data(), sizeof(dev5::material_t) * mat_num);
			load.read((char*)&path_num, sizeof(uint32_t));
			paths.resize(path_num);
			load.read((char*)paths.data(), sizeof(dev5::file_path_t) * path_num);
			load.close();

			std::wstring wString(paths[materials[0][dev5::material_t::DIFFUSE].input].begin(), paths[materials[0][dev5::material_t::DIFFUSE].input].end());
			const wchar_t* relativePath=L"..\\Assets\\";
			wString=relativePath + wString;
			const wchar_t* convertedFilePath = wString.c_str();

			CreateWICTextureFromFile(device, convertedFilePath, NULL, &textureResource[TEXTURE_RESOURCE::MAGE_DIFFUSE]);

			wString.assign(paths[materials[0][dev5::material_t::EMISSIVE].input].begin(), paths[materials[0][dev5::material_t::EMISSIVE].input].end());
			wString = relativePath + wString;
			convertedFilePath = wString.c_str();

			CreateWICTextureFromFile(device, convertedFilePath, NULL, &textureResource[TEXTURE_RESOURCE::MAGE_EMISSIVE]);

			wString.assign(paths[materials[0][dev5::material_t::SPECULAR].input].begin(), paths[materials[0][dev5::material_t::SPECULAR].input].end());
			wString = relativePath + wString;
			convertedFilePath = wString.c_str();

			CreateWICTextureFromFile(device, convertedFilePath, NULL, &textureResource[TEXTURE_RESOURCE::MAGE_SPEC]);
		}
		void create_constant_buffers()
		{
			D3D11_BUFFER_DESC mvp_bd;
			ZeroMemory(&mvp_bd, sizeof(mvp_bd));

			mvp_bd.Usage = D3D11_USAGE_DYNAMIC;
			mvp_bd.ByteWidth = sizeof(MVP_t);
			mvp_bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			mvp_bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			HRESULT hr = device->CreateBuffer(&mvp_bd, NULL, &constant_buffer[CONSTANT_BUFFER::MVP]);

			D3D11_BUFFER_DESC light_bd;
			ZeroMemory(&light_bd, sizeof(light_bd));

			light_bd.Usage = D3D11_USAGE_DYNAMIC;
			light_bd.ByteWidth = sizeof(lightCons);
			light_bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			light_bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			hr = device->CreateBuffer(&mvp_bd, NULL, &constant_buffer[CONSTANT_BUFFER::LIGHT]);

			D3D11_BUFFER_DESC mTransf;
			ZeroMemory(&mTransf, sizeof(mTransf));

			mTransf.Usage = D3D11_USAGE_DYNAMIC;
			mTransf.ByteWidth = sizeof(mTransform);
			mTransf.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			mTransf.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			hr = device->CreateBuffer(&mTransf, NULL, &constant_buffer[CONSTANT_BUFFER::TRANSFORM]);
		}

		void create_debug_renderer()
		{
			
			D3D11_BUFFER_DESC BufferDesc;
			ZeroMemory(&BufferDesc, sizeof(BufferDesc));
			BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			BufferDesc.ByteWidth = sizeof(colored_vertex)* debug_renderer::get_line_vert_capacity();
			BufferDesc.CPUAccessFlags = NULL;
			BufferDesc.Usage = D3D11_USAGE_DEFAULT;
			BufferDesc.MiscFlags = 0;
			BufferDesc.StructureByteStride = 0;
			D3D11_SUBRESOURCE_DATA VerticesData;
			ZeroMemory(&VerticesData, sizeof(VerticesData));
			VerticesData.pSysMem = debug_renderer::get_line_verts();
			device->CreateBuffer(&BufferDesc, &VerticesData, &vertex_buffer[VERTEX_BUFFER::COLORED_VERTEX]);
		}

		void UpdateGrid()
		{
			XMFLOAT4 color = { 0.1f, 0.2f, 0.3f, 1.0f };

			for (size_t i = 0; i < 22; i++)
			{
				debug_renderer::add_line(XMFLOAT3{ i - 11.f ,0 , 10.f }, XMFLOAT3{ i - 11.f, 0, -11.f }, color);
				debug_renderer::add_line(XMFLOAT3{ -11.f ,0 , i - 11.f }, XMFLOAT3{ 10.f ,0 , i - 11.f }, color);
			}
		}

		void DrawPoseTransform(joint& myJoint)
		{
			XMVECTOR pos = myJoint.global_xform.r[3];

			XMVECTOR Xaxis = pos + XMVector3Normalize(myJoint.global_xform.r[0]) * 0.2f;
			XMVECTOR Yaxis = pos + XMVector3Normalize(myJoint.global_xform.r[1]) * 0.2f;
			XMVECTOR Zaxis = pos + XMVector3Normalize(myJoint.global_xform.r[2]) * 0.2f;
			debug_renderer::add_line({ XMVectorGetX(pos),XMVectorGetY(pos),XMVectorGetZ(pos) }, { XMVectorGetX(Xaxis),XMVectorGetY(Xaxis),XMVectorGetZ(Xaxis) }, colours[0]);
			debug_renderer::add_line({ XMVectorGetX(pos),XMVectorGetY(pos),XMVectorGetZ(pos) }, { XMVectorGetX(Yaxis),XMVectorGetY(Yaxis),XMVectorGetZ(Yaxis) }, colours[1]);
			debug_renderer::add_line({ XMVectorGetX(pos),XMVectorGetY(pos),XMVectorGetZ(pos) }, { XMVectorGetX(Zaxis),XMVectorGetY(Zaxis),XMVectorGetZ(Zaxis) }, colours[2]);
		}

		void UpdateAnimation()
		{
			if (!AnimCtrl)
			{
				animTimer.Signal();
				currentTime += animTimer.Delta();
				if (currentTime > animClip.duration)
				{
					animTimer.Restart();
					animStep = 0;
					currentTime = 0;
				}



				if (animClip.frames[animStep].time < currentTime)
				{
					if (animStep < animClip.frames.size() - 1)
						animStep++;
				}
			}
			for (size_t i = 0; i < animClip.frames[animStep].joints.size(); i++)
			{

				DrawPoseTransform(animClip.frames[animStep].joints[i]);
				XMFLOAT3 pos1;
				XMFLOAT3 pos2;
				XMStoreFloat3(&pos1, animClip.frames[animStep].joints[i].global_xform.r[3]);
				if (animClip.frames[animStep].joints[i].parent_index != -1)
				{
					XMStoreFloat3(&pos2, animClip.frames[animStep].joints[animClip.frames[animStep].joints[i].parent_index].global_xform.r[3]);
					debug_renderer::add_line(pos1, pos2, (XMFLOAT4)DirectX::Colors::Azure);
				}
			}

			//Draw T pose
			for (size_t i = 0; i < myJoints.size(); i++)
			{

				DrawPoseTransform(myJoints[i]);
				XMFLOAT3 pos1;
				XMFLOAT3 pos2;
				XMStoreFloat3(&pos1, myJoints[i].global_xform.r[3]);
				if (myJoints[i].parent_index != -1)
				{
					XMStoreFloat3(&pos2, myJoints[myJoints[i].parent_index].global_xform.r[3]);
					debug_renderer::add_line(pos1, pos2, (XMFLOAT4)DirectX::Colors::Azure);
				}
			}

			for (size_t i = 0; i < invJoints.size(); i++)
			{
				DrawPoseTransform(invJoints[i]);
				XMFLOAT3 pos1;
				XMFLOAT3 pos2;
				XMStoreFloat3(&pos1, invJoints[i].global_xform.r[3]);
				if (invJoints[i].parent_index != -1)
				{
					XMStoreFloat3(&pos2, invJoints[invJoints[i].parent_index].global_xform.r[3]);
					debug_renderer::add_line(pos1, pos2, (XMFLOAT4)DirectX::Colors::Azure);
				}
			}
		}
		void MatrixInit()
		{
			m_Lookat = m_Lookat * XMMatrixTranslation(-5,2,2);;
			m_Turnto = m_Turnto*XMMatrixTranslation(5,5,5);
			m_Matrix = m_Matrix * XMMatrixTranslation(0.5f,0,0);

			for (size_t i = 0; i < 5; i++)
			{
				float x = rand() % 20 - 10;
				float y = 0;
				float z = rand() % 20 - 10;
				myAABBs[i].max = XMVectorSet(x + rand()%3+1, y + rand()%3+1, z + rand()%3+1, 1);
				myAABBs[i].min = XMVectorSet(x, y, z, 1);
				myAABBs[i].color = (XMFLOAT4)DirectX::Colors::Aqua;
			}
			
		}
		XMMATRIX LookAt(XMVECTOR _lookerPos, XMVECTOR _target)
		{
			XMVECTOR zAxis = XMVector3Normalize(_target - _lookerPos);
			XMVECTOR xAxis = XMVector3Normalize(XMVector3Cross(DefaultUp, zAxis));
			XMVECTOR yAxis = XMVector3Normalize(XMVector3Cross(zAxis, xAxis));
			return XMMATRIX{ xAxis ,yAxis ,zAxis ,_lookerPos };
		}

		XMMATRIX TurnTo(XMMATRIX looker, XMVECTOR _target,float ratio)
		{
			XMVECTOR view = XMVector3Normalize(_target -looker.r[3]);
			XMVECTOR xAxis = XMVector3Normalize(looker.r[0]);
			XMVECTOR yAxis = XMVector3Normalize(looker.r[1]);

			float dotY = XMVectorGetX(XMVector3Dot(view, looker.r[0]));
			float dotX = XMVectorGetX(XMVector3Dot(view, looker.r[1]));
			XMMATRIX rotX = XMMatrixRotationX(-dotX*ratio);
			XMMATRIX rotY = XMMatrixRotationY(dotY*ratio);
			looker = XMMatrixMultiply(rotX,looker);
			looker = XMMatrixMultiply(rotY, looker);

			//Basically this is just look at but using exist Zaxis
			XMVECTOR newzAxis = looker.r[2];

			XMVECTOR newxAxis = XMVector3Normalize(XMVector3Cross(DefaultUp, newzAxis));
			XMVECTOR newyAxis = XMVector3Normalize(XMVector3Cross(newzAxis, newxAxis));

			return XMMATRIX{newxAxis ,newyAxis ,newzAxis ,looker.r[3] };
		}
		void UpdateMatrix()
		{
			float dtime = timer.Delta();

			DirectX::Mouse::State mouseState = m_pMouse->GetState();
			DirectX::Mouse::State lastMouseState = m_MouseTracker.GetLastState();
			int dx = mouseState.x - lastMouseState.x, dy = mouseState.y - lastMouseState.y;
			DirectX::Keyboard::State keyState = m_pKeyboard->GetState();
			DirectX::Keyboard::State lastKeyState = m_KeyboardTracker.GetLastState();


			m_MouseTracker.Update(mouseState);
		
			if ((mouseState.leftButton == true && m_MouseTracker.leftButton == m_MouseTracker.HELD)&&(lastMouseState.x != mouseState.x || lastMouseState.y != mouseState.y))
			{
					myCam.camYaw += (mouseState.x - lastMouseState.x) * 0.01f;

					myCam.camPitch += (mouseState.y - lastMouseState.y) * 0.01f;
			}
			
			if (keyState.IsKeyDown(Keyboard::W))
			{
				myCam.moveBackForward += dtime * cameraSpeed*5;
				//m_ViewMatrix = m_ViewMatrix * XMMatrixTranslation(0, 0, dtime * cameraSpeed * 10);
			}
			if (keyState.IsKeyDown(Keyboard::S))
			{
				myCam.moveBackForward -= dtime * cameraSpeed*5;
				//m_ViewMatrix = m_ViewMatrix * XMMatrixTranslation(0, 0, -dtime * cameraSpeed * 10);
			}
		
			if (keyState.IsKeyDown(Keyboard::A))
			{
				myCam.moveLeftRight -= dtime * cameraSpeed*5;
				//m_ViewMatrix = m_ViewMatrix * XMMatrixTranslation(-dtime * cameraSpeed*10, 0, 0);
			}
			if (keyState.IsKeyDown(Keyboard::D))
			{
				myCam.moveLeftRight += dtime * cameraSpeed*5;
				//m_ViewMatrix = m_ViewMatrix * XMMatrixTranslation(dtime * cameraSpeed * 10, 0, 0);
			}
			if (keyState.IsKeyDown(Keyboard::F1))
			{
				AnimCtrl = !AnimCtrl;
			}
			if (AnimCtrl)
			{
				if (keyState.IsKeyDown(Keyboard::J))
				{
					if(animStep<animClip.frames	.size()-1)
					animStep++;
				}
				if (keyState.IsKeyDown(Keyboard::K))
				{
					if (animStep >0)
					animStep--;
				}
			}
		
			XMMATRIX temp=myCam.GetMatrix();
			myCam.UpdateCamera();
			m_ViewMatrix=myCam.GetMatrix();
			/*calculate_frustum(myFrustum, m_Matrix);
			for (size_t i = 0; i < 5; i++)
			{
				if (aabb_to_frustum(myAABBs[i], myFrustum))
				{
					myAABBs[i].color = (XMFLOAT4)DirectX::Colors::YellowGreen;
				}
				else
				{
					myAABBs[i].color = (XMFLOAT4)DirectX::Colors::Aqua;
				}
				drawAABB(myAABBs[i]);
			}*/
		

			if (keyState.IsKeyDown(Keyboard::Up))
			{
				m_Matrix = m_Matrix * XMMatrixTranslation(0, 0, dtime * cameraSpeed * 10);
			}
			if (keyState.IsKeyDown(Keyboard::Down))
			{
				m_Matrix = m_Matrix * XMMatrixTranslation(0, 0, -dtime * cameraSpeed * 10);
			}
			if (keyState.IsKeyDown(Keyboard::Left))
			{
				m_Matrix = XMMatrixRotationY(-dtime * cameraSpeed*5)* m_Matrix;
			}
			if (keyState.IsKeyDown(Keyboard::Right))
			{
				m_Matrix =  XMMatrixRotationY(dtime*cameraSpeed*5)* m_Matrix;
			}


		}

		
		void Update()
		{
				UpdateGrid();
				UpdateMatrix();
				UpdateAnimation();
		}

		
	};
}