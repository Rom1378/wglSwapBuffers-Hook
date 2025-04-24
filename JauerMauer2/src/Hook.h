#pragma once
#include "../pch.h"

namespace hook {
	namespace patching {
		//the trampoline execute stolen bytes the call detour then call the original function 
		uintptr_t createTrampoline(uintptr_t target);

		//minimun is 12. must be increased depending on the function you are hooking.
		//return the address of the trampoline
		uintptr_t installHook(uintptr_t target, uintptr_t detour, uint32_t nbStolenBytes = 12);
		void enableHook(uintptr_t target);
		void disableHook(uintptr_t target);
		void cleanup();
	}

	void cleanup();
}
