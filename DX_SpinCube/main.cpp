/*
 * Direct3D Spinning Cube Demo
 *
 * This is a basic spinning cube example written in Direct3D using C++. This also requires
 * you to have, at least, Visual Studio 2017 installed with the Windows SDK module.
 *
 * You will also need to have the DirectX SDK installed on your machine, a link to this will
 * be at the bottom of this code block.
 *
 * This demo is also licensed under the GNU General Public License, version 2.0, making this demo
 * Free Software, and under a copyleft license.
 *
 * GNU GPL2:					https://gnu.org/licenses/old-licenses/gpl-2.0.en.html
 * Demo Source on GitHub:		https://github.com/neodymeum/directx-cube
 * DirectX SDK Download:		https://www.microsoft.com/en-gb/download/details.aspx?id=6812
 *
 *     _   __              __          ____          _
 *    / | / /__  ____     / /   ____ _/ __/_______  (_)
 *   /  |/ / _ \/ __ \   / /   / __ `/ /_/ ___/ _ \/ /         Neo Lafrei
 *  / /|  /  __/ /_/ /  / /___/ /_/ / __/ /  /  __/ /  mailto:neodymeum@outlook.com
 * /_/ |_/\___/\____/  /_____/\__,_/_/ /_/   \___/_/
 */

#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

#define SCREEN_X			400
#define SCREEN_Y			400

#define IS_WINDOWED			TRUE
#define WINDOW_CLASS		"WindowClass"
#define WINDOW_TITLE		"Direct3D Spinning Cube"

LPDIRECT3D9						d3d;
LPDIRECT3DDEVICE9				d3ddev;
LPDIRECT3DVERTEXBUFFER9			v_buffer = NULL;
LPDIRECT3DINDEXBUFFER9			i_buffer = NULL;

void initDirect3D(HWND hWnd);
void renderFrame(void);
void initGraphics(void);
void initLight(void);
void clean3D(void);

struct CUSTOMVERTEX {
	FLOAT X, Y, Z;
	D3DVECTOR NORMAL;
};

#define CUSTOMFVF				(D3DFVF_XYZ | D3DFVF_NORMAL)

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	HWND		hWnd;
	WNDCLASSEX	wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WindowProc;
	wc.hInstance		= hInstance;
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName	= WINDOW_CLASS;

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(NULL, WINDOW_CLASS, WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW, 0, 0, SCREEN_X, SCREEN_Y,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	initDirect3D(hWnd);

	MSG msg;

	while (TRUE) {
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
			break;

		renderFrame();
	}
	clean3D();
	return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) { case WM_DESTROY: { PostQuitMessage(0); return 0; } break; }
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* Initialize Direct3D and prepare for use */
void initDirect3D(HWND hWnd) {
	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	D3DPRESENT_PARAMETERS d3dpp;

	ZeroMemory(&d3dpp, sizeof(d3dpp));

	d3dpp.Windowed					= IS_WINDOWED;
	d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow				= hWnd;
	d3dpp.BackBufferFormat			= D3DFMT_X8R8G8B8;
	d3dpp.BackBufferWidth			= SCREEN_X;
	d3dpp.BackBufferHeight			= SCREEN_Y;
	d3dpp.EnableAutoDepthStencil	= TRUE;
	d3dpp.AutoDepthStencilFormat	= D3DFMT_D16;

	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp, &d3ddev);

	initGraphics();
	initLight();

	d3ddev->SetRenderState(D3DRS_LIGHTING, TRUE);
	d3ddev->SetRenderState(D3DRS_ZENABLE, TRUE);
	d3ddev->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));
}

/* Render the frame */
void renderFrame(void) {
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	d3ddev->BeginScene();
	d3ddev->SetFVF(CUSTOMFVF);

	D3DXMATRIX matView;
	D3DXMatrixLookAtLH(&matView,
		&D3DXVECTOR3(0.0f, 8.0f, 25.0f), 
		&D3DXVECTOR3(0.0f, 0.0f, 0.0f),
		&D3DXVECTOR3(0.0f, 1.0f, 0.0f));

	d3ddev->SetTransform(D3DTS_VIEW, &matView);
	
	D3DXMATRIX matProjection;
	D3DXMatrixPerspectiveFovLH(&matProjection, D3DXToRadian(45), (FLOAT)SCREEN_X / (FLOAT)SCREEN_Y,
		1.0f, 100.0f);

	d3ddev->SetTransform(D3DTS_PROJECTION, &matProjection);

	static float index = 0.0f;
	static float target = 20.0f;
	static float yaw = 0.0f;
	static float pitch = 0.0f;

	if (index > target) {
		target = 0.0f;
		index -= 0.05f;
		pitch -= 0.02f;
		yaw -= 0.06f;
	}
	else {
		target = 20.0f;
		index += 0.05f;
		pitch += 0.02f;
		yaw += 0.06f;
	}


	D3DXMATRIX matRotateY;
	D3DXMatrixRotationY(&matRotateY, index);
	/* D3DXMatrixRotationYawPitchRoll(&matRotateY, yaw, pitch, index); */

	d3ddev->SetTransform(D3DTS_WORLD, &(matRotateY));
	d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));
	d3ddev->SetIndices(i_buffer);
	d3ddev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 24, 0, 12);
	d3ddev->EndScene();
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

void clean3D(void) {
	v_buffer->Release();
	i_buffer->Release();
	d3ddev->Release();
	d3d->Release();
}

void initGraphics(void) {
	CUSTOMVERTEX vertices[] = {

		// Cube Side 1
		{ -3.0f, -3.0f, 3.0f, 0.0f, 0.0f, 1.0f, },
		{ 3.0f, -3.0f, 3.0f, 0.0f, 0.0f, 1.0f, },
		{ -3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, },
		{ 3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 1.0f, },

		// Cube Side 2
		{ -3.0f, -3.0f, -3.0f, 0.0f, 0.0f, -1.0f, },
		{ -3.0f, 3.0f, -3.0f, 0.0f, 0.0f, -1.0f, },
		{ 3.0f, -3.0f, -3.0f, 0.0f, 0.0f, -1.0f, },
		{ 3.0f, 3.0f, -3.0f, 0.0f, 0.0f, -1.0f, },

		// Cube Side 3
		{ -3.0f, 3.0f, -3.0f, 0.0f, 1.0f, 0.0f, },
		{ -3.0f, 3.0f, 3.0f, 0.0f, 1.0f, 0.0f, },
		{ 3.0f, 3.0f, -3.0f, 0.0f, 1.0f, 0.0f, },
		{ 3.0f, 3.0f, 3.0f, 0.0f, 1.0f, 0.0f, },

		// Cube Side 4
		{ -3.0f, -3.0f, -3.0f, 0.0f, -1.0f, 0.0f, },
		{ 3.0f, -3.0f, -3.0f, 0.0f, -1.0f, 0.0f, },
		{ -3.0f, -3.0f, 3.0f, 0.0f, -1.0f, 0.0f, },
		{ 3.0f, -3.0f, 3.0f, 0.0f, -1.0f, 0.0f, },

		// Cube Side 5
		{ 3.0f, -3.0f, -3.0f, 1.0f, 0.0f, 0.0f, },
		{ 3.0f, 3.0f, -3.0f, 1.0f, 0.0f, 0.0f, },
		{ 3.0f, -3.0f, 3.0f, 1.0f, 0.0f, 0.0f, },
		{ 3.0f, 3.0f, 3.0f, 1.0f, 0.0f, 0.0f, },

		// Cube Side 6
		{ -3.0f, -3.0f, -3.0f, -1.0f, 0.0f, 0.0f, },
		{ -3.0f, -3.0f, 3.0f, -1.0f, 0.0f, 0.0f, },
		{ -3.0f, 3.0f, -3.0f, -1.0f, 0.0f, 0.0f, },
		{ -3.0f, 3.0f, 3.0f, -1.0f, 0.0f, 0.0f, },
	};

	d3ddev->CreateVertexBuffer(24 * sizeof(CUSTOMVERTEX), 0, CUSTOMFVF, D3DPOOL_MANAGED, &v_buffer, NULL);

	VOID* pVoid;
	v_buffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, vertices, sizeof(vertices));

	short indices[] = {
		// Side 1
		0, 1, 2,
		2, 1, 3,

		// Side 2
		4, 5, 6,
		6, 5, 7,

		// Side 3
		8, 9, 10,
		10, 9, 11,

		// Side 4
		12, 13, 14,
		14, 13, 15,

		// Side 5
		16, 17, 18,
		18, 17, 19,

		// Side 6
		20, 21, 22,
		22, 21, 23,
	};

	d3ddev->CreateIndexBuffer(36 * sizeof(short), 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &i_buffer, NULL);
	i_buffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, indices, sizeof(indices));
	i_buffer->Unlock();
}

void initLight(void) {
	D3DLIGHT9		light;
	D3DMATERIAL9	material;

	ZeroMemory(&light, sizeof(light));
	light.Type			= D3DLIGHT_DIRECTIONAL;
	light.Diffuse		= D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
	light.Direction		= D3DXVECTOR3(-1.0f, -0.3f, -1.0f);

	d3ddev->SetLight(0, &light);
	d3ddev->LightEnable(0, TRUE);

	ZeroMemory(&material, sizeof(D3DMATERIAL9));
	material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	d3ddev->SetMaterial(&material);
}