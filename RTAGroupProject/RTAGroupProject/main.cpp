
#include <iostream>
#include <ctime>
#include <d3d11.h>
#include "VertexShader.csh"
#include "PixelShader.csh"
#pragma comment (lib, "d3d11.lib")
#define SAFE_RELEASE(p){if(p && p != nullptr){p->Release(); p = nullptr;}}
using namespace std;

#define BACKBUFFER_WIDTH	1600
#define BACKBUFFER_HEIGHT	900

class RTA_PROJECT
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	ID3D11Texture2D *texture1;
	ID3D11Device *thedevice;
	ID3D11DeviceContext *thedevicecontext;
	IDXGISwapChain *theswaswaswapchain;
	ID3D11RenderTargetView *RTV;
	D3D11_VIEWPORT theVP;
	ID3D11Buffer *buffer;
	unsigned int numverts = 0;
	D3D11_BUFFER_DESC buffdesc;
	ID3D11Buffer *constbuffer;
	D3D11_BUFFER_DESC constbuffdesc;

	ID3D11VertexShader *vertexshader;
	ID3D11PixelShader *pixelshader;

	ID3D11InputLayout *InputLayout;

public:

	struct SIMPLE_VERTEX
	{
		float x, y;
	};
	RTA_PROJECT(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

RTA_PROJECT::RTA_PROJECT(HINSTANCE hinst, WNDPROC proc)
{
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Supersonic Acrobatic Rocket Powered Battlecars 2: Electric Boogaloo: Dawn of the Rise of the Rising Revengeance of the Brotherhood of the Dawn of the Final Day of the Reckoning of the 99 Dragons: The Presequel of Time and Space  LLC", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

#pragma region Device and Swapchain Stuff
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

	ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.OutputWindow = window;
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.Windowed = TRUE;

	if (_DEBUG)
	{
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDesc, &theswaswaswapchain, &thedevice, NULL, &thedevicecontext);
	}
	else
	{
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDesc, &theswaswaswapchain, &thedevice, NULL, &thedevicecontext);
	}
#pragma endregion

#pragma region BufferStuff
	theswaswaswapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&texture1);

	thedevice->CreateRenderTargetView(texture1, NULL, &RTV);

	ZeroMemory(&theVP, sizeof(D3D11_VIEWPORT));
	theVP.TopLeftX = 0;
	theVP.TopLeftY = 0;
	theVP.Width = BACKBUFFER_WIDTH;
	theVP.Height = BACKBUFFER_HEIGHT;
	theVP.MinDepth = 0;
	theVP.MaxDepth = 1;

#pragma endregion

	int PixelShadersize = sizeof(PixelShader) / sizeof(PixelShader[0]);
	thedevice->CreatePixelShader(PixelShader, PixelShadersize, NULL, &pixelshader);
	int VertexShadersize = sizeof(VertexShader) / sizeof(VertexShader[0]);
	thedevice->CreateVertexShader(VertexShader, VertexShadersize, NULL, &vertexshader);
}

bool RTA_PROJECT::Run()
{
	const float ColorRGBA[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	thedevicecontext->OMSetRenderTargets(1, &RTV, NULL);
	thedevicecontext->RSSetViewports(1, &theVP);
	thedevicecontext->ClearRenderTargetView(RTV, ColorRGBA);


	theswaswaswapchain->Present(0, 0);

	return true;
}

bool RTA_PROJECT::ShutDown()
{
	//SAFE_RELEASE(buffer);
	SAFE_RELEASE(theswaswaswapchain);
	SAFE_RELEASE(thedevice);
	SAFE_RELEASE(thedevicecontext);
	SAFE_RELEASE(RTV);
	SAFE_RELEASE(texture1);
	SAFE_RELEASE(vertexshader);
	SAFE_RELEASE(pixelshader);
//	SAFE_RELEASE(constbuffer);
	//SAFE_RELEASE(InputLayout);
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	RTA_PROJECT myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
