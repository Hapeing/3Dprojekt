#include "Camera.h"


Camera::Camera()
	:pos(0,0,-2,1), lookAt(0,0,0,1), up(0,1,0,0), rot(0.f)
{
}

Camera::~Camera()
{
}

void Camera::update(float dt, DirectX::XMMATRIX* world)
{

	rot += 0.001f*dt;
	*world = DirectX::XMMatrixMultiply(*world, DirectX::XMMatrixRotationY(this->rot));

}

DirectX::XMMATRIX Camera::getProjection()
{
	return DirectX::XMMatrixPerspectiveFovLH(3.14f*0.45f, 640.0f / 480.0f, 0.1f, 20.0f);
}

DirectX::XMMATRIX Camera::getView()
{
	DirectX::FXMVECTOR p = DirectX::XMVectorSet(pos.x, pos.y, pos.z, pos.w);
	DirectX::FXMVECTOR l = DirectX::XMVectorSet(lookAt.x, lookAt.y, lookAt.z, lookAt.w);
	DirectX::FXMVECTOR u = DirectX::XMVectorSet(up.x, up.y, up.z, up.w);

	return DirectX::XMMatrixLookAtLH(p, l, u);
}
