#define IMGUI_DISABLE_INCLUDE_IMCONFIG_H
#include <imgui.h>
#include <reshade/reshade.hpp>

extern "C" DLLEXPORT const char* NAME = "Sky Reflection Fix for Skyrim";
extern "C" DLLEXPORT const char* DESCRIPTION = "";

HMODULE m_hModule;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID)
{
	if (dwReason == DLL_PROCESS_ATTACH)
		m_hModule = hModule;
	return TRUE;
}

bool _disableFix = false;

static void DrawMenu(reshade::api::effect_runtime*)
{
	ImGui::Checkbox("Disable Fix", &_disableFix);
}

struct Hooks
{
	struct TESWaterReflections__Update_Actor__GetLOSPosition
	{
		static RE::NiPoint3* thunk(RE::PlayerCharacter* a_player, RE::NiPoint3* a_target, int unk1, float unk2)
		{
			auto ret = func(a_player, a_target, unk1, unk2);
			if (!_disableFix) {
				auto camera = RE::PlayerCamera::GetSingleton();
				ret->x = camera->cameraRoot->world.translate.x;
				ret->y = camera->cameraRoot->world.translate.y;
				ret->z = camera->cameraRoot->world.translate.z;
			}
			return ret;
		}
		static inline REL::Relocation<decltype(thunk)> func;
	};


	static void Install()
	{
		stl::write_thunk_call<TESWaterReflections__Update_Actor__GetLOSPosition>(REL::RelocationID(31373, 32160).address() + REL::Relocate(0x1AD, 0x1CA, 0x1ed));
	}
};

static void MessageHandler(SKSE::MessagingInterface::Message* message)
{
	switch (message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		{
			if (reshade::register_addon(m_hModule)) {
				logger::info("Registered addon");
				reshade::register_overlay(nullptr, &DrawMenu);
			} else {
				logger::info("Failed to register addon");
			}
			break;
		}
	}
}

bool Load()
{
	Hooks::Install();

	SKSE::GetMessagingInterface()->RegisterListener("SKSE", MessageHandler);

	return true;
}
