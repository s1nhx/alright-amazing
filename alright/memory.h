#pragma once
#include "includes.h"

bool compare_bytes(const std::uint8_t* data, const std::uint8_t* bytes, const char* mask);
bool compare_bytes(const std::uint8_t* data, const char* bytes, const char* mask);
std::uintptr_t find_pattern(std::uintptr_t base, std::size_t len, const std::uint8_t* bytes, const char* mask);
std::uintptr_t find_pattern(std::uintptr_t base, std::size_t len, const char* bytes, const char* mask);
std::uintptr_t find_device(std::uint32_t Len);
void* get_function_address(int VTableIndex);
void showCursor(bool state);
void nop_min_max_fps(std::uintptr_t& proc, std::uintptr_t& cmd);