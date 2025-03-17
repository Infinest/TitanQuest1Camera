#include "dllmain.h"


// Game hooks
void __fastcall hookSetGameLoading(void* gGameEngine, void* unused, bool isLoading)
{
	if (isLoading && camera != nullptr)
	{
		funcSetFreeFly(camera, false);
	}
	originalSetGameLoading(gGameEngine, isLoading);
}

void __fastcall hookSetCameraYaw(void* WorldCamera, float radian)
{
	// Disallow the game itself making alterations to the camera yaw
	return;
}

void __fastcall hookSetCameraPitch(void* WorldCamera, float radian)
{
	// Disallow the game itself making alteration to the camera pitch
	return;
}

void __fastcall hookWorldCameraUpdate(void* WorldCamera)
{
	float x = ((float*)((char*)WorldCamera + 0x98))[0];
	float z = ((float*)((char*)WorldCamera + 0x9c))[0];
	float y = ((float*)((char*)WorldCamera + 0xa0))[0];
	originalWorldCameraUpdate(WorldCamera);

	if (WorldCamera == camera)
	{
		float fov = ((float*)((char*)WorldCamera + 0x18))[0];
		if (fovIncreasePressed)
		{
			*(float*)((char*)WorldCamera + 0x18) = fov + 0.001;
		}
		else if (fovDecreasePressed)
		{
			*(float*)((char*)WorldCamera + 0x18) = fov - 0.001;
		}

		if (funcGetFreeFly(camera))
		{
			Vector3d cameraMovement = Vector3d();
			if (leftPressed)
			{
				cameraMovement.x = -1;
			}
			else if (rightPressed)
			{
				cameraMovement.x = 1;
			}

			if (upPressed)
			{
				cameraMovement.y = -1;
			}
			else if (downPressed)
			{
				cameraMovement.y = 1;
			}


			cameraMovement.setVectorPitch(funcGetCameraPitch(camera));

			if (ascendPressed)
			{
				cameraMovement.z = 1;
			}
			else if (descendPressed)
			{
				cameraMovement.z = -1;
			}

			cameraMovement.setVectorYaw(-funcGetCameraYaw(camera));
			cameraMovement.normalize();
			cameraMovement.scale(0.05);

			*(float*)((char*)WorldCamera + 0x98) = x + cameraMovement.x;
			*(float*)((char*)WorldCamera + 0x9c) = z + cameraMovement.z;
			*(float*)((char*)WorldCamera + 0xa0) = y + cameraMovement.y;
		}
	}
}

// Dinput8 hooks & helper functions
HRESULT __stdcall hookGetDeviceState(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData) {
	HRESULT result = originalGetDeviceState(pThis, cbData, lpvData);
	if (result == DI_OK)
	{
		if (cbData == sizeof(DIMOUSESTATE2))
		{
			if (((LPDIMOUSESTATE2)lpvData)->rgbButtons[2] != 0) // Middle mouse button is pressed
			{
				if (camera != nullptr)
				{
					float degrees;
					float radians = funcGetCameraYaw(camera);
					if (funcGetFreeFly(camera))
					{
						degrees = std::fmod((radians * 180 / M_PI) - ((LPDIMOUSESTATE2)lpvData)->lX * 0.1, 360);
					}
					else
					{
						degrees = std::fmod((radians * 180 / M_PI) + ((LPDIMOUSESTATE2)lpvData)->lX * 0.1, 360);
					}
					if (degrees < 0) {
						degrees += 360.0;
					}
					originalSetCameraYaw(camera, degrees * M_PI / 180);

					radians = funcGetCameraPitch(camera);
					degrees = (radians * 180 / M_PI) + ((LPDIMOUSESTATE2)lpvData)->lY * 0.1;
					if (std::abs(degrees) < 90)
					{
						originalSetCameraPitch(camera, degrees * M_PI / 180);
					}
				}
			}
		}
	}
	return result;
}

HRESULT __stdcall hookGetDeviceData(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pwdInOut, DWORD dwFlags)
{
	HRESULT result = originalGetDeviceData(pThis, cbObjectData, rgdod, pwdInOut, dwFlags);
	if (result == DI_OK)
	{
		for (int i = 0; i < *pwdInOut; i++)
		{
			switch (rgdod[i].dwOfs)
			{
			case DIK_NUMPAD8:
				upPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD5:
				downPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD4:
				leftPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD6:
				rightPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD7:
				ascendPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD9:
				descendPressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPADPLUS:
				fovIncreasePressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPADMINUS:
				fovDecreasePressed = 0 < LOBYTE(rgdod[i].dwData);
				break;
			case DIK_NUMPAD0:
				if (0 < LOBYTE(rgdod[i].dwData))
				{
					funcSetFreeFly(camera, !funcGetFreeFly(camera));
				}
				break;
			}
		}
	}

	return result;
}

uintptr_t getDirectInputDevice8VTableOffset(LPDIRECTINPUT8& device, int vTableOffset)
{
	LPDIRECTINPUTDEVICE8 mouse = nullptr;
	if (device->CreateDevice(GUID_SysMouse, &mouse, NULL) == DI_OK)
	{
		uintptr_t avTable = ((uintptr_t*)mouse)[0];
		uintptr_t functionPointer = ((uintptr_t*)(avTable + vTableOffset * sizeof(uintptr_t)))[0];
		return functionPointer;
	}
	return 0;
}

void setup()
{
	HMODULE game;
	do {
		game = (HMODULE)GetModuleHandle(L"Game.dll");
	} while (!game);

	HMODULE engine;
	do {
		engine = (HMODULE)GetModuleHandle(L"Engine.dll");
	} while (!engine);

	gGameEngine = GetProcAddress(game, "?gGameEngine@GAME@@3PAVGameEngine@1@A");
	funcSetGameLoading = (SetGameLoadingT)GetProcAddress(game, "?SetGameLoading@GameEngine@GAME@@QAEX_N@Z");
	funcGetCamera = (GetCameraT)GetProcAddress(game, "?GetCamera@GameEngine@GAME@@QAEPAVGameCamera@2@XZ");
	funcSetCameraYaw = (SetCameraYawT)GetProcAddress(engine, "?SetCameraYaw@WorldCamera@GAME@@QAEXM@Z");
	funcGetCameraYaw = (GetCameraYawT)GetProcAddress(engine, "?GetCameraYaw@WorldCamera@GAME@@QBEMXZ");
	funcSetCameraPitch = (SetCameraPitchT)GetProcAddress(engine, "?SetCameraPitch@WorldCamera@GAME@@QAEXM@Z");
	funcGetCameraPitch = (GetCameraPitchT)GetProcAddress(engine, "?GetCameraPitch@WorldCamera@GAME@@QBEMXZ");
	funcSetCameraFov = (SetCameraFovT)GetProcAddress(engine, "?SetCameraFOV@WorldCamera@GAME@@QAEXM@Z");
	funcGetCameraFov = (GetCameraFovT)GetProcAddress(engine, "?GetCameraFOV@WorldCamera@GAME@@QAEXM@Z");
	funcSetFreeFly = (SetFreeFlyT)GetProcAddress(game, "?SetFreeFly@GameCamera@GAME@@QAEX_N@Z");
	funcGetFreeFly = (GetFreeFlyT)GetProcAddress(game, "?GetFreeFly@GameCamera@GAME@@QAE_NXZ");
	funcWorldCameraUpdate = (WorldCameraUpdateT)GetProcAddress(engine, "?Update@WorldCamera@GAME@@UAEXXZ");

	while (((uint16_t*)gGameEngine)[0] == 0)
	{
		Sleep(100);
	}

	camera = funcGetCamera((void*)((uintptr_t*)gGameEngine)[0]);
	originalSetGameLoading = (SetGameLoadingT)trampHook((char*)funcSetGameLoading, (char*)hookSetGameLoading, 10);
	originalSetCameraYaw = (SetCameraYawT)trampHook((char*)funcSetCameraYaw, (char*)hookSetCameraYaw, 6);
	originalSetCameraPitch = (SetCameraPitchT)trampHook((char*)funcSetCameraPitch, (char*)hookSetCameraPitch, 6);
	originalWorldCameraUpdate = (WorldCameraUpdateT)trampHook((char*)funcWorldCameraUpdate, (char*)hookWorldCameraUpdate, 6);

#if DEBUG_OUTPUT
	std::cout << std::hex << "Game.dll base: " << game << std::endl;
	std::cout << std::hex << "Engine.dll base: " << engine << std::endl;
	std::cout << std::hex << "gGameEngine object offset: " << gGameEngine << std::endl;
	std::cout << std::hex << "Game.dll GAME::GameEngine::GetCamera() offset: " << funcGetCamera << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::GetCameraYaw offset: " << funcGetCameraYaw << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::SetCameraYaw offset: " << funcSetCameraYaw << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::GetCameraPitch offset: " << funcGetCameraPitch << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::SetCameraPitch offset: " << funcSetCameraPitch << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::SetCameraFOV offset: " << funcSetCameraFov << std::endl;
	std::cout << std::hex << "Engine.dll GAME::WorldCamera::GetCameraFOV offset: " << funcGetCameraFov << std::endl;
	std::cout << std::hex << "WorldCamera offset: " << camera << std::endl;
	std::cout << std::hex << "Game.dll GAME::GameCamera::SetFreeFly() offset: " << funcSetFreeFly << std::endl;
	std::cout << std::hex << "Game.dll GAME::GameCamera::GetFreeFly() offset: " << funcGetFreeFly << std::endl;
	std::cout << std::hex << "Game.dll GAME::GameCamera::WorldCameraUpdate() offset: " << funcWorldCameraUpdate << std::endl;
#endif
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		hL = LoadLibrary(L"C:\\Windows\\SysWOW64\\dinput8.dll");
		if (!hL) return false;

		LPWSTR output[MAX_PATH];
		GetModuleFileName(NULL, (LPWSTR)output, MAX_PATH);

#if DEBUG_OUTPUT
		AllocConsole();
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		std::cout.clear();
		MessageBox(NULL, (LPCWSTR)output, L"OK", MB_OK);
#endif

		p[0] = GetProcAddress(hL, "DirectInput8Create");
		p[1] = GetProcAddress(hL, "DllCanUnloadNow");
		p[2] = GetProcAddress(hL, "DllGetClassObject");
		p[3] = GetProcAddress(hL, "DllRegisterServer");
		p[4] = GetProcAddress(hL, "DllUnregisterServer");
		p[5] = GetProcAddress(hL, "GetdfDIJoystick");
		CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)setup, hModule, 0, nullptr);
	case DLL_PROCESS_DETACH:
		FreeLibrary(hL);
		return 1;
	default:
		return 1;
	}
}

// Proxy functions for Dinput8.dll
extern "C"
{
	HRESULT __stdcall PROXY_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, void** ppvOut, LPUNKNOWN punkOuter) {
		HRESULT result = ((DirectInput8CreateT)(p[0 * 4]))(hinst, dwVersion, riidltf, ppvOut, punkOuter);

		if (result == DI_OK && originalGetDeviceState == nullptr)
		{
			uintptr_t functionAddress = getDirectInputDevice8VTableOffset(*(LPDIRECTINPUT8*)ppvOut, 9);
			originalGetDeviceState = (GetDeviceStateT)trampHook((char*)functionAddress, (char*)hookGetDeviceState, 5);
			functionAddress = getDirectInputDevice8VTableOffset(*(LPDIRECTINPUT8*)ppvOut, 10);
			std::cout << std::hex << "GetDeviceData address: " << functionAddress << std::endl;
			originalGetDeviceData = (GetDeviceDataT)trampHook((char*)functionAddress, (char*)hookGetDeviceData, 5);
		}
		return result;
	}

	__declspec(naked) void __stdcall PROXY_DllCanUnloadNow() {
		__asm
		{			jmp p[1 * 4]
		}
	}
	__declspec(naked) void __stdcall PROXY_DllGetClassObject() {
		__asm
		{			jmp p[2 * 4]
		}
	}
	__declspec(naked) void __stdcall PROXY_DllRegisterServer() {
		__asm
		{			jmp p[3 * 4]
		}
	}
	__declspec(naked) void __stdcall PROXY_DllUnregisterServer() {
		__asm
		{			jmp p[4 * 4]
		}
	}
	__declspec(naked) void __stdcall PROXY_GetdfDIJoystick() {
		__asm
		{			jmp p[5 * 4]
		}
	}
}
