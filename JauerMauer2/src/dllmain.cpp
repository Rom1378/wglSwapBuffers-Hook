#include "../pch.h"
#include <intrin.h>
//#include "GLHelper.h"
#include "Hook.h"

#include "GLF.h"
#include "W2S.h"
#include "JavaList.h"
#include "Entity.h"


using swap_buffers_fnsig = __int64(__fastcall*)(HDC);
swap_buffers_fnsig original_wgl_swap_buffers = nullptr;
uintptr_t m_wgl_swap_buffers = 0;

JNIEnv* env;
JavaVM* jvm;

GLubyte red[3] = { 255, 0, 0 };

std::vector<Entity> entityList;
Entity player;

bool lockettListScreenPos;

std::vector<glm::vec2> ettListScreenPos;
std::mutex ettListMutex;


bool initialized = false;

BOOL   __stdcall detourFunction(HDC dc) {
	SetupOrtho();
	DrawOutLine(10, 10, 20, 20, 2.0f, red);


	//draw ettscreenpos
	RECT window_rect;
	GetWindowRect(FindWindowA(nullptr, "Minecraft 1.7.10"), &window_rect);
	
	std::lock_guard<std::mutex> lock(ettListMutex);
	for (auto& pos : ettListScreenPos) {
		//DrawOutLine(pos.x, pos.y, 20, 20, 2.0f, red);

		drawLine((window_rect.right - window_rect.left) / 2, (window_rect.bottom - window_rect.top) / 2,
			pos.x, pos.y, 2.0f, red);
	}

	RestoreGL();

	return original_wgl_swap_buffers(dc); // redirect to trampoline with correct args
}

/*
//https://www.ired.team/miscellaneous-reversing-forensics/windows-kernel-internals/pe-file-header-parser-in-c++
//https://medium.com/@s12deff/import-address-table-hooking-68d519a1da43
PIMAGE_IMPORT_DESCRIPTOR getImportTable() {
	LPVOID baseAddress = GetModuleHandleA(NULL);
	PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)baseAddress;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((DWORD_PTR)baseAddress + dosHeaders->e_lfanew);
	IMAGE_DATA_DIRECTORY importTableDesc = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR importTable = NULL;
	importTable = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)baseAddress + importTableDesc.VirtualAddress);
	return importTable;
}
int iatHooking(std::string funcName, LPVOID hookFunction) {
	//	Import Table represents a list of the DLLs that are imported by the executable, inside the import table we have
	//	the DLL name and the functions that are imported from that DLL.
	//
	//	Interesting fields:
	//
	//	OriginalFirstThunk: Import Name Table (INT) Pointer
	//	FirstThunk: Import Address Table (IAT) Pointer
	//	Name: DLL Name

	PIMAGE_IMPORT_DESCRIPTOR importTable = getImportTable();
	if (importTable == NULL) {
		std::cout << "Import Table not found" << std::endl;
		return 0;
	}


	LPVOID baseAddress = GetModuleHandleA(NULL);
	while (importTable->Name != NULL) {
		PIMAGE_THUNK_DATA INT = (PIMAGE_THUNK_DATA)((DWORD_PTR)baseAddress + importTable->OriginalFirstThunk);
		PIMAGE_THUNK_DATA IAT = (PIMAGE_THUNK_DATA)((DWORD_PTR)baseAddress + importTable->FirstThunk);
		std::cout << "Import Table Name: " << (char*)((DWORD_PTR)baseAddress + importTable->Name) << std::endl;
		std::cout << "INT: " << INT << std::endl;
		std::cout << "IAT: " << IAT << std::endl;

		while (INT->u1.AddressOfData != NULL) {
			PIMAGE_IMPORT_BY_NAME importByName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)baseAddress + INT->u1.AddressOfData);
			std::cout << "Function Name: " << importByName->Name << std::endl;
			if (std::string(importByName->Name) == funcName) {
				std::cout << "Found " << funcName << std::endl;
				//DWORD oldProtect;
				//VirtualProtect(&IAT->u1.Function, sizeof(DWORD_PTR), PAGE_READWRITE, &oldProtect);
				//IAT->u1.Function = (DWORD_PTR)hookFunction;
			}
			INT++;
			IAT++;
		}
		importTable++;
	}
	return 0;
}
void** find(const char* function, HMODULE module)
{
	if (!module)
		module = GetModuleHandle(0);

	PIMAGE_DOS_HEADER img_dos_headers = (PIMAGE_DOS_HEADER)module;
	PIMAGE_NT_HEADERS img_nt_headers = (PIMAGE_NT_HEADERS)((BYTE*)img_dos_headers + img_dos_headers->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR img_import_desc = (PIMAGE_IMPORT_DESCRIPTOR)((BYTE*)img_dos_headers + img_nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
	if (img_dos_headers->e_magic != IMAGE_DOS_SIGNATURE)
		printf("ERROR: e_magic is no valid DOS signature\n");

	for (IMAGE_IMPORT_DESCRIPTOR* iid = img_import_desc; iid->Name != 0; iid++) {
		for (int func_idx = 0; *(func_idx + (void**)(iid->FirstThunk + (size_t)module)) != nullptr; func_idx++) {
			char* mod_func_name = (char*)(*(func_idx + (size_t*)(iid->OriginalFirstThunk + (size_t)module)) + (size_t)module + 2);
			const intptr_t nmod_func_name = (intptr_t)mod_func_name;
			if (nmod_func_name >= 0) {
				if (!::strcmp(function, mod_func_name))
					return func_idx + (void**)(iid->FirstThunk + (size_t)module);
			}
		}
	}

	return 0;

}



*/


std::vector<glm::vec2> w2sEtt(std::vector<Entity> etts) {
	RECT window_rect;
	GetWindowRect(FindWindowA(nullptr, "Minecraft 1.7.10"), &window_rect);

	glm::vec2 screenPos;
	std::vector<glm::vec2> screenPoss;

	std::vector<float> model = Get_MODELVIEW();
	std::vector<float> proj = Get_PROJECTION();
	std::vector<int> viewport = Get_VIEWPORT();

	for (auto& et : etts) {
		glm::vec3 ettPos(et.getPosX(), et.getPosY(), et.getPosZ());
		ettPos -= glm::vec3(player.getPosX(), player.getPosY(), player.getPosZ());

		if (!WorldToScreen(ettPos, screenPos, model, proj, viewport)) {
			//std::cout << "world to screen failed" << std::endl;
		}
		else {
			screenPoss.push_back(screenPos);
		}
	}
	return screenPoss;
}

void updateGameData() {
	entityList.clear();
	JavaList loadedEntitilist(getTheWorld(), "field_72996_f", "Ljava/util/List;");
	uint32_t len = loadedEntitilist.getSize();
	jobject theworld = getTheWorld();

	for (uint32_t i = 0; i < len; i++) {
		jobject entityObj = loadedEntitilist.get(i);
		if (entityObj != nullptr) {
			entityList.emplace_back(theworld, entityObj);
			env->DeleteLocalRef(entityObj);
		}
	}

	player = Entity(theworld, loadedEntitilist.get(0));
	std::lock_guard<std::mutex> lock(ettListMutex);
	ettListScreenPos = w2sEtt(entityList);

}
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


	jsize count;
	if (JNI_GetCreatedJavaVMs(&jvm, 1, &count) != JNI_OK || count == 0) {
		std::cout << "Failed to get the JVM" << std::endl;
		return 0;
	}
	jint res = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
	if (res == JNI_EDETACHED)
		res = jvm->AttachCurrentThread((void**)&env, nullptr);
	if (res != JNI_OK) {
		std::cout << "Failed to attach to thread" << std::endl;
		return 0;
	}
	std::cout << "Attached to JVM" << std::endl;
	std::cout << "Env: " << env << std::endl;

	updateGameData();

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
	while (GetAsyncKeyState(0x58) == 0) {
		//enable of disable hook with arrowkeys


		if (GetAsyncKeyState(VK_UP) & 1) {
			std::cout << "enableHook" << std::endl;
			hook::patching::enableHook(m_wgl_swap_buffers);
		}
		if (GetAsyncKeyState(VK_DOWN) & 1) {
			std::cout << "disableHook" << std::endl;
			hook::patching::disableHook(m_wgl_swap_buffers);

		}
		updateGameData();
		Sleep(5);
	}


	entityList.clear();
	hook::cleanup();
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