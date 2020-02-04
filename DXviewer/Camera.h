#include <DirectXMath.h>
using namespace DirectX;
#pragma once
class Camera
{
public:

	Camera();
	~Camera();

	XMMATRIX camRotationMatrix;
	XMMATRIX camMatrix;
	XMVECTOR camPosition;
	XMVECTOR camTarget;
	XMVECTOR camUp;
	XMVECTOR DefaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR DefaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR camForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR camRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float camYaw = 0.0f;
	float camPitch = 0.0f;

	void UpdateCamera();
	void Reset();
	XMMATRIX GetMatrix();
	void LoadViewMatrix(XMMATRIX &_view);
};

