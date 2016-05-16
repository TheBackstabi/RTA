
#include <iostream>
#include <fstream>
#include <ctime>
#include <d3d11.h>
#include <vector>
#include <DirectXMath.h>
#include <dxgi1_2.h>
#include "MathStuffh.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "VertexShader.csh"
#include "PixelShader.csh"
#include <fbxsdk.h>
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib") 
#define SAFE_RELEASE(p){if(p && p != nullptr){p->Release(); p = nullptr;}}
using namespace std;
using namespace DirectX;

#define BACKBUFFER_WIDTH	1600
#define BACKBUFFER_HEIGHT	900
class RTAPROJECT
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	IDXGISwapChain *sc;
	ID3D11DeviceContext *thedevicecontext;
	ID3D11Device *thedevice;
	ID3D11RenderTargetView *RenTarView;
	ID3D11Buffer *buffer;
	ID3D11Buffer *buffertempobj;

	unsigned int numverts = 0;
	D3D11_BUFFER_DESC buffdesc;

	ID3D11DepthStencilView* pDSV;
	DXGI_SWAP_CHAIN_DESC SwapChainDescVar;
	ID3D11Texture2D* pDepthStencil = NULL;
	D3D11_TEXTURE2D_DESC texdepth;
	D3D11_DEPTH_STENCIL_VIEW_DESC texdsv;
	D3D11_VIEWPORT vp;
	ID3D11Texture2D* thetexture;
	ID3D11ShaderResourceView *floorRSV[2];
	ID3D11Buffer *constbuffer;
	D3D11_BUFFER_DESC constbuffdesc;
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ID3D11DepthStencilState * pDSState;
	ID3D11Buffer *constbuffer2;
	D3D11_BUFFER_DESC constbuffdesc2;
	ID3D11Buffer *Indexbuffer;
	D3D11_BUFFER_DESC Indexbufferdesc;
	ID3D11DepthStencilState* depthstenstate;
	float translateparam = 0.0f;
	ID3D11VertexShader *vertshader;
	ID3D11PixelShader *pixshader;
	POINT mousePos;
	FbxManager* fbxManager = nullptr;
public:

	struct MyVertex
	{
		float pos[3];
	};
	struct MyUV
	{
		float uv[2];
	};
	struct MyNormal
	{
		float normal[3];
	};
	vector<MyVertex> vertexvec;
	vector<MyNormal> normalvec;
	vector<MyUV> uvvec;
	vector<unsigned int> indiciesvec;
	SEND_TO_VRAM toShader;
	ID3D11InputLayout *InputLayout;
	SEND_TO_OBJECT objecttoObject;
	SEND_TO_SCENE objecttoScene;
	unsigned short Indicarr[1692];
	D3D11_MAPPED_SUBRESOURCE Mapsubres;
	D3D11_MAPPED_SUBRESOURCE Mapsubres2;
	vector<tVertex> tempverts;
	vector<unsigned int> temptriangle;

	RTAPROJECT(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	HRESULT LoadFBX(string filePath, vector<MyVertex>& pOutVertexVector, vector<MyNormal>& pOutNormalVector, vector<MyUV>& pOutUVector, string& filepath);

	void WritetoBinary(string filename, vector<MyVertex>& pOutVertexVector, vector<MyNormal>& pOutNormalVector, vector<MyUV>& pOutUVector, string& filepath);

	void readfromRTAmesh(string filename, vector<MyVertex>& pinVertexVector, vector<MyNormal>& pinNormalVector, vector<MyUV>& pinUVector, string& filepath);

	void Loadfile(string filename, vector<MyVertex>& pinVertexVector, vector<MyNormal>& pinNormalVector, vector<MyUV>& pinUVector, string& filepath);
};

HRESULT RTAPROJECT::LoadFBX(string filePath, vector<MyVertex>& pOutVertexVector, vector<MyNormal>& pOutNormalVector, vector<MyUV>& pOutUVector, string& filepath)
{
	if (fbxManager == nullptr)
	{
		fbxManager = FbxManager::Create();

		FbxIOSettings* pIOsettings = FbxIOSettings::Create(fbxManager, IOSROOT);
		fbxManager->SetIOSettings(pIOsettings);
	}

	FbxImporter* pImporter = FbxImporter::Create(fbxManager, "");
	FbxScene* pFbxScene = FbxScene::Create(fbxManager, "");

	bool bSuccess = pImporter->Initialize(filePath.c_str(), -1, fbxManager->GetIOSettings());
	if (!bSuccess) return E_FAIL;

	bSuccess = pImporter->Import(pFbxScene);
	if (!bSuccess) return E_FAIL;

	pImporter->Destroy();

	FbxNode* pFbxRootNode = pFbxScene->GetRootNode();

	if (pFbxRootNode)
	{
		int childcount = pFbxRootNode->GetChildCount();
		for (int i = 0; i < childcount; i++)
		{
			FbxNode* pFbxChildNode = pFbxRootNode->GetChild(i);

			if (pFbxChildNode->GetNodeAttribute() == NULL)
				continue;

			FbxNodeAttribute::EType AttributeType = pFbxChildNode->GetNodeAttribute()->GetAttributeType();

			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			FbxMesh* pMesh = (FbxMesh*)pFbxChildNode->GetNodeAttribute();
			
			FbxVector4* pVertices = pMesh->GetControlPoints();
			bool unmapped;
			for (int j = 0; j < pMesh->GetPolygonCount(); j++)
			{
				int iNumVertices = pMesh->GetPolygonSize(j);
				assert(iNumVertices == 3);

				for (int k = 0; k < iNumVertices; k++)
				{
					int iControlPointIndex = pMesh->GetPolygonVertex(j, k);


					MyUV uv;
					FbxVector2 uvCoords;
					FbxStringList uvStringList;
					pMesh->GetUVSetNames(uvStringList);

					pMesh->GetPolygonVertexUV(j, k, uvStringList.GetStringAt(0), uvCoords, unmapped);
					uv.uv[0] = static_cast<float>(uvCoords[0]);
					uv.uv[1] = static_cast<float>(uvCoords[1]);
					uv.uv[1] = 1 - uv.uv[1];
					pOutUVector.push_back(uv);

					MyVertex vertex;
					vertex.pos[0] = (float)pVertices[iControlPointIndex].mData[0];
					vertex.pos[1] = (float)pVertices[iControlPointIndex].mData[1];
					vertex.pos[2] = (float)pVertices[iControlPointIndex].mData[2];
					pOutVertexVector.push_back(vertex);


					MyNormal norm;
					FbxVector4 fbxNormal;
					bool result = pMesh->GetPolygonVertexNormal(j, k, fbxNormal);
					norm.normal[0] = fbxNormal.mData[0];
					norm.normal[1] = fbxNormal.mData[1];
					norm.normal[2] = fbxNormal.mData[2];

					pOutNormalVector.push_back(norm);


				}
			}

			int mcount = pFbxChildNode->GetSrcObjectCount<FbxSurfaceMaterial>();
			for (int index = 0; index < mcount; index++)
			{
				FbxSurfaceMaterial *material = (FbxSurfaceMaterial*)pFbxChildNode->GetSrcObject<FbxSurfaceMaterial>(index);
				if (material)
				{
					FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

					// Directly get textures
					int texture_count = prop.GetSrcObjectCount<FbxTexture>();
					for (int j = 0; j < texture_count; j++)
					{
						const FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxFileTexture>(j));
						// Then, you can get all the properties of the texture, include its name
						
						const char* texture_name = FbxCast<FbxFileTexture>(texture)->GetFileName();

						filepath = texture_name;
					}

				}
			}

		}

	}
	return S_OK;
}

void loadfromfile(string filepath, RTAPROJECT* tempthis)
{
	fstream file;


	file.open(filepath, ios_base::binary | ios_base::in);
	if (file.is_open())
	{
		unsigned int verlen;

		file.read((char *)&verlen, 4);

		tempthis->tempverts.resize(verlen);

		for (unsigned int i = 0; i < verlen; i++)
		{
			file.read((char*)&tempthis->tempverts[i], sizeof(tVertex));
		}
		unsigned int tlen;
		file.read((char *)&tlen, 4);

		tempthis->temptriangle.resize(tlen * 3);
		for (unsigned int i = 0; i < tempthis->temptriangle.size(); i++)
		{
			file.read((char *)&tempthis->temptriangle[i], sizeof(unsigned int));
		}


		file.close();
	}
}

void RTAPROJECT::WritetoBinary(string fbxfilenamenoextension, vector<MyVertex>& pinVertexVector, vector<MyNormal>& pinNormalVector, vector<MyUV>& pinUVector, string& filepath)
{
	fstream file;


	file.open(fbxfilenamenoextension + ".RTAmesh", ios_base::binary | ios_base::out);
	if (file.is_open())
	{
		unsigned int strlen = filepath.size();
		file.write((char*)&strlen, sizeof(unsigned int));

		file.write(&filepath[0], strlen);
		unsigned int vertexvecsize = pinVertexVector.size();
		file.write((char*)&vertexvecsize, sizeof(unsigned int));

		for (unsigned int i = 0; i < pinVertexVector.size(); i++)
		{
			file.write((char*)&pinVertexVector[i], sizeof(MyVertex));
		}

		unsigned int normalvecsize = pinNormalVector.size();
		file.write((char*)&normalvecsize, sizeof(unsigned int));

		for (unsigned int i = 0; i < pinNormalVector.size(); i++)
		{
			file.write((char*)&pinNormalVector[i], sizeof(MyNormal));
		}

		unsigned int uvvvecsize = pinUVector.size();
		file.write((char*)&uvvvecsize, sizeof(unsigned int));

		for (unsigned int i = 0; i < pinUVector.size(); i++)
		{
			file.write((char*)&pinUVector[i], sizeof(MyUV));
		}
	}
}

void RTAPROJECT::readfromRTAmesh(string filename, vector<MyVertex>& pinVertexVector, vector<MyNormal>& pinNormalVector, vector<MyUV>& pinUVector, string& filepath)
{
	fstream file;


	file.open(filename + ".RTAmesh", ios_base::binary | ios_base::in);
	if (file.is_open())
	{
		unsigned int strlen = 0;
		file.read((char*)&strlen, sizeof(unsigned int));
		filepath.resize(strlen);
		file.read(&filepath[0], strlen);
		unsigned int vertexvecsize = 0;
		file.read((char*)&vertexvecsize, sizeof(unsigned int));

		pinVertexVector.resize(vertexvecsize);
		for (unsigned int i = 0; i < vertexvecsize; i++)
		{
			file.read((char*)&pinVertexVector[i], sizeof(MyVertex));
		}

		unsigned int normalvecsize = 0;
		file.read((char*)&normalvecsize, sizeof(unsigned int));
		pinNormalVector.resize(normalvecsize);
		for (unsigned int i = 0; i < pinNormalVector.size(); i++)
		{
			file.read((char*)&pinNormalVector[i], sizeof(MyNormal));
		}

		unsigned int uvvvecsize = 0;
		file.read((char*)&uvvvecsize, sizeof(unsigned int));
		pinUVector.resize(uvvvecsize);
		for (unsigned int i = 0; i < pinUVector.size(); i++)
		{
			file.read((char*)&pinUVector[i], sizeof(MyUV));
		}
	}
}

void RTAPROJECT::Loadfile(string filenamenoextension, vector<MyVertex>& pinVertexVector, vector<MyNormal>& pinNormalVector, vector<MyUV>& pinUVector, string& filepath)
{
	fstream file;


	file.open(filenamenoextension + ".RTAmesh", ios_base::binary | ios_base::in);
	if (file.is_open())
	{
		readfromRTAmesh(filenamenoextension, pinVertexVector, pinNormalVector, pinUVector, filepath);
	}
	else
	{
		string fullfilename = filenamenoextension + ".fbx";

		LoadFBX(fullfilename, pinVertexVector, pinNormalVector, pinUVector, filepath);

		WritetoBinary(filenamenoextension, pinVertexVector, pinNormalVector, pinUVector, filepath);
	}
}
RTAPROJECT::RTAPROJECT(HINSTANCE hinst, WNDPROC proc)
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

	window = CreateWindow(L"DirectXApplication", L"Supersonic Acrobatic Rocket Powered Battlecars 2: Electric Boogaloo: Dawn of the Rise of the Rising Revengeance of the Brotherhood of the Dawn of the Final Day of the Reckoning of the 99 Dragons: The Presequel of Time and Space  LLC Project", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

	objecttoScene.ViewMatrix = Identity();
	objecttoObject.WorldMat = DirectXWorldMat();

	ZeroMemory(&SwapChainDescVar, sizeof(DXGI_SWAP_CHAIN_DESC));

	SwapChainDescVar.BufferCount = 1;
	SwapChainDescVar.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDescVar.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDescVar.OutputWindow = window;
	SwapChainDescVar.SampleDesc.Count = 4;
	SwapChainDescVar.Windowed = TRUE;
	if (_DEBUG)
	{
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDescVar, &sc, &thedevice, NULL, &thedevicecontext);
	}
	else
	{
		D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDescVar, &sc, &thedevice, NULL, &thedevicecontext);
	}
	sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&thetexture);

	thedevice->CreateRenderTargetView(thetexture, NULL, &RenTarView);

	thetexture->Release();

	sc->GetDesc(&SwapChainDescVar);

	string thepath;
	
	Loadfile("Teddy_Idle", vertexvec, normalvec, uvvec, thepath);

	wstring tempstring(thepath.begin(), thepath.end());
	const wchar_t* szName = tempstring.c_str();
	//Use CreateWICTextureFromFile for anything that isn't a .dds
	//Use CreateDDSTextureFromFile for .dds files (duh)
	CreateWICTextureFromFile(thedevice, szName, NULL, &floorRSV[0]);
	//CreateDDSTextureFromFile(thedevice, szName, NULL, &floorRSV[0]);
#pragma region .MESH fileloading
	//loadfromfile("Arwing_002.mesh", this);
	//size_t size = tempverts.size();
	//SIMPLE_VERTEX* loadedvertexes = new SIMPLE_VERTEX[size];
	//for (unsigned int i = 0; i < tempverts.size(); i++)
	//{
	//	loadedvertexes[i].x = tempverts[i].fX;
	//	loadedvertexes[i].y = tempverts[i].fY;
	//	loadedvertexes[i].z = tempverts[i].fZ;
	//	loadedvertexes[i].w = 1;
	//	loadedvertexes[i].n = tempverts[i].fNX;
	//	loadedvertexes[i].r = tempverts[i].fNY;
	//	loadedvertexes[i].m = tempverts[i].fNZ;
	//	loadedvertexes[i].u = tempverts[i].fU;
	//	loadedvertexes[i].v = tempverts[i].fV;
	//}
#pragma endregion
	size_t size = vertexvec.size();
	SIMPLE_VERTEX* loadedvertexes = new SIMPLE_VERTEX[size];
	for (unsigned int i = 0; i < vertexvec.size(); i++)
	{

		loadedvertexes[i].x = vertexvec[i].pos[0];
		loadedvertexes[i].y = vertexvec[i].pos[1];
		loadedvertexes[i].z = vertexvec[i].pos[2];
		loadedvertexes[i].w = 1;
		loadedvertexes[i].n = normalvec[i].normal[0];
		loadedvertexes[i].r = normalvec[i].normal[1];
		loadedvertexes[i].m = normalvec[i].normal[2];
		loadedvertexes[i].u = uvvec[i].uv[0];
		loadedvertexes[i].v = uvvec[i].uv[1];
	}

	unsigned int* loadedindicies = new unsigned int[temptriangle.size()];
	for (unsigned int j = 0; j < temptriangle.size(); j++)
	{
		loadedindicies[j] = temptriangle[j];
	}
#pragma region
	ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));

	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = BACKBUFFER_WIDTH;
	vp.Height = BACKBUFFER_HEIGHT;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	//Vertex Buffer
	D3D11_SUBRESOURCE_DATA thedata;
	thedata.pSysMem = loadedvertexes;
	thedata.SysMemPitch = 0;
	thedata.SysMemSlicePitch = 0;

	ZeroMemory(&buffdesc, sizeof(buffdesc));

	buffdesc.Usage = D3D11_USAGE_IMMUTABLE;
	buffdesc.ByteWidth = sizeof(SIMPLE_VERTEX) * vertexvec.size();
	buffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffdesc.CPUAccessFlags = NULL;

	thedevice->CreateBuffer(&buffdesc, &thedata, &buffer);

	D3D11_SUBRESOURCE_DATA theInddata;
	theInddata.pSysMem = loadedindicies;
	theInddata.SysMemPitch = 0;
	theInddata.SysMemSlicePitch = 0;

	ZeroMemory(&Indexbufferdesc, sizeof(Indexbufferdesc));

	Indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;
	Indexbufferdesc.ByteWidth = sizeof(unsigned int) * temptriangle.size();
	Indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	Indexbufferdesc.CPUAccessFlags = NULL;

	thedevice->CreateBuffer(&Indexbufferdesc, &theInddata, &Indexbuffer);
#pragma endregion
	int PixelShadersize = sizeof(PixelShader) / sizeof(PixelShader[0]);
	thedevice->CreatePixelShader(PixelShader, PixelShadersize, NULL, &pixshader);
	int VertexShadersize = sizeof(VertexShader) / sizeof(VertexShader[0]);
	thedevice->CreateVertexShader(VertexShader, VertexShadersize, NULL, &vertshader);


	D3D11_INPUT_ELEMENT_DESC thelayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	thedevice->CreateInputLayout(thelayout, 3, VertexShader, VertexShadersize, &InputLayout);

	ZeroMemory(&constbuffdesc, sizeof(constbuffdesc));

	constbuffdesc.Usage = D3D11_USAGE_DYNAMIC;

	constbuffdesc.ByteWidth = sizeof(SEND_TO_OBJECT);

	constbuffdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	constbuffdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	thedevice->CreateBuffer(&constbuffdesc, NULL, &constbuffer);

	ZeroMemory(&constbuffdesc2, sizeof(constbuffdesc2));

	constbuffdesc2.Usage = D3D11_USAGE_DYNAMIC;

	constbuffdesc2.ByteWidth = sizeof(SEND_TO_SCENE);

	constbuffdesc2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	constbuffdesc2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	thedevice->CreateBuffer(&constbuffdesc2, NULL, &constbuffer2);

	HRESULT hr = S_OK;
	texdepth.Width = BACKBUFFER_WIDTH;
	texdepth.Height = BACKBUFFER_HEIGHT;
	texdepth.MipLevels = 1;
	texdepth.ArraySize = 1;
	texdepth.Format = DXGI_FORMAT_D32_FLOAT;
	texdepth.SampleDesc.Count = 4;
	texdepth.SampleDesc.Quality = 0;
	texdepth.Usage = D3D11_USAGE_DEFAULT;
	texdepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texdepth.CPUAccessFlags = 0;
	texdepth.MiscFlags = 0;
	hr = thedevice->CreateTexture2D(&texdepth, NULL, &pDepthStencil);

	ZeroMemory(&texdsv, sizeof(texdsv));
	texdsv.Format = DXGI_FORMAT_D32_FLOAT;
	texdsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	texdsv.Texture2D.MipSlice = 0;

	hr = thedevice->CreateDepthStencilView(pDepthStencil, &texdsv, &pDSV);

}

bool RTAPROJECT::Run()
{
#pragma region
	POINT newMousePos = mousePos;
	GetCursorPos(&newMousePos);
	sc->GetDesc(&SwapChainDescVar);

	XMStoreFloat4x4((XMFLOAT4X4*)&objecttoScene.ProjectionMatrix, XMMatrixPerspectiveFovLH(3.14f / 3.0f, (((float)SwapChainDescVar.BufferDesc.Width) / (float)SwapChainDescVar.BufferDesc.Height), 0.1f, 1000.0f));

	const float ColorRGBA[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
	thedevicecontext->ClearRenderTargetView(RenTarView, ColorRGBA);

	thedevicecontext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH, 1, NULL);

	thedevicecontext->OMSetRenderTargets(1, &RenTarView, pDSV);
	thedevicecontext->RSSetViewports(1, &vp);
	translateparam = .1f;

	if (GetAsyncKeyState('W'))
	{
		objecttoScene.ViewMatrix = MatrixMultMatrix(MatrixTranslateZ(translateparam), objecttoScene.ViewMatrix);
	}
	if (GetAsyncKeyState('S'))
	{
		objecttoScene.ViewMatrix = MatrixMultMatrix(MatrixTranslateZ(-translateparam), objecttoScene.ViewMatrix);

	}
	if (GetAsyncKeyState('A'))
	{
		objecttoScene.ViewMatrix = MatrixMultMatrix(MatrixTranslateX(-translateparam), objecttoScene.ViewMatrix);

	}
	if (GetAsyncKeyState('D'))
	{
		objecttoScene.ViewMatrix = MatrixMultMatrix(MatrixTranslateX(translateparam), objecttoScene.ViewMatrix);
	}
	if (GetAsyncKeyState('Q'))
	{
		objecttoScene.ViewMatrix.mat[3][1] += translateparam;
	}
	if (GetAsyncKeyState('E'))
	{
		objecttoScene.ViewMatrix.mat[3][1] -= translateparam;

	}
	if (GetAsyncKeyState(VK_RBUTTON) || GetAsyncKeyState(VK_LBUTTON)){
		FLOAT3 savedPosition;
		savedPosition.x = objecttoScene.ViewMatrix.mat[3][0];
		savedPosition.y = objecttoScene.ViewMatrix.mat[3][1];
		savedPosition.z = objecttoScene.ViewMatrix.mat[3][2];
		objecttoScene.ViewMatrix.mat[3][0] = objecttoScene.ViewMatrix.mat[3][1] = objecttoScene.ViewMatrix.mat[3][2] = 0;
		float xRatio = float(newMousePos.x - mousePos.x);
		float yRatio = float(newMousePos.y - mousePos.y);
		objecttoScene.ViewMatrix = MatrixMultMatrix(RotationX(-yRatio*.01f), objecttoScene.ViewMatrix);
		objecttoScene.ViewMatrix = MatrixMultMatrix(objecttoScene.ViewMatrix, RotationY(-xRatio*.01f));
		objecttoScene.ViewMatrix.mat[3][0] = savedPosition.x;
		objecttoScene.ViewMatrix.mat[3][1] = savedPosition.y;
		objecttoScene.ViewMatrix.mat[3][2] = savedPosition.z;
	}
#pragma region ObjectDrawCall
	thedevicecontext->Map(constbuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapsubres);

	memcpy(Mapsubres.pData, &objecttoObject, sizeof(objecttoObject));

	thedevicecontext->Unmap(constbuffer, NULL);
	thedevicecontext->Map(constbuffer2, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &Mapsubres2);
	Matrix savedViewMatrix = objecttoScene.ViewMatrix;
	SpecialCaseMatInverse(objecttoScene.ViewMatrix);
	memcpy(Mapsubres2.pData, &objecttoScene, sizeof(objecttoScene));
	objecttoScene.ViewMatrix = savedViewMatrix;

	thedevicecontext->Unmap(constbuffer2, NULL);
	thedevicecontext->VSSetConstantBuffers(0, 1, &constbuffer);
	thedevicecontext->VSSetConstantBuffers(1, 1, &constbuffer2);
	unsigned int Stride = sizeof(SIMPLE_VERTEX);
	unsigned int Offset = 0;
	thedevicecontext->IASetVertexBuffers(0, 1, &buffer, &Stride, &Offset);
	thedevicecontext->IASetIndexBuffer(Indexbuffer, DXGI_FORMAT_R32_UINT, 0);
	thedevicecontext->VSSetShader(vertshader, 0, 0);
	thedevicecontext->PSSetShader(pixshader, 0, 0);
	thedevicecontext->PSSetShaderResources(0, 1, &floorRSV[0]);
	thedevicecontext->IASetInputLayout(InputLayout);
	thedevicecontext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//thedevicecontext->DrawIndexed(temptriangle.size(), 0, 0);
	thedevicecontext->Draw(vertexvec.size(), 0);
	mousePos = newMousePos;
	sc->Present(0, 0);

	return true;
}


bool RTAPROJECT::ShutDown()
{
	SAFE_RELEASE(constbuffer);
	SAFE_RELEASE(constbuffer2);
	SAFE_RELEASE(vertshader);
	SAFE_RELEASE(pixshader);
	SAFE_RELEASE(InputLayout);
	SAFE_RELEASE(sc);
	SAFE_RELEASE(thedevicecontext);
	SAFE_RELEASE(thedevice);
	SAFE_RELEASE(RenTarView);
	SAFE_RELEASE(buffer);
	SAFE_RELEASE(pDepthStencil);
	SAFE_RELEASE(pDSV);
	SAFE_RELEASE(floorRSV[0]);
	SAFE_RELEASE(Indexbuffer);
	UnregisterClass(L"DirectXApplication", application);
	return true;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	RTAPROJECT myApp(hInstance, (WNDPROC)WndProc);
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
						//case WM_SIZE:
						//if (sc)
						//{
						//	thedevicecontext->OMSetRenderTargets(0, 0, 0);

						//	thedevicecontext->ClearState();

						//	RenTarView->Release();

						//	HRESULT hr;
						//	unsigned int w = LOWORD(lParam);
						//	unsigned int h = HIWORD(lParam);
						//	hr = sc->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, 0);

						//	DXGI_SWAP_CHAIN_DESC SwapChainDesc;

						//	ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));

						//	sc->GetDesc(&SwapChainDesc);

						//	ID3D11Texture2D * textbuffer;

						//	hr = sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&textbuffer);
						//	hr = thedevice->CreateRenderTargetView(textbuffer, NULL, &RenTarView);

						//	textbuffer->Release();
						//	pDepthStencil->Release();
						//	pDSV->Release();
						//	//HRESULT hr = S_OK;
						//	texdepth.Width = w;
						//	texdepth.Height = h;
						//	texdepth.MipLevels = 1;
						//	texdepth.ArraySize = 1;
						//	texdepth.Format = DXGI_FORMAT_D32_FLOAT;
						//	texdepth.SampleDesc.Count = 4;
						//	texdepth.SampleDesc.Quality = 0;
						//	texdepth.Usage = D3D11_USAGE_DEFAULT;
						//	texdepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
						//	texdepth.CPUAccessFlags = 0;
						//	texdepth.MiscFlags = 0;
						//	hr = thedevice->CreateTexture2D(&texdepth, NULL, &pDepthStencil);

						//	ZeroMemory(&texdsv, sizeof(texdsv));
						//	texdsv.Format = DXGI_FORMAT_D32_FLOAT;
						//	texdsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
						//	texdsv.Texture2D.MipSlice = 0;

						//	hr = thedevice->CreateDepthStencilView(pDepthStencil, // Depth stencil texture
						//		&texdsv, // Depth stencil desc
						//		&pDSV);  // [out] Depth stencil view

						//	thedevicecontext->OMSetRenderTargets(1, &RenTarView, NULL);

						//	vp.Width = w;
						//	vp.Height = h;
						//	vp.MinDepth = 0.0f;
						//	vp.MaxDepth = 1.0f;
						//	vp.TopLeftX = 0;
						//	vp.TopLeftY = 0;
						//	thedevicecontext->RSSetViewports(1, &vp);

						//}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

