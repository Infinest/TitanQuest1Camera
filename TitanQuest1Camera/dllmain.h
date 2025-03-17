#define _USE_MATH_DEFINES
#include "includes.h"
#include "hook.h"
#include "vector3d.h"

#if DEBUG_OUTPUT
FILE* pCout = nullptr;
#endif

typedef void(__thiscall* SetGameLoadingT)(void* gGameEngine, bool isLoading);
typedef void* (__thiscall* GetCameraT)(void* gGameEngine);
typedef void(__thiscall* SetCameraYawT)(void* WorldGamera, float radian);
typedef float(__thiscall* GetCameraYawT)(void* WorldGamera);
typedef void(__thiscall* SetCameraPitchT)(void* WorldGamera, float radian);
typedef float(__thiscall* GetCameraPitchT)(void* WorldGamera);
typedef void(__thiscall* SetCameraFovT)(void* WorldCamera, float fov);
typedef float(__thiscall* GetCameraFovT)(void* WorldCamera);
typedef void(__thiscall* SetFreeFlyT)(void* WorldCamera, bool isFreeFly);
typedef bool(__thiscall* GetFreeFlyT)(void* WorldCamera);
typedef void(__thiscall* WorldCameraUpdateT)(void* WorldCamera);
typedef HRESULT(__stdcall* DirectInput8CreateT)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter);
typedef HRESULT(__stdcall* GetDeviceStateT)(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData);
typedef HRESULT(__stdcall* GetDeviceDataT)(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pwdInOut, DWORD dwFlags);

// External object pointers
void* gGameEngine = nullptr;
void* camera = nullptr;

// External function pointers
SetGameLoadingT funcSetGameLoading = nullptr;
GetCameraT funcGetCamera = nullptr;
SetCameraYawT funcSetCameraYaw = nullptr;
GetCameraYawT funcGetCameraYaw = nullptr;
SetCameraPitchT funcSetCameraPitch = nullptr;
GetCameraPitchT funcGetCameraPitch = nullptr;
SetCameraFovT funcSetCameraFov = nullptr;
GetCameraFovT funcGetCameraFov = nullptr;
SetFreeFlyT funcSetFreeFly = nullptr;
GetFreeFlyT funcGetFreeFly = nullptr;
WorldCameraUpdateT funcWorldCameraUpdate = nullptr;

// Pointers to original external functions after trampoline hooks have been applied
SetGameLoadingT originalSetGameLoading = nullptr;
GetDeviceStateT originalGetDeviceState = nullptr;
GetDeviceDataT originalGetDeviceData = nullptr;
SetCameraYawT originalSetCameraYaw = nullptr;
WorldCameraUpdateT originalWorldCameraUpdate = nullptr;
SetCameraPitchT originalSetCameraPitch = nullptr;

// Addresses of original Dinput8 function
FARPROC p[6];

// Handle for original Dinput8.dll
HINSTANCE hL = 0;

bool upPressed = false;
bool downPressed = false;
bool leftPressed = false;
bool rightPressed = false;
bool ascendPressed = false;
bool descendPressed = false;
bool fovIncreasePressed = false;
bool fovDecreasePressed = false;

// Game hooks
void __fastcall hookSetGameLoading(void* gGameEngine, void* unused, bool isLoading);
void __fastcall hookSetCameraYaw(void* WorldCamera, float radian);
void __fastcall hookSetCameraPitch(void* WorldCamera, float radian);
void __fastcall hookWorldCameraUpdate(void* WorldCamera);

// Dinput8 hooks & helper functions
uintptr_t getDirectInputDevice8VTableOffset(LPDIRECTINPUT8& device, int vTableOffset);
HRESULT __stdcall hookGetDeviceState(IDirectInputDevice8* pThis, DWORD cbData, LPVOID lpvData);
HRESULT __stdcall hookGetDeviceData(IDirectInputDevice8* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pwdInOut, DWORD dwFlags);

void setup();
BOOL WINAPI DllMain(HMODULE hModule, DWORD reason, LPVOID);

// Proxy functions for Dinput8.dll
extern "C"
{
	HRESULT __stdcall PROXY_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, void** ppvOut, LPUNKNOWN punkOuter);
	void __stdcall PROXY_DllCanUnloadNow();
	void __stdcall PROXY_DllGetClassObject();
	void __stdcall PROXY_DllRegisterServer();
	void __stdcall PROXY_DllUnregisterServer();
	void __stdcall PROXY_GetdfDIJoystick();
}