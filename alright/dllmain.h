#pragma once
#include "includes.h"
#include "memory.h"

mINI::INIFile file("alright.ini");
mINI::INIStructure ini;

/* Signatures */
// Game loop singature
hook_shared_t<void(*)()> game_loop_hook;

// Wndproc signature
using wndproc_t = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
hook_shared_t<wndproc_t> wndproc_hook;

std::map<const char*, int> fpsups = {
	{ "all", 0 },
	{ "vlods", 1 },
	{ "nodep", 2 },
	{ "highpriority", 3 },
	{ "optimiser", 4 },
	{ "flickr", 5 },
	{ "birds", 6 },
	{ "other", 7 },
	{ "anticrasher", 8 },
	{ "lowfd", 9 },
	{ "lowlod", 10 },
};

std::map<int, const char*> keys = {
	{ 0x01, "Левая кнопка мыши" },
	{ 0x02, "Правая кнопка мыши" },
	{ 0x03, "Средняя кнопка мыши" },
	{ 0x04, "Кнопка мыши X1" },
	{ 0x05, "Кнопка мыши X2" },
	{ 0x08, "Backspace" },
	{ 0x09, "Tab" },
	{ 0x0C, "Очистить" },
	{ 0x0D, "Enter" },
	{ 0x10, "Shift" },
	{ 0x11, "Ctrl" },
	{ 0x12, "Alt" },
	{ 0x13, "Pause" },
	{ 0x14, "Caps Lock" },
	{ 0x1B, "Esc" },
	{ 0x20, "Пробел" },
	{ 0x21, "Page Up" },
	{ 0x22, "Page Down" },
	{ 0x23, "End" },
	{ 0x24, "Home" },
	{ 0x25, "Стрелка влево" },
	{ 0x26, "Стрелка вверх" },
	{ 0x27, "Стрелка вправо" },
	{ 0x28, "Стрелка вниз" },
	{ 0x2C, "Print Screen" },
	{ 0x2D, "Insert" },
	{ 0x2E, "Delete" },
	{ 0x30, "0" },
	{ 0x31, "1" },
	{ 0x32, "2" },
	{ 0x33, "3" },
	{ 0x34, "4" },
	{ 0x35, "5" },
	{ 0x36, "6" },
	{ 0x37, "7" },
	{ 0x38, "8" },
	{ 0x39, "9" },
	{ 0x41, "A" },
	{ 0x42, "B" },
	{ 0x43, "C" },
	{ 0x44, "D" },
	{ 0x45, "E" },
	{ 0x46, "F" },
	{ 0x47, "G" },
	{ 0x48, "H" },
	{ 0x49, "I" },
	{ 0x4A, "J" },
	{ 0x4B, "K" },
	{ 0x4C, "L" },
	{ 0x4D, "M" },
	{ 0x4E, "N" },
	{ 0x4F, "O" },
	{ 0x50, "P" },
	{ 0x51, "Q" },
	{ 0x52, "R" },
	{ 0x53, "S" },
	{ 0x54, "T" },
	{ 0x55, "U" },
	{ 0x56, "V" },
	{ 0x57, "W" },
	{ 0x58, "X" },
	{ 0x59, "Y" },
	{ 0x5A, "Z" },
	{ 0x5B, "Левая клавиша Windows" },
	{ 0x5C, "Правая клавиша Windows" },
	{ 0x5D, "Контекстное меню" },
	{ 0x60, "Нумпад 0" },
	{ 0x61, "Нумпад 1" },
	{ 0x62, "Нумпад 2" },
	{ 0x63, "Нумпад 3" },
	{ 0x64, "Нумпад 4" },
	{ 0x65, "Нумпад 5" },
	{ 0x66, "Нумпад 6" },
	{ 0x67, "Нумпад 7" },
	{ 0x68, "Нумпад 8" },
	{ 0x69, "Нумпад 9" },
	{ 0x70, "F1" },
	{ 0x71, "F2" },
	{ 0x72, "F3" },
	{ 0x73, "F4" },
	{ 0x74, "F5" },
	{ 0x75, "F6" },
	{ 0x76, "F7" },
	{ 0x77, "F8" },
	{ 0x78, "F9" },
	{ 0x79, "F10" },
	{ 0x7A, "F11" },
	{ 0x7B, "F12" },
};

const char* getKeyName(int vkCode) {
	auto it = keys.find(vkCode);
	if (it != keys.end()) {
		return it->second;
	}
	else {
		return "Неизвестная клавиша";
	}
}

inline bool is_file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

template <typename T>
std::string read_with_size(RakNet::BitStream* bs) {
	T size;
	if (!bs->Read(size))
		return {};
	std::string str(size, '\0');
	bs->Read(str.data(), size);
	return str;
}

void set_fps(const char* params) {
	sampapi::CMDPROC(rakhook::samp_addr() + 0x68160)(params);
	ini["alright"]["fps"] = params;
}

void set_weather(const char* params) {
	RakNet::BitStream bs;
	bs.Reset();
	bs.Write<int>(atoi(params));
	rakhook::emul_rpc(152, bs); // RPC_SetWeather
}

void set_time(const char* params) {
	RakNet::BitStream bs;
	bs.Write<int>(atoi(params));
	bs.Write<UINT8>(0);
	rakhook::emul_rpc(29, bs); // RPC_SetWorldTime
}

void set_fogdist(const char* params) {
	*(float*)0xB7C4F0 = atof(params);
	ini["alright"]["fd"] = params;
}

void set_loddist(const char* params) {
	*(float*)0x858FD8 = atof(params);
	ini["alright"]["ld"] = params;
}

bool catching_key;
void set_drugs_key() {
	catching_key = true;
	sampapi::v037r3::RefChat()->AddMessage(0, "catching key...");
}

void process_case(int func) { // SHIT implementation, could've been done better
	switch (func) {
	case 1: // vlods
		*(byte*)0x52C9EE ^= 1;
		break;

	case 2: // nodep (why???)
		SetProcessDEPPolicy(0);
		break;

	case 3: // highpriority
		SetPriorityClass(GetCurrentProcess(), 0x80);
		break;

	case 4: // optimiser.asi (14ms fps delay fix)
		*(byte*)0x53E94C = 1;
		*(byte*)0xBAB318 = 0;
		break;

	case 5: // flickr.asi (texture limit patch)
		*(DWORD*)0x5B8E55 = 90000;
		*(DWORD*)0x5B8EB0 = 90000;
		break;

	case 6: // disable birds
		*(DWORD*)0x53E170 = 0x90909090;
		*(byte*)0x53E174 = 0x90;
		break;

	case 7: // other stuff
		*(DWORD*)0x736F88 = 0x0;        // heli blows only once

		*(DWORD*)0x555854 = 0x90909090; // interior reflection fix
		*(byte*)0x555858 = 0x90;

		*(DWORD*)0xC2B9CC = 0x3EB3B675; // car speed fix? i guess

		memset(reinterpret_cast<void*>(0x5557CF), 0x90, 7); // binthesky by DK
		break;

	case 8: // anticrasher
		static auto samp_addr = rakhook::samp_addr();
		DWORD old;
		VirtualProtect((LPVOID)(samp_addr + 0x5CF2C), 20, PAGE_EXECUTE_READWRITE, &old);
		*(DWORD*)(samp_addr + 0x5CF2C) = 0x90909090;
		*(BYTE*)(samp_addr + 0x5CF30) = 0x90;
		*(DWORD*)(samp_addr + 0x5CF39) = 0x90909090;
		*(BYTE*)(samp_addr + 0x5CF3D) = 0x90;
		VirtualProtect((LPVOID)(samp_addr + 0x5CF2C), 20, old, &old);
		break;

	case 9: // lowfd
		set_fogdist("100");
		break;
	case 10: // lowlod
		set_loddist("50");
		break;
	}
}

void process_function(int func) { // SHIT implementation V2, could've been done better
	if (func == 0) {
		for (int i = 1; i <= fpsups.size() - 1; ++i) {
			process_case(i);
		}
	}
	else {
		process_case(func);
	}
}

void do_immediately() { // functions that can NOT (probably) cause any crashes, lags & etc. 
	process_function(fpsups["nodep"]);
	process_function(fpsups["highpriority"]);
	process_function(fpsups["anticrasher"]);
	process_function(fpsups["optimiser"]);
	process_function(fpsups["flickr"]);
	process_function(fpsups["birds"]);
	process_function(fpsups["other"]);
}