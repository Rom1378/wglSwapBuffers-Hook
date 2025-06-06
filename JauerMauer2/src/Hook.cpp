#include "Hook.h"

namespace hook {
	void cleanup() {
		patching::cleanup();
	}
}

namespace hook::patching {

	std::vector<BYTE> movraxjmprax = {
		0x48, 0xB8,             // mov rax, imm64
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Target address
		0xFF, 0xE0,
	};

	struct PatchingHookData {
		uintptr_t trampoline = 0;
		std::vector<BYTE> savedOriginalBytes = std::vector<BYTE>(movraxjmprax.size(), 0x90); 
		uintptr_t originalFunction = 0;
		uintptr_t detourFunction = 0;
		bool isHooked = false;
	};

	std::unordered_map<uintptr_t, PatchingHookData> hookMap;

	//the trampoline execute stolen bytes the call detour then call the original function 
	uintptr_t createTrampoline(uintptr_t target) {

		auto& hookData = hookMap[target];

		size_t sizeSavedBytes = hookData.savedOriginalBytes.size();

		LPVOID trampMem = VirtualAlloc(NULL, sizeSavedBytes + movraxjmprax.size(), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!trampMem) {
			std::cout << "createTrampoline failed" << std::endl;
			return 0;
		}

		BYTE* trampoline = (BYTE*)trampMem;

		// 1. Copy the original bytes (overwritten by the hook)
		memcpy(trampoline, hookData.savedOriginalBytes.data(), sizeSavedBytes);

		// 2. Add a jump back to the original function (after the hook)
		trampoline += sizeSavedBytes;

		void* jmpBackAddr = (BYTE*)target + sizeSavedBytes;
		*(uintptr_t*)(movraxjmprax.data() + 2) = (uintptr_t)jmpBackAddr; // Set the jump address in movraxjmprax
		memcpy(trampoline, movraxjmprax.data(), movraxjmprax.size());

		return (uintptr_t)trampMem;

	}

	uintptr_t installHook(uintptr_t target, uintptr_t detour, uint32_t nbStolenBytes) {

		if (nbStolenBytes<12) {
			std::cout << "nbStolenBytes must be at least 12" << std::endl;
			return 0;
		}

		if (hookMap.find(target) != hookMap.end()) {
			std::cout << "Hook already installed at: " << std::hex << target << std::endl;
			return 0;
		}
		
		hookMap[target] = PatchingHookData();
		auto& hookData = hookMap[target];
		hookData.savedOriginalBytes.resize(nbStolenBytes);

		std::cout << "target: " << target << std::endl;
		// Save the original bytes
		memcpy(hookData.savedOriginalBytes.data(), (void*)target, nbStolenBytes);

		hookData.trampoline = createTrampoline(target);

		if (!hookData.trampoline) {
			std::cout << "createTrampoline failed" << std::endl;
			return 0;
		}

		hookData.detourFunction = detour;
		hookData.originalFunction = hookData.trampoline;

		hookData.isHooked = false;
		return hookData.trampoline;
	}

	void restoreOriginalBytes(uintptr_t target) {

		// Restore the original bytes
		DWORD oldProtect;
		BOOL result = 0;
		size_t sizeSavedBytes = hookMap[target].savedOriginalBytes.size();
		result = VirtualProtect((LPVOID)target, sizeSavedBytes, PAGE_EXECUTE_READWRITE, &oldProtect);
		if (result == 0) {
			std::cout << "VirtualProtect failed" << std::endl;
			std::cout << "GetLastError: " << GetLastError() << std::endl;
		}
		else {
			std::cout << "VirtualProtect success" << std::endl;
		}
		memcpy((void*)target, hookMap[target].savedOriginalBytes.data(), sizeSavedBytes);
		VirtualProtect((LPVOID)target, sizeSavedBytes, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), (LPVOID)target, sizeSavedBytes);
		hookMap[target].isHooked = false;
	}

	void enableHook(uintptr_t target) {
		if (hookMap.find(target) == hookMap.end()) {
			std::cout << "Hook not installed at: " << std::hex << target << std::endl;
			return;
		}
		if (hookMap[target].isHooked) {
			std::cout << "Hook already enabled at: " << std::hex << target << std::endl;
			return;
		}

		*(uintptr_t*)(movraxjmprax.data() + 2) = hookMap[target].detourFunction; // Set the detour address in movraxjmprax

		//calculate nb nop to write
		uint32_t nbNops = hookMap[target].savedOriginalBytes.size() - movraxjmprax.size();

		DWORD oldProtect;

		VirtualProtect((LPVOID)target, movraxjmprax.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy((void*)target, movraxjmprax.data(), movraxjmprax.size());
		memset((void*)(target + movraxjmprax.size()), 0x90, nbNops);
		VirtualProtect((LPVOID)target, movraxjmprax.size(), oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), (LPVOID)target, movraxjmprax.size());


		hookMap[target].isHooked = true;
		std::cout << "Hook enabled at: " << std::hex << target << std::endl;
	}

    void disableHook(uintptr_t target) {
		restoreOriginalBytes(target);
	}

	void cleanup(){
		for (auto& hook : hookMap) {
			restoreOriginalBytes(hook.first);
			VirtualFree((LPVOID)hook.second.trampoline, 0, MEM_RELEASE);
		}
		hookMap.clear();
	}

	/*
	///////////IAT HOOKING////////

	uintptr_t installHook(uintptr_t target, uintptr_t detour) {

		LPVOID imageBase = GetModuleHandleA(NULL);
		PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;
		PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + dosHeaders->e_lfanew);

		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = NULL;
		IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(importsDirectory.VirtualAddress + (DWORD_PTR)imageBase);
		LPCSTR libraryName = NULL;
		HMODULE library = NULL;
		PIMAGE_IMPORT_BY_NAME functionName = NULL;

		while (importDescriptor->Name != NULL)
		{
			libraryName = (LPCSTR)importDescriptor->Name + (DWORD_PTR)imageBase;
			library = LoadLibraryA(libraryName);

			if (library)
			{
				PIMAGE_THUNK_DATA originalFirstThunk = NULL, firstThunk = NULL;
				originalFirstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->OriginalFirstThunk);
				firstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->FirstThunk);

				while (originalFirstThunk->u1.AddressOfData != NULL)
				{
					functionName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)imageBase + originalFirstThunk->u1.AddressOfData);

					// find MessageBoxA address
					if (std::string(functionName->Name).compare("MessageBoxA") == 0)
					{
						SIZE_T bytesWritten = 0;
						DWORD oldProtect = 0;
						VirtualProtect((LPVOID)(&firstThunk->u1.Function), 8, PAGE_READWRITE, &oldProtect);

						// swap MessageBoxA address with address of hookedMessageBox
						firstThunk->u1.Function = (DWORD_PTR)detour;
					}
					++originalFirstThunk;
					++firstThunk;
				}
			}

			importDescriptor++;
		}
		return 0;
	}
	*/
}