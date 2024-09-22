#include "dllmain.h"

bool opened = false;
static bool ImGui_inited = false;
int drugskey;
bool init_command = false;
bool debugee;
DWORD memory;
UINT8 cef_app_set_mode;
UINT8 strange_byte;

std::uintptr_t cmd_fpslimit_addr = 0;
std::uintptr_t proc_fpslimit_addr = 0;

using namespace sampapi::v037r3;

void debugee_mode() {
	debugee ^= true;
}

void game_loop() {
	game_loop_hook->call_original();

	// new fps limiter (thank you, scout)
	static auto& game = **reinterpret_cast<std::uintptr_t**>(cmd_fpslimit_addr + 0xA + 2);
	if (!game) return;

	using namespace std::chrono_literals;
	static constexpr auto ms_to_ns = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(1ms).count());
	static auto			  previous = std::chrono::steady_clock::now();

	static auto cgame_fpslimit_off = *reinterpret_cast<std::uint8_t*>(proc_fpslimit_addr + 0xE7 + 2);
	auto		fpslimit = *reinterpret_cast<int*>(game + cgame_fpslimit_off);

	auto fps_duration = std::chrono::nanoseconds(static_cast<std::uint64_t>(890.0f / static_cast<float>(fpslimit) * ms_to_ns));
	auto loop_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - previous);
	if (loop_duration <= fps_duration) std::this_thread::sleep_for(fps_duration - loop_duration);
	previous = std::chrono::steady_clock::now();

	static bool initialized = false;
	if (initialized || !rakhook::initialize()) return;

	// register commands
	RefInputBox()->AddCommand("fps", (sampapi::CMDPROC)set_fps);
	RefInputBox()->AddCommand("sw", (sampapi::CMDPROC)set_weather);
	RefInputBox()->AddCommand("st", (sampapi::CMDPROC)set_time);
	RefInputBox()->AddCommand("fd", (sampapi::CMDPROC)set_fogdist);
	RefInputBox()->AddCommand("ld", (sampapi::CMDPROC)set_loddist);
	RefInputBox()->AddCommand("drugskey", (sampapi::CMDPROC)set_drugs_key);
	RefInputBox()->AddCommand("debugee", (sampapi::CMDPROC)debugee_mode);

	initialized = true;
}

LRESULT wndproc_hooked(HWND& hwnd, UINT& Message, WPARAM& wparam, LPARAM& lparam) {
	if (Message == WM_KEYDOWN) {
		if (wparam == drugskey) {
			RefInputBox()->Send("/drugs");
		}
		if (catching_key) {
			drugskey = wparam;
			ini["alright"]["drugskey"] = std::to_string(wparam);
			RefChat()->AddChatMessage("", 0, "{h}new key: ");
			RefChat()->AddMessage(0, getKeyName(wparam));
			catching_key = false;
		}
	}

	return wndproc_hook->call_original(hwnd, Message, wparam, lparam);
}

class entry_point {
public:
	entry_point() {
		// constructor

		if (!rakhook::samp_addr()) return;

		wndproc_hook = std::make_shared<hook_t<wndproc_t>>(reinterpret_cast<wndproc_t>(0x747EB0), wndproc_hooked);
		game_loop_hook = std::make_shared<hook_t<void(*)()>>(reinterpret_cast<void(*)()>(0x53BEE0), game_loop);

		nop_min_max_fps(proc_fpslimit_addr, cmd_fpslimit_addr);
		
		VirtualProtect((LPVOID)0x401000, 0x4F3000, PAGE_EXECUTE_READWRITE, NULL);
		
		if (is_file_exists("alright.ini")) {
			file.read(ini);
			if (stoi(ini["alright"]["fd"]))
				set_fogdist(ini["alright"]["fd"].c_str());
			if (stoi(ini["alright"]["ld"]))
				set_loddist(ini["alright"]["ld"].c_str());
			if (stoi(ini["alright"]["fps"]))
				set_fps(ini["alright"]["fps"].c_str());
			if (stoi(ini["alright"]["drugskey"]))
				drugskey = stoi(ini["alright"]["drugskey"]);

		}
		else {
			std::ofstream new_ini { "alright.ini" };
			
			ini["alright"].set({
				{"fd", "0"},
				{"ld", "0"},
				{"fps", "0"},
				{"drugskey", "0"},
			});

		}

		do_immediately();
	}
	~entry_point() {
		// destructor

		rakhook::destroy();

		wndproc_hook = nullptr;
		game_loop_hook = nullptr;

		file.write(ini);
	}
} _entry_point;
