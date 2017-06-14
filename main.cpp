//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//--------------------------------------------------------------------------------------
#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Inc\SimpleMath.h"
#include "bth_image.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateDirect3DContext(HWND wndHandle);

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gDeviceContext = nullptr;
ID3D11RenderTargetView* gBackbufferRTV = nullptr;

ID3D11Buffer* gVertexBuffer = nullptr;
ID3D11Buffer* gIndexBuffer = nullptr;
ID3D11Buffer* gConstantBuffer = nullptr;

ID3D11InputLayout* gVertexLayout = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;
ID3D11GeometryShader* gGeometryShader = nullptr;

ID3D11SamplerState* sampleState = nullptr;
ID3D11ShaderResourceView* textureView = nullptr;
ID3D11Texture2D* texture = nullptr;

ID3D11DepthStencilView* DSView = nullptr;
ID3D11Texture2D* depthTexture = nullptr;

float rotation = 0;

struct TriangleVertex
{
	float x, y, z;
	//float r, g, b;
	float u, v;
};

TriangleVertex triangleVertices[4] =
{
	//-0.5f, -0.5f, 0.0f, //v0 bot left
	//0.0f, 1.0f,	//v0 texture

	//-0.5f, 0.5f, 0.0f,	//v1 top left
	//0.0f, 0.0f,	//v1 texture

	//0.5f, -0.5f, 0.0f,	//v2 bot right
	//1.0f, 1.0f,	//v2 texture

	//0.5f, 0.5f, 0.0f,	//v3 top right
	//1.0f, 0.0f	//v3 texture

	-0.0f, -0.5f, -0.5f, //v0 bot left
	0.0f, 1.0f,	//v0 texture

	0.0f, 0.5f, -0.5f,	//v1 top left
	0.0f, 0.0f,	//v1 texture

	0.0f, -0.5f, 0.5f,	//v2 bot right
	1.0f, 1.0f,	//v2 texture

	0.0f, 0.5f, 0.5f,	//v3 top right
	1.0f, 0.0f	//v3 texture
};

struct VS_CONSTANT_BUFFER
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 proj;
} VsData;

void CreateShaders()
{
	//create vertex shader
	ID3DBlob* pVS = nullptr;
	D3DCompileFromFile(
		L"Vertex.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"VS_main",		// entry point
		"vs_5_0",		// shader model (target)
		0,				// shader compile options			// here DEBUGGING OPTIONS
		0,				// effect compile options
		&pVS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		// how to use the Error blob, see here
		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);


	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	gDevice->CreateInputLayout(inputDesc, ARRAYSIZE(inputDesc), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gVertexLayout);
	// we do not need anymore this COM object, so we release it.
	pVS->Release();

	//create pixel shader
	ID3DBlob* pPS = nullptr;
	D3DCompileFromFile(
		L"Fragment.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"PS_main",		// entry point
		"ps_5_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pPS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		// how to use the Error blob, see here
		// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
		);

	gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);
	// we do not need anymore this COM object, so we release it.
	pPS->Release();



	//create geometry shader
	ID3DBlob* pGS = nullptr;
	D3DCompileFromFile(
		L"GeometryShader.hlsl", // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		"main",		// entry point
		"gs_5_0",		// shader model (target)
		0,				// shader compile options
		0,				// effect compile options
		&pGS,			// double pointer to ID3DBlob		
		nullptr			// pointer for Error Blob messages.
		);

	gDevice->CreateGeometryShader(pGS->GetBufferPointer(), pGS->GetBufferSize(), nullptr, &gGeometryShader);
	// we do not need anymore this COM object, so we release it.
	pGS->Release();

	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = BTH_IMAGE_WIDTH;
	texDesc.Height = BTH_IMAGE_HEIGHT;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.MiscFlags = 0;
	texDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* texture = nullptr;
	D3D11_SUBRESOURCE_DATA texData;
	ZeroMemory(&texData, sizeof(texData));
	texData.pSysMem = (void*)BTH_IMAGE_DATA;
	texData.SysMemPitch = BTH_IMAGE_WIDTH * 4 * sizeof(char);
	gDevice->CreateTexture2D(&texDesc, &texData, &texture);

	D3D11_SHADER_RESOURCE_VIEW_DESC resViewDesc;
	ZeroMemory(&resViewDesc, sizeof(resViewDesc));
	resViewDesc.Format = texDesc.Format;
	resViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resViewDesc.Texture2D.MipLevels = texDesc.MipLevels;
	resViewDesc.Texture2D.MostDetailedMip = 0;
	gDevice->CreateShaderResourceView(texture, &resViewDesc, &textureView);

	texture->Release();


	//Depth buffer

	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
	depthTexDesc.Width = 640.0f;
	depthTexDesc.Height = 480.0f;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.MiscFlags = 0;

	 HRESULT hr = gDevice->CreateTexture2D(&depthTexDesc, nullptr, &depthTexture);
	//gDeviceContext->OMSetRenderTargets(0, nullptr, DSView);
	

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(dsvDesc));
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	hr = gDevice->CreateDepthStencilView(depthTexture, &dsvDesc, &DSView);
	

	//constant buffer
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(VsData);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &VsData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	gDevice->CreateBuffer(&cbDesc, &InitData, &gConstantBuffer);
	gDeviceContext->GSSetConstantBuffers(0, 1, &gConstantBuffer);
}

void CreateTriangleData()
{
	


	//unsigned int indexVertecies[6]{ 0,1,2, 3,3,1};

	/*D3D11_BUFFER_DESC vertexBufferDesc;
	memset(&vertexBufferDesc, 0, sizeof(vertexBufferDesc));
	vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(indexVertecies);

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indexVertecies;
	gDevice->CreateBuffer(&vertexBufferDesc, &indexData, &gIndexBuffer);*/

}

void SetViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)640;
	vp.Height = (float)480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

void Render()
{
	gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, DSView);


	

	D3D11_MAPPED_SUBRESOURCE WVPdata;
	memset(&WVPdata, 0, sizeof(WVPdata));
	gDeviceContext->Map(gConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &WVPdata);
	memcpy(WVPdata.pData, &VsData, sizeof(VsData));


	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	//how to bind the buffer
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;				//how to use the data
	bufferDesc.ByteWidth = sizeof(triangleVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);

	rotation += 0.001;

	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();
	
	DirectX::XMVECTOR cameraPos;
	DirectX::XMVECTOR cameraLookAt;
	DirectX::XMVECTOR cameraUp;
	//setting cameraPos
	cameraPos = DirectX::XMVectorSet(0, 0, -2, 1);

	//setting lookAt vector
	cameraLookAt = DirectX::XMVectorSet(0, 0, 0, 1);

	//setting up vector
	cameraUp = DirectX::XMVectorSet(0, 1, 0, 0);

	//setting up view matrix
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cameraPos, cameraLookAt, cameraUp);

	//setting up projection matrix
	DirectX::XMMATRIX mProjection = DirectX::XMMatrixPerspectiveFovLH(3.14f*0.45f, 640.0f / 480.0f, 0.1f, 20.0f);

	//setting matrecies to the constant buffer
	DirectX::XMStoreFloat4x4(&VsData.world, DirectX::XMMatrixTranspose(mWorld *  DirectX::XMMatrixRotationY(rotation)));
	DirectX::XMStoreFloat4x4(&VsData.view, DirectX::XMMatrixTranspose(mView));
	DirectX::XMStoreFloat4x4(&VsData.proj, DirectX::XMMatrixTranspose(mProjection));
	



	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 1.0f, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV, clearColor);
	gDeviceContext->ClearDepthStencilView(DSView, D3D11_CLEAR_DEPTH, 1.0f, 0.0f);

	gDeviceContext->VSSetShader(gVertexShader, nullptr, 0);
	gDeviceContext->HSSetShader(nullptr, nullptr, 0);
	gDeviceContext->DSSetShader(nullptr, nullptr, 0);
	gDeviceContext->GSSetShader(gGeometryShader, nullptr, 0);
	gDeviceContext->PSSetShader(gPixelShader, nullptr, 0);

	UINT32 vertexSize = sizeof(float) * 5;
	UINT32 offset = 0;
	gDeviceContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vertexSize, &offset);

	gDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP); //how the information shuld be rendered
	gDeviceContext->IASetInputLayout(gVertexLayout);	//bind a input layout to the assembler stage
	gDeviceContext->PSSetShaderResources(0, 1, &textureView);

	gDeviceContext->Draw(4, 0);
}

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance); //1. Skapa fönster
	
	if (wndHandle)
	{
		CreateDirect3DContext(wndHandle); //2. Skapa och koppla SwapChain, Device och Device Context

		SetViewport(); //3. Sätt viewport

		CreateShaders(); //4. Skapa vertex- och pixel-shaders

		CreateTriangleData(); //5. Definiera triangelvertiser, 6. Skapa vertex buffer, 7. Skapa input layout

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				Render(); //8. Rendera

				gSwapChain->Present(0, 0); //9. Växla front- och back-buffer
			}
		}

		gVertexBuffer->Release();
		//gIndexBuffer->Release();

		gVertexLayout->Release();
		gVertexShader->Release();
		gPixelShader->Release();
		gGeometryShader->Release();

		//sampleState->Release();

		gBackbufferRTV->Release();
		gSwapChain->Release();
		gDevice->Release();
		gDeviceContext->Release();
		DestroyWindow(wndHandle);
	}

	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HRESULT CreateDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multisamples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();
	}
	return hr;
}