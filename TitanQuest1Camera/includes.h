#pragma once
#include <iostream>
#include <windows.h>
#include <Psapi.h>

#define DEBUG_OUTPUT false
#define DIRECTINPUT_VERSION 0x800
#include "dinput.h"
#pragma comment(lib, "Dinput8.lib")
#pragma comment(lib, "Dxguid.lib")