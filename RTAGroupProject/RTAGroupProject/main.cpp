// D3D LAB 1a "Line Land".
// Author: L.Norri CD DRX, FullSail University

// Introduction:
// Welcome to the first lab of the Direct3D Graphics Programming class.
// This is the ONLY guided lab in this course! 
// Future labs will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this lab and start from the top.

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************

#include <iostream>
#include <ctime>
#include <d3d11.h>
#include "VertexShader.csh"
#include "PixelShader.csh"
#pragma comment (lib, "d3d11.lib")

using namespace std;

#define BACKBUFFER_WIDTH	500
#define BACKBUFFER_HEIGHT	500
//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
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

	struct SEND_TO_VRAM
	{
		float constantColor[4];
		float constantOffset[2];
		float padding[2];
	};
	SEND_TO_VRAM toShader;
	ID3D11InputLayout *InputLayout;

public:

	struct SIMPLE_VERTEX
	{
		float x, y;
	};
	SIMPLE_VERTEX vertexarr[360];
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
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
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Supersonic Acrobatic Rocket Powered Battlecars 2: Electric Boogaloo: Dawn of the Rise of the Rising Revengeance of the Brotherhood of the Dawn of the Final Day of the Reckoning of the 99 Dragons: The Presequel of Time and Space  LLC", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

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

#pragma region Constant Buffer Stuff
	ZeroMemory(&constbuffdesc, sizeof(constbuffdesc));

	constbuffdesc.Usage = D3D11_USAGE_DYNAMIC;
	constbuffdesc.ByteWidth = sizeof(SEND_TO_VRAM);
	constbuffdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constbuffdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	thedevice->CreateBuffer(&constbuffdesc, NULL, &constbuffer);
#pragma endregion
	int PixelShadersize = sizeof(PixelShader) / sizeof(PixelShader[0]);
	thedevice->CreatePixelShader(PixelShader, PixelShadersize, NULL, &pixelshader);
	int VertexShadersize = sizeof(VertexShader) / sizeof(VertexShader[0]);
	thedevice->CreateVertexShader(VertexShader, VertexShadersize, NULL, &vertexshader);
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	const float ColorRGBA[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

	thedevicecontext->OMSetRenderTargets(1, &RTV, NULL);
	thedevicecontext->RSSetViewports(1, &theVP);
	thedevicecontext->ClearRenderTargetView(RTV, ColorRGBA);


	theswaswaswapchain->Present(0, 0);

	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	theswaswaswapchain->Release();
	thedevice->Release();
	thedevicecontext->Release();
	RTV->Release();
	texture1->Release();
	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
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
//********************* END WARNING ************************//