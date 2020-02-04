#include "frustum_culling.h"

using namespace end;


plane_t end::calculate_plane(XMVECTOR a, XMVECTOR b, XMVECTOR c, XMVECTOR d)
{
	plane_t plane;
	plane.normal = XMVector3Normalize(XMVector3Cross(b - a, c - b));
	plane.offset =XMVectorGetX(XMVector3Dot(plane.normal, a));
	XMVECTOR planeCenter = (a + b + c + d) / 4.0f;
	XMFLOAT3 point1;
	XMFLOAT3 point2;

	XMStoreFloat3(&point1, planeCenter);
	XMStoreFloat3(&point2, (plane.normal * 0.33f) + planeCenter);

	debug_renderer::add_line(point1, point2,XMFLOAT4(DirectX::Colors::AliceBlue), XMFLOAT4(DirectX::Colors::Cyan));
	return plane;
}

void end::calculate_frustum(frustum_t& frustum, const XMMATRIX& view)
{

	float fov=15.0f;
	float nearDist=0.5f;
	float farDist=10.0f;
	float fHalfHeightN =2*	tanf(XMConvertToRadians(fov*0.5f))*nearDist;
	float fHalfHeightF = 2 * tanf(XMConvertToRadians(fov * 0.5f)) * farDist;
	float fHalfWidthN = fHalfHeightN * 16 / 9.0f;
	float fHalfWidthF = fHalfHeightF * 16 / 9.0f;
	XMVECTOR nCenter = view.r[3] + view.r[2] * 1;
	XMVECTOR fCenter = view.r[3] + view.r[2] * 10.0f;

	// near frustrum corners
	XMFLOAT3 nTL; XMStoreFloat3(&nTL, nCenter - (view.r[0] * fHalfWidthN) + (view.r[1] * fHalfHeightN));
	XMFLOAT3 nTR; XMStoreFloat3(&nTR, nCenter + (view.r[0] * fHalfWidthN) + (view.r[1] * fHalfHeightN));
	XMFLOAT3 nBL; XMStoreFloat3(&nBL, nCenter - (view.r[0] * fHalfWidthN) - (view.r[1] * fHalfHeightN));
	XMFLOAT3 nBR; XMStoreFloat3(&nBR, nCenter + (view.r[0] * fHalfWidthN) - (view.r[1] * fHalfHeightN));

	// far frustrum corners
	XMFLOAT3 fTL; XMStoreFloat3(&fTL, fCenter - (view.r[0] * fHalfWidthF) + (view.r[1] * fHalfHeightF));
	XMFLOAT3 fTR; XMStoreFloat3(&fTR, fCenter + (view.r[0] * fHalfWidthF) + (view.r[1] * fHalfHeightF));
	XMFLOAT3 fBL; XMStoreFloat3(&fBL, fCenter - (view.r[0] * fHalfWidthF) - (view.r[1] * fHalfHeightF));
	XMFLOAT3 fBR; XMStoreFloat3(&fBR, fCenter + (view.r[0] * fHalfWidthF) - (view.r[1] * fHalfHeightF));

	// left plane
	frustum[0] = calculate_plane( XMLoadFloat3(&fBL), XMLoadFloat3(&nBL), XMLoadFloat3(&fTL), XMLoadFloat3(&nTL));
	// right plane
	frustum[1] = calculate_plane(XMLoadFloat3(&nBR), XMLoadFloat3(&fBR),  XMLoadFloat3(&nTR), XMLoadFloat3(&fTR));
	// bottom plane
	frustum[2] = calculate_plane(XMLoadFloat3(&nBR), XMLoadFloat3(&nBL), XMLoadFloat3(&fBR), XMLoadFloat3(&fBL));
	// top plane
	frustum[3] = calculate_plane(XMLoadFloat3(&fTR), XMLoadFloat3(&fTL),  XMLoadFloat3(&nTR), XMLoadFloat3(&nTL));
	// far plane
	frustum[4] = calculate_plane(XMLoadFloat3(&fBR), XMLoadFloat3(&fBL), XMLoadFloat3(&fTR), XMLoadFloat3(&fTL));
	// near plane
	frustum[5] = calculate_plane(XMLoadFloat3(&nBL), XMLoadFloat3(&nBR), XMLoadFloat3(&nTL), XMLoadFloat3(&nTR));


	XMFLOAT4 Color =(XMFLOAT4)DirectX::Colors::Red;
	
	// near square bl, br, tr, tl bl
	debug_renderer::add_line(nBL, nBR, Color);
	debug_renderer::add_line(nBR, nTR, Color);
	debug_renderer::add_line(nTR, nTL, Color);
	debug_renderer::add_line(nTL, nBL, Color);
	// far square bl br tr tl bl
	debug_renderer::add_line(fBL, fBR, Color);
	debug_renderer::add_line(fBR, fTR, Color);
	debug_renderer::add_line(fTR, fTL, Color);
	debug_renderer::add_line(fTL, fBL, Color);
	// right sqaure nbr ntr ftr fbr nbr			   
	debug_renderer::add_line(nBR, nTR, Color);
	debug_renderer::add_line(nTR, fTR, Color);
	debug_renderer::add_line(fTR, fBR, Color);
	debug_renderer::add_line(fBR, nBR, Color);
	// left sqaure nbr ntr ftr fbr nbr			   
	debug_renderer::add_line(nBL, nTL, Color);
	debug_renderer::add_line(nTL, fTL, Color);
	debug_renderer::add_line(fTL, fBL, Color);
	debug_renderer::add_line(fBL, nBL, Color);

}

int end::classify_sphere_to_plane(const sphere_t& sphere, const plane_t& plane)
{
	float dis = XMVectorGetX(XMVector3Dot(sphere.center, plane.normal)) - plane.offset;
	if (dis > sphere.radius)
		return 1;
	else if (dis < -sphere.radius)
		return -1;
	else
		return 0;
}

int end::classify_aabb_to_plane(const aabb_t& aabb, const plane_t& plane)
{
	XMFLOAT3 normal;
	XMFLOAT3 extent;
	XMStoreFloat3(&normal, plane.normal);
	XMStoreFloat3(&extent, (aabb.max - aabb.min) / 2.0f);
	float radius = extent.x * abs(normal.x) + extent.y * abs(normal.y) + extent.z * abs(normal.z);
	sphere_t sphere;
	sphere.center = (aabb.min + aabb.max) / 2.0f;
	sphere.radius = radius;
	return classify_sphere_to_plane(sphere, plane);
}

bool end::aabb_to_frustum(const aabb_t& aabb, const frustum_t& frustum)
{
	for (int i = 0; i < 6; i++)
	{
		if (classify_aabb_to_plane(aabb, frustum[i]) == -1)
			return false;
	}
	return true;
}

void end::drawAABB(aabb_t cube)
{
	XMVECTOR cubes[8];
	cubes[0] = cube.min;
	cubes[7] = cube.max;
	cubes[1] = cubes[0];
	cubes[1].m128_f32[0] = XMVectorGetX(cubes[7]);
	cubes[2] = cubes[1]; 
	cubes[2].m128_f32[2] =XMVectorGetZ(cubes[7]);
	cubes[3] = cubes[0];
	cubes[3].m128_f32[2] = XMVectorGetZ(cubes[7]);

	cubes[4] = cubes[1];
	cubes[4].m128_f32[1] = XMVectorGetY(cubes[7]);
	cubes[5] = cubes[0];
	cubes[5].m128_f32[1] = XMVectorGetY(cubes[7]);
	cubes[6] = cubes[3];
	cubes[6].m128_f32[1] = XMVectorGetY(cubes[7]);

	XMFLOAT3 One, Two, Thr, Fou, Fiv, Six, Sev, Eig;
	XMStoreFloat3(&One, cubes[0]); XMStoreFloat3(&Two, cubes[1]); XMStoreFloat3(&Thr, cubes[2]); XMStoreFloat3(&Fou, cubes[3]);
	XMStoreFloat3(&Fiv, cubes[4]); XMStoreFloat3(&Six, cubes[5]); XMStoreFloat3(&Sev, cubes[6]); XMStoreFloat3(&Eig, cubes[7]);
	debug_renderer::add_line(One, Two, cube.color);	debug_renderer::add_line(One, Fou, cube.color); debug_renderer::add_line(One, Six, cube.color);	debug_renderer::add_line(Two, Thr, cube.color);
	debug_renderer::add_line(Two, Fiv, cube.color); debug_renderer::add_line(Thr, Fou, cube.color); debug_renderer::add_line(Thr, Eig, cube.color);	debug_renderer::add_line(Fou, Sev, cube.color);
	debug_renderer::add_line(Fiv, Six, cube.color); debug_renderer::add_line(Fiv, Eig, cube.color); debug_renderer::add_line(Six, Sev, cube.color); debug_renderer::add_line(Sev, Eig, cube.color);
}