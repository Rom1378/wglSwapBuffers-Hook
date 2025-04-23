#include "pch.h"
#pragma comment(lib, "libMinHook.x64.lib")
//#include "JoeMauer.h"
#include "MinHook.h"
#include <iostream>
#include <vector>
#include <intrin.h>
//__writecr0(__readcr0() & ~0x1000);

typedef BOOL(WINAPI* wglSwapBuffersType)(HDC hdc);

using swap_buffers_fnsig = __int64(__fastcall)(HDC);
using swap_buffers_fn = int(__stdcall*)(HDC);
swap_buffers_fn original_wgl_swap_buffers = nullptr;
FARPROC m_wgl_swap_buffers = 0;
//extern bool(__stdcall DRAWManagerr::* best)(HDC dc);
//bool(__stdcall* detourFunction_ptr)(HDC dc);
HGLRC g_hRC = NULL; // Store OpenGL context


void DrawOutLine(float x, float y, float width, float height, float lineWidth, const GLubyte color[3]) {
	glLineWidth(lineWidth);
	glBegin(GL_LINE_STRIP);
	glVertex2f(x - 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y - 0.5f);
	glVertex2f(x + width + 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y + height + 0.5f);
	glVertex2f(x - 0.5f, y - 0.5f);
	glEnd();
}


BOOL   __stdcall detourFunction(HDC dc) {
	std::cout << "detourFunction" << std::endl;
	GLubyte red[] = { 255, 0, 0 };
	DrawOutLine(0, 0, 100, 100, 5, red);
	//restore original bytes
	//hook::patching::restoreOriginalBytes(m_wgl_swap_buffers);
	// Call the original function via trampoline
	//using OriginalFn = void(__stdcall*)();
	//OriginalFn originalFn = (OriginalFn)hook::patching::trampoline;
	//originalFn(); // Safely call the original
	return original_wgl_swap_buffers(dc);
}


/*
__declspec(naked) void asmTrampoline() {
	_asm {
		// Save all volatile registers that our hook might modify
		push rax
		push rcx
		push rdx
		push r8
		push r9
		push r10
		push r11

		// Call our C++ hook function
		// First parameter (HDC) is already in rcx for x64 calling convention
		sub rsp, 32      // Reserve shadow space for Win64 calling convention
		call hookedWglSwapBuffers
		add rsp, 32      // Restore stack after call

		// Save the return value (in rax)
		mov[rsp + 8 * 7], rax  // Save return value above the pushed registers

		// Restore registers in reverse order
		pop r11
		pop r10
		pop r9
		pop r8
		pop rdx
		pop rcx
		pop rax

		// Jump to the trampoline that has the original function prologue
		jmp[trampolineFunction]
	}
}*/




namespace hook {
	namespace patching {


		std::vector<BYTE> movraxjmprax = {
			0x48, 0xB8,             // mov rax, imm64
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Target address
			0xFF, 0xE0,
			0x90, 0x90,0x90// nop
		};
		std::vector<BYTE> savedOriginalBytes(movraxjmprax.size());
		FARPROC trampoline = nullptr;



		//the trampoline execute stolen bytes the call detour then call the original function 
		FARPROC createTrampoline(FARPROC target) {
			LPVOID trampMem = VirtualAlloc(NULL, savedOriginalBytes.size() + 2, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!trampMem) {
				std::cout << "createTrampoline failed" << std::endl;
				return nullptr;
			}

			BYTE* trampoline = (BYTE*)trampMem;

			// 1. Copy the original bytes (overwritten by the hook)
			memcpy(trampoline, savedOriginalBytes.data(), savedOriginalBytes.size());

			// 2. Add a jump back to the original function (after the hook)
			trampoline += savedOriginalBytes.size();

			void* jmpBackAddr = (BYTE*)target + savedOriginalBytes.size();
			*(FARPROC*)(movraxjmprax.data() + 2) = (FARPROC)jmpBackAddr; // Set the jump address in movraxjmprax
			memcpy(trampoline, movraxjmprax.data(), movraxjmprax.size());


			//3 call detour function
			//mov rax, detour
			//call rax
			/*std::vector<BYTE> movraxcallraxpoprcx = {
				//save rcx.
				//0x51,//push rcx


				0x48, 0xB8, // movabs rax,0x0000000000000000
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xFF, 0xD0,  //call rax
				//0x59 //pop rcx
			};
			*/

			//set detour addr in movraxcallrax
			/*
			*(FARPROC*)(pushrcxmovraxcallraxpoprcx.data() + 2) = (FARPROC)detourFunction;

			memcpy(trampoline, pushrcxmovraxcallraxpoprcx.data(), pushrcxmovraxcallraxpoprcx.size());
			trampoline += pushrcxmovraxcallraxpoprcx.size();

			//then add a jump back to the original function + offset

			void* jmpBackAddr = (BYTE*)target + savedOriginalBytes.size();

			*(FARPROC*)(movraxjmprax.data() + 2) = (FARPROC)jmpBackAddr;



			memcpy(trampoline, movraxjmprax.data(), movraxjmprax.size());
			*/
			return (FARPROC)trampMem;

		}

		void installHook(FARPROC target, FARPROC detour) {
			//edit movraxjmprax[]



			std::cout << "target: " << target << std::endl;
			// Save the original bytes
			memcpy(savedOriginalBytes.data(), (void*)target, savedOriginalBytes.size());

			trampoline = createTrampoline(m_wgl_swap_buffers);
			if (!trampoline) return;

			original_wgl_swap_buffers = (swap_buffers_fn)trampoline;


			*(FARPROC*)(movraxjmprax.data() + 2) = (FARPROC)detourFunction; // Set the detour address in movraxjmprax
			//cout the hooking 
			std::cout << "movraxjmprax: ";
			for (auto byte : movraxjmprax) {
				std::cout << std::hex << (int)byte << " ";
			}
			std::cout << std::endl;
			// Write the new bytes
			DWORD oldProtect;
			VirtualProtect((LPVOID)target, movraxjmprax.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
			memcpy((void*)target, movraxjmprax.data(), movraxjmprax.size());
			VirtualProtect((LPVOID)target, movraxjmprax.size(), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), (LPVOID)target, movraxjmprax.size());

		}

		void restoreOriginalBytes(FARPROC target) {
			// Restore the original bytes
			DWORD oldProtect;
			BOOL result = 0;
			result = VirtualProtect((LPVOID)target, savedOriginalBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
			if (result == 0) {
				std::cout << "VirtualProtect failed" << std::endl;
				std::cout << "GetLastError: " << GetLastError() << std::endl;
			}
			else {
				std::cout << "VirtualProtect success" << std::endl;
			}
			memcpy((void*)target, savedOriginalBytes.data(), savedOriginalBytes.size());
			VirtualProtect((LPVOID)target, savedOriginalBytes.size(), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), (LPVOID)target, savedOriginalBytes.size());
		}

	}
}

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
	m_wgl_swap_buffers = GetProcAddress(GetModuleHandleA("opengl32.dll"), "wglSwapBuffers");

	hook::patching::installHook(m_wgl_swap_buffers, (FARPROC)detourFunction);

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

	//origina bytes

	//(swap_buffers_fn)&detourFunction

	//init hook
	//if (!learn::init_hook()) {
	//	std::cout << "Failed to initialize hook" << std::endl;
		//return 1;
	//}

	//MH_EnableHook(MH_ALL_HOOKS);


	std::cin.get();

	//should restore the original bytes
	hook::patching::restoreOriginalBytes(m_wgl_swap_buffers);

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