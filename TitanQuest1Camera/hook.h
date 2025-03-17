#pragma once
#include "includes.h"

void patch(BYTE* dst, BYTE* src, uint16_t size);
bool hook(char* src, char* dst, uint16_t len);
char* trampHook(char* src, char* dst, uint16_t len);