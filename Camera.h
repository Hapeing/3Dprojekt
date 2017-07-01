#pragma once
#include <DirectXMath.h>
//#include <Mouse.h>

class Camera
{
private:
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT4 lookAt;
	DirectX::XMFLOAT4 up;
	float rot;

public:
	Camera();
	virtual ~Camera();

	void update(float dt, DirectX::XMMATRIX* world);
	DirectX::XMMATRIX getProjection();
	DirectX::XMMATRIX getView();

};