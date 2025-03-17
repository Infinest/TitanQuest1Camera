#include "hook.h"


void patch(BYTE* dst, BYTE* src, uint16_t size) {
	DWORD oProc;
	VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oProc);
	memcpy(dst, src, size);
	VirtualProtect(dst, size, oProc, &oProc);
}

bool hook(char* src, char* dst, uint16_t len) {
	if (len < 5) return false;

	DWORD oProc;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oProc);
	memset(src, 0x90, len);
	uintptr_t relAddress = (uintptr_t)(dst - src - 5);
	*src = (char)0xE9;
	*(uintptr_t*)(src + 1) = (uintptr_t)relAddress;
	VirtualProtect(src, len, oProc, &oProc);
	return true;
}

char* trampHook(char* src, char* dst, uint16_t len) {
	if (len < 5) return 0;
	char* gateway = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!gateway) {
		return nullptr;
	}

	memcpy(gateway, src, len);
	uintptr_t jumpAddress = (uintptr_t)(src - gateway - 5);
	*(gateway + len) = (char)0xE9;
	*(uintptr_t*)(gateway + len + 1) = jumpAddress;
	
	if (hook(src, dst, len)) {
		return gateway;
	}

	VirtualFree(gateway, 0, MEM_RELEASE);
	return nullptr;
}