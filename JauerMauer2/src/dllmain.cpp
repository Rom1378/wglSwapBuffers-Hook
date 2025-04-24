#include "../pch.h"
#include <vector>
#include <intrin.h>
#include "GLHelper.h"
#include "Hook.h"

using swap_buffers_fnsig = __int64(__fastcall*)(HDC);
swap_buffers_fnsig original_wgl_swap_buffers = nullptr;
uintptr_t m_wgl_swap_buffers = 0;


BOOL   __stdcall detourFunction(HDC dc) {
	std::cout << "detourFunction" << std::endl;
	GLubyte red[] = { 255, 0, 0 };
	DrawOutLine(0, 0, 100, 100, 5, red);

	return original_wgl_swap_buffers(dc); // redirect to trampoline with correct args
}


/*
namespace learn {
	int init_hook()
	{
		if (MH_Initialize() != MH_OK) {
			std::cout << "MH_Initialize() != MH_OK" << std::endl;
			return 0;
		}
		m_wgl_swap_buffers = GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");

		if (m_wgl_swap_buffers == 0) {
			std::cout << "m_wgl_swap_buffers: " << m_wgl_swap_buffers << std::endl;
			return 0;
		}
		std::cout << "m_wgl_swap_buffers" << m_wgl_swap_buffers << std::endl;
		original_wgl_swap_buffers = reinterpret_cast<swap_buffers_fn>(m_wgl_swap_buffers);

		if (MH_CreateHook(m_wgl_swap_buffers, detourFunction, reinterpret_cast<void**>(&original_wgl_swap_buffers)) != MH_OK) {
			std::cout << "MH_CreateHook != MH_OK" << std::endl;
			return 0;
		}
		return 1;
	}

	void enableHook() {
		MH_STATUS stats = MH_EnableHook(m_wgl_swap_buffers);
		std::cout << "stats" << stats << std::endl;
		if (stats != MH_OK) {
			std::cout << "stats" << stats << std::endl;
		}
	}
	void disableHook() {
		MH_DisableHook(MH_ALL_HOOKS);
	}

}
*/
/*c_context create_gl_context()
{
	c_context res;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	res.m_hdc_devicectx = GetDC(FindWindowA(nullptr, "Minecraft 1.7.10"));

	int pixel_format = ChoosePixelFormat(res.m_hdc_devicectx, &pfd);
	SetPixelFormat(res.m_hdc_devicectx, pixel_format, &pfd);

	res.m_oglrenderctx = wglGetCurrentContext();
	res.m_glrenderctx = wglCreateContext(res.m_hdc_devicectx);
	return res;
}
*/


DWORD WINAPI inject(HMODULE hmodule) {
	AllocConsole();
	FILE* fIn;
	FILE* fOut;
	freopen_s(&fIn, "conin$", "r", stdin);
	freopen_s(&fOut, "conout$", "w", stdout);
	freopen_s(&fOut, "conout$", "w", stderr);

	std::cout << "Injected" << std::endl;
	m_wgl_swap_buffers = (uintptr_t)GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");
	original_wgl_swap_buffers = (swap_buffers_fnsig)hook::patching::installHook(m_wgl_swap_buffers, (uintptr_t)detourFunction, 15);

	/*
	* write the hook
	OPENGL32.wglSwapBuffers - 48 89 5C 24 08        - mov [rsp+08],rbx
	OPENGL32.dll+41A75      - 48 89 74 24 10        - mov [rsp+10],rsi
	OPENGL32.dll+41A7A      - 57                    - push rdi
	OPENGL32.dll+41A7B      - 48 83 EC 40           - sub rsp,40 { 64 }
	OPENGL32.dll+41A7F      - 48 8B F1              - mov rsi,rcx
	OPENGL32.dll+41A82      - 33 FF                 - xor edi,edi
	OPENGL32.dll+41A84      - E8 B3FCFDFF           - call OPENGL32.wglSwapMultipleBuffers+10AC
	OPENGL32.dll+41A89      - 85 C0                 - test eax,eax
	OPENGL32.dll+41A8B      - 74 16                 - je OPENGL32.dll+41AA3
	OPENGL32.dll+41A8D      - 8D 4F 01              - lea ecx,[rdi+01]
	*/
	
	//exit if x pressed
	while(GetAsyncKeyState(0x58) == 0) {
		//enable of disable hook with arrowkeys
		if (GetAsyncKeyState(VK_UP) & 1) {
			std::cout << "enableHook" << std::endl;
			hook::patching::enableHook(m_wgl_swap_buffers);
		}
		if (GetAsyncKeyState(VK_DOWN) & 1) {
			std::cout << "disableHook" << std::endl;
			hook::patching::disableHook(m_wgl_swap_buffers);
		}


		Sleep(100);
	}


	hook::cleanup();

	//JoeMauer* joe = new JoeMauer();
	//delete joe;
	fclose(stdout);
	fclose(stdin);
	fclose(stderr);
	FreeConsole();
	FreeLibraryAndExitThread(hmodule, 0);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)inject, hModule, 0, nullptr);
	return TRUE;
}