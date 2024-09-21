#include "memory.h"

bool compare_bytes(const std::uint8_t* data, const std::uint8_t* bytes, const char* mask) {
	if (!data || !bytes || !mask) return false;
	for (; *mask; ++mask, ++data, ++bytes) {
		if (*mask == 'x' && std::memcmp(data, bytes, 1) != 0) {
			return false;
		}
	}
	return *mask == 0;
}

bool compare_bytes(const std::uint8_t* data, const char* bytes, const char* mask) {
	return compare_bytes(data, reinterpret_cast<const std::uint8_t*>(bytes), mask);
}

std::uintptr_t find_pattern(std::uintptr_t base, std::size_t len, const std::uint8_t* bytes, const char* mask) {
	for (auto i = 0u; i < len; ++i) {
		if (compare_bytes(reinterpret_cast<std::uint8_t*>(base + i), bytes, mask)) {
			return base + i;
		}
	}
	return 0;
}

std::uintptr_t find_pattern(std::uintptr_t base, std::size_t len, const char* bytes, const char* mask) {
	return find_pattern(base, len, reinterpret_cast<const std::uint8_t*>(bytes), mask);
}

std::uintptr_t find_device(std::uint32_t Len) {
	static std::uintptr_t base = [](std::size_t Len) {
		std::string path_to(MAX_PATH, '\0');
		if (auto size = GetSystemDirectoryA(path_to.data(), MAX_PATH)) {
			path_to.resize(size);
			path_to += "\\d3d9.dll";
			std::uintptr_t dwObjBase = reinterpret_cast<std::uintptr_t>(LoadLibraryA(path_to.c_str()));
			while (dwObjBase++ < dwObjBase + Len) {
				if (*reinterpret_cast<std::uint16_t*>(dwObjBase + 0x00) == 0x06C7 &&
					*reinterpret_cast<std::uint16_t*>(dwObjBase + 0x06) == 0x8689 &&
					*reinterpret_cast<std::uint16_t*>(dwObjBase + 0x0C) == 0x8689) {
					dwObjBase += 2;
					break;
				}
			}
			return dwObjBase;
		}
		return std::uintptr_t(0);
		}(Len);
		return base;
}

void* get_function_address(int VTableIndex) {
	return (*reinterpret_cast<void***>(find_device(0x128000)))[VTableIndex];
}

void showCursor(bool state)
{
	using RwD3D9GetCurrentD3DDevice_t = LPDIRECT3DDEVICE9(__cdecl*)();

	auto rwCurrentD3dDevice{ reinterpret_cast<
		RwD3D9GetCurrentD3DDevice_t>(0x7F9D50U)() };

	if (nullptr == rwCurrentD3dDevice) {
		return;
	}

	static DWORD
		updateMouseProtection,
		rsMouseSetPosProtFirst,
		rsMouseSetPosProtSecond;

	if (state)
	{
		::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U,
			PAGE_EXECUTE_READWRITE, &updateMouseProtection);

		::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U,
			PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtFirst);

		::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U,
			PAGE_EXECUTE_READWRITE, &rsMouseSetPosProtSecond);

		// NOP: CPad::UpdateMouse
		*reinterpret_cast<uint8_t*>(0x53F3C6U) = 0xE9U;
		*reinterpret_cast<uint32_t*>(0x53F3C6U + 1U) = 0x15BU;

		// NOP: RsMouseSetPos
		memset(reinterpret_cast<void*>(0x53E9F1U), 0x90, 5U);
		memset(reinterpret_cast<void*>(0x748A1BU), 0x90, 5U);

		rwCurrentD3dDevice->ShowCursor(TRUE);
	}
	else
	{
		// Original: CPad::UpdateMouse
		memcpy(reinterpret_cast<void*>(0x53F3C6U), "\xE8\x95\x6C\x20\x00", 5U);

		// Original: RsMouseSetPos
		memcpy(reinterpret_cast<void*>(0x53E9F1U), "\xE8\xAA\xAA\x0D\x00", 5U);
		memcpy(reinterpret_cast<void*>(0x748A1BU), "\xE8\x80\x0A\xED\xFF", 5U);

		using CPad_ClearMouseHistory_t = void(__cdecl*)();
		using CPad_UpdatePads_t = void(__cdecl*)();

		reinterpret_cast<CPad_ClearMouseHistory_t>(0x541BD0U)();
		reinterpret_cast<CPad_UpdatePads_t>(0x541DD0U)();

		::VirtualProtect(reinterpret_cast<void*>(0x53F3C6U), 5U,
			updateMouseProtection, &updateMouseProtection);

		::VirtualProtect(reinterpret_cast<void*>(0x53E9F1U), 5U,
			rsMouseSetPosProtFirst, &rsMouseSetPosProtFirst);

		::VirtualProtect(reinterpret_cast<void*>(0x748A1BU), 5U,
			rsMouseSetPosProtSecond, &rsMouseSetPosProtSecond);

		rwCurrentD3dDevice->ShowCursor(FALSE);
	}
}

void nop_min_max_fps(std::uintptr_t &proc, std::uintptr_t &cmd) {

	auto samp_addr = reinterpret_cast<std::uintptr_t>(GetModuleHandleA("samp.dll"));
	auto samp_size = reinterpret_cast<IMAGE_NT_HEADERS*>(samp_addr + reinterpret_cast<IMAGE_DOS_HEADER*>(samp_addr)->e_lfanew)->OptionalHeader.SizeOfImage;

	DWORD old;
	constexpr auto proc_fpslimit_bytes = "\x51\x56\x57\x8B\xF9\xE8";
	constexpr auto proc_fpslimit_mask = "xxxxxx";
	std::uintptr_t proc_fpslimit_addr_ = find_pattern(samp_addr, samp_size, proc_fpslimit_bytes, proc_fpslimit_mask);
	proc = proc_fpslimit_addr_;
	VirtualProtect((LPVOID)proc_fpslimit_addr_, 6, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<void*>(proc_fpslimit_addr_), 0xC3, 1);
	VirtualProtect((LPVOID)proc_fpslimit_addr_, 6, old, &old);

	constexpr auto cmd_fpslimit_bytes = "\x83\xFE\x14\x72";
	constexpr auto cmd_fpslimit_mask = "xxxx";
	std::uintptr_t cmd_fpslimit_addr_ = find_pattern(samp_addr, samp_size, cmd_fpslimit_bytes, cmd_fpslimit_mask);
	cmd = cmd_fpslimit_addr_;
	VirtualProtect((LPVOID)cmd_fpslimit_addr_, 10, PAGE_EXECUTE_READWRITE, &old);
	memset(reinterpret_cast<void*>(cmd_fpslimit_addr_), 0x90, 10);
	VirtualProtect((LPVOID)cmd_fpslimit_addr_, 10, old, &old);

	constexpr auto read_fpslimit_bytes = "\x83\xF8\x14\x7C";
	constexpr auto read_fpslimit_mask = "xxxx";
	std::uintptr_t read_fpslimit_addr_ = find_pattern(samp_addr, samp_size, read_fpslimit_bytes, read_fpslimit_mask);
	if (read_fpslimit_addr_) {
		VirtualProtect((LPVOID)read_fpslimit_addr_, 10, PAGE_EXECUTE_READWRITE, &old);
		memset(reinterpret_cast<void*>(read_fpslimit_addr_), 0x90, 10);
		VirtualProtect((LPVOID)read_fpslimit_addr_, 10, old, &old);
	}
	else {
		constexpr auto vmp_read_fpslimit_bytes = "\x0F\x8C\x00\x00\x00\x00\x60\x66\xC7\x44\x24\x04\x2F\x90";
		constexpr auto vmp_read_fpslimit_mask = "xx????xxxxxxxx";
		std::uintptr_t read_fpslimit_addr_ = find_pattern(samp_addr, samp_size, vmp_read_fpslimit_bytes, vmp_read_fpslimit_mask);
		VirtualProtect((LPVOID)read_fpslimit_addr_, 6, PAGE_EXECUTE_READWRITE, &old);
		memset(reinterpret_cast<void*>(read_fpslimit_addr_), 0x90, 6);
		VirtualProtect((LPVOID)read_fpslimit_addr_, 6, old, &old);
		VirtualProtect((LPVOID)(read_fpslimit_addr_ + 0x1C), 6, PAGE_EXECUTE_READWRITE, &old);
		memset(reinterpret_cast<void*>(read_fpslimit_addr_ + 0x1C), 0x90, 6);
		VirtualProtect((LPVOID)(read_fpslimit_addr_ + 0x1C), 6, old, &old);
	}

	memcpy(reinterpret_cast<void*>(0x745240), "\xB0\x00\x90\x90\x90", 5); // patch gta
}