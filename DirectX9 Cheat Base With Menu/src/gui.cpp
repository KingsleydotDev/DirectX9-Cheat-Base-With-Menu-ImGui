#include "gui.h"
#include "hooks.h"

#include "../ext/imgui/imgui.h"
#include "../ext/imgui/imgui_impl_win32.h"
#include "../ext/imgui/imgui_impl_dx9.h"
#include "../ext/imgui/imgui_internal.h"

#include "cruin.h"
#include "icon_font.h"
#include "images.h"
#include "ubuntu_bold.h"
#include "ubuntu_medium.h"
#include "ubunutu_regular.h"
#include "functions.h"
#include "logo.h"

#include <stdexcept>
#include <string>
#include <iostream>
#include <iostream>
#include <map>
using namespace functions;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

// window process
LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
);

bool gui::SetupWindowClass(const char* windowClassName) noexcept
{
	//Populate Window Class
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = windowClassName;
	windowClass.hIconSm = NULL;

	// register
	if (!RegisterClassEx(&windowClass))
	{
		return false;
	}

	return true;
}

void gui::DestroyWindowClass() noexcept
{
	UnregisterClass(
		windowClass.lpszClassName,
		windowClass.hInstance
	);
}

bool gui::SetupWindow(const char* windowName) noexcept
{
	// Setup Temp Window
	window = CreateWindow(
		windowClass.lpszClassName,
		windowName,
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		100,
		100,
		0,
		0,
		windowClass.hInstance,
		0
	);
	if (!window) {
		return false;
	}
	return true;
}
void gui::DestroyWindow() noexcept
{
	if (window) {
		DestroyWindow(window);
	}
}

bool gui::SetupDirectX() noexcept
{
	const auto handle = GetModuleHandle("d3d9.dll");

	if (!handle)
	{
		return false;
	}

	using CreateFn = LPDIRECT3D9(__stdcall*)(UINT);

	const auto create = reinterpret_cast<CreateFn>(GetProcAddress(handle, "Direct3DCreate9"));

	if (!create) 
	{
		return false;
	}

	d3d9 = create(D3D_SDK_VERSION);

	if (!d3d9)
	{
		return false;
	}

	D3DPRESENT_PARAMETERS params = { };
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	if (d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_NULLREF,
		window,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT,
		&params,
		&device) < 0) return false;

	return true;

}
void gui::DestroyDirectX() noexcept
{
	if (device)
	{
		device->Release();
		device = NULL;
	}
	if (d3d9)
	{
		d3d9->Release();
		d3d9 = NULL;
	}
}

// Setup Device
void gui::Setup()
{
	if (!SetupWindowClass("hackClass001"))
		throw std::runtime_error("Failed to create window class.");

	if (!SetupWindow("Hack Window"))
		throw std::runtime_error("Failed to create window.");

	if (!SetupDirectX())
		throw std::runtime_error("Failed to setup DirectX.");

	DestroyWindow();
	DestroyWindowClass();
}

void CustomStyleColor() // Отрисовка цветов
{
    ImGuiStyle& s = ImGui::GetStyle();

    s.Colors[ImGuiCol_WindowBg] = ImColor(165, 175, 185, 100);
    s.Colors[ImGuiCol_ChildBg] = ImColor(22, 21, 26, 255);
    s.Colors[ImGuiCol_PopupBg] = ImColor(17, 16, 21, 255);
    s.Colors[ImGuiCol_TextDisabled] = ImColor(66, 65, 70, 255);
    s.Colors[ImGuiCol_Border] = ImColor(14, 13, 19, 0);
    s.WindowBorderSize = 0;
    s.WindowPadding = ImVec2(0, 0);
    s.ChildRounding = 7;
    s.PopupRounding = 5;
    s.PopupBorderSize = 0;
    s.WindowRounding = 8.f;
    s.FrameRounding = 4.f;
    s.ScrollbarSize = 2.f;
    s.FramePadding = ImVec2(6, 3);
    s.ItemInnerSpacing = ImVec2(10, 0);
    s.ItemSpacing = ImVec2(0, 10);

}

ImFont* ico_0;
ImFont* ico_1;
ImFont* cruin_0;
ImFont* ubu_1;
ImFont* ubu_0;
ImFont* ubu_2;
ImFont* ubu_preview;
extern ImFont* ubu_child;
ImFont* weapon_font;
static bool menu;
extern IDirect3DTexture9* settings_img = nullptr;
extern IDirect3DTexture9* keyboard_img = nullptr;
extern IDirect3DTexture9* person_img = nullptr;
extern IDirect3DTexture9* visual_img = nullptr;
extern IDirect3DTexture9* rifle_img = nullptr;
extern IDirect3DTexture9* lg = nullptr;


void gui::SetupMenu(LPDIRECT3DDEVICE9 device) noexcept
{
	auto params = D3DDEVICE_CREATION_PARAMETERS{};
	device->GetCreationParameters(&params);

	window = params.hFocusWindow;

	originalWindowProcess = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProcess))
	);

	//Imgui
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.Fonts->AddFontFromMemoryTTF(&ubuntu_1, sizeof ubuntu_1, 15, NULL, io.Fonts->GetGlyphRangesCyrillic());

    ico_0 = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 21, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico_1 = io.Fonts->AddFontFromMemoryTTF(&icon, sizeof icon, 28, NULL, io.Fonts->GetGlyphRangesCyrillic());

    ubu_0 = io.Fonts->AddFontFromMemoryTTF(&ubuntu_0, sizeof ubuntu_0, 18, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ubu_1 = io.Fonts->AddFontFromMemoryTTF(&ubuntu_0, sizeof ubuntu_0, 28, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ubu_2 = io.Fonts->AddFontFromMemoryTTF(&ubuntu_2, sizeof ubuntu_2, 25, NULL, io.Fonts->GetGlyphRangesCyrillic());
    cruin_0 = io.Fonts->AddFontFromMemoryTTF(&cruin, sizeof cruin, 25, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ubu_child = io.Fonts->AddFontFromMemoryTTF(&ubuntu_1, sizeof ubuntu_1, 15, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ubu_preview = io.Fonts->AddFontFromMemoryTTF(&ubuntu_1, sizeof ubuntu_1, 12, NULL, io.Fonts->GetGlyphRangesCyrillic());
    weapon_font = io.Fonts->AddFontFromMemoryTTF(&weapon, sizeof weapon, 14, NULL, io.Fonts->GetGlyphRangesCyrillic());

    CustomStyleColor();

    if (lg == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &logo, sizeof(logo), 35, 35, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &lg);
    if (settings_img == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &settings, sizeof(settings), 16, 16, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &settings_img);
    if (keyboard_img == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &keyboard, sizeof(keyboard), 16, 16, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &keyboard_img);
    if (person_img == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &person, sizeof(person), 16, 16, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &person_img);
    if (visual_img == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &visual, sizeof(visual), 24, 24, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &visual_img);
    if (rifle_img == nullptr) D3DXCreateTextureFromFileInMemoryEx(device, &rifle, sizeof(rifle), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &rifle_img);

    CustomStyleColor();
	setup = true;
}

void gui::Destroy() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// restore wnd proc
	SetWindowLongPtr(
		window,
		GWLP_WNDPROC,
		reinterpret_cast<LONG_PTR>(originalWindowProcess)
	);

	DestroyDirectX();
}
extern bool check_0 = true;
extern bool check_1 = false;
extern int slider_one = 125;
extern int slider_two = 50;
extern float slider_three = 65;
extern char input[64] = { "" };
extern const char* items[] = { "Afghan","Derail","Estate","Favela","Highrise","Invasion","Karachi","Quarry","Rundown","Rust","Scrapyard","Skidrow","Sub Base","Terminal","Underpass","Wasteland","-----DLC MAPS-----","Bailout","Crash","Salvage","Overgrown","Storm","Carnival","Fuel","Strike","Trailer Park","Vacant" };
extern float color_edit[4] = { 64 / 255.f, 77 / 255.f, 236 / 255.f, 190 / 255.f };
extern int selectedItem = 0;
extern float alpha_line = 0.0f;
extern const char* items1[4]{ "Uno", "Dos", "Tres", "Quatro" };
extern int selectedItem1 = 0;

extern bool esp_preview = false;
extern float preview_alpha = 0.0f;

extern const char* multi_items[5] = { "One", "Two", "Three", "Four", "Five" };
extern bool multi_items_count[5] = { "One", "Two", "Three", "Four", "Five" };

extern int min0 = 1, max0 = 50;
extern int ilow0 = 1, ihigh0 = 50;

extern float min1 = 0.0f, max1 = 5.0f;
extern float ilow1 = 0.0f, ihigh1 = 5.0f;

extern float min2 = 0.0f, max2 = 105.0f;
extern float ilow2 = 0.0f, ihigh2 = 105.0f;

extern float tab_alpha = 0.f;
extern float subtab_alpha = 0.f;
extern float keybind_alpha = 0.0f;
extern float tab_add = 0.0f;
extern float subtab_add = 0.0f;
int active_tab = 0;
extern int active_subtab = 0;

extern int tabs = 0;
extern int sub_tabs = 0;

extern float child_sliding = 40.f;
extern int key0 = 0;





void gui::Render() noexcept
{

    DWORD dwFov = *(DWORD*)0xAAC1F8;
    *(float*)(dwFov + 0xC) = 80.0f;

    DWORD dwFPS = *(DWORD*)0x1B90730;
    *(int*)(dwFPS + 0xC) = slider_one;

    ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));

    ImGui::Begin("Hello, world!", &menu, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus);
    {

        const auto& p = ImGui::GetWindowPos();
        ImGuiStyle* s = &ImGui::GetStyle();

        //DrawBackgroundBlur(ImGui::GetWindowDrawList(), g_pd3dDevice);


        const int vtx_idx_1 = ImGui::GetWindowDrawList()->VtxBuffer.Size;
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(192 + p.x, 0 + p.y), ImVec2(WIDTH + p.x, HEIGHT + p.y), ImColor(21, 21, 26, 255), s->WindowRounding, ImDrawFlags_RoundCornersRight);

        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0 + p.x, 0 + p.y), ImVec2(192 + p.x, HEIGHT + p.y), ImColor(35, 32, 45, 200), s->WindowRounding, ImDrawFlags_RoundCornersLeft);
        ImGui::GetWindowDrawList()->AddRect(ImVec2(0 + p.x, 0 + p.y), ImVec2(WIDTH + p.x, HEIGHT + p.y), ImColor(39, 38, 45, 255), s->WindowRounding, ImDrawFlags_None, 1.5f);
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(192 + p.x, 0 + p.y), ImVec2(193.5f + p.x, HEIGHT + p.y), ImColor(39, 38, 45, 255), 0);

        ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x + 192, 45 + p.y), ImVec2(WIDTH + p.x, 45 + p.y), ImColor(38, 37, 43, 255), 0.5f);

        ///////////////////////////////////////////// LOGO + NAME AND DATA

        //ImGui::GetWindowDrawList()->AddImage(lg, ImVec2(735 + p.x, 15 + p.y), ImVec2(770 + p.x, 50 + p.y), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));

        ///////////////////////////////////////////// CHEAT NAME

         ImGui::GetWindowDrawList()->AddText(cruin_0, 25, ImVec2(25 + p.x, 20 + p.y), ImColor(255, 255, 255, 255), "GRIIMS TOOL");

        ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x + 47, p.y + 64), ImVec2(p.x + 91, p.y + 65), ImGui::GetColorU32(c::accent_color, alpha_line), ImGui::GetColorU32(c::accent_color), ImGui::GetColorU32(c::accent_color), ImGui::GetColorU32(c::accent_color, alpha_line));
        ImGui::GetWindowDrawList()->AddRectFilledMultiColor(ImVec2(p.x + 91, p.y + 64), ImVec2(p.x + 141, p.y + 65), ImGui::GetColorU32(c::accent_color), ImGui::GetColorU32(c::accent_color, alpha_line), ImGui::GetColorU32(c::accent_color, alpha_line), ImGui::GetColorU32(c::accent_color));

        ImGui::SetCursorPosY(80);

        ///////////////////////////////////////////// TABS

        ImGui::BeginGroup();
        {
            ImGui::Spacing();

            ImGui::TextColored(ImColor(77, 80, 97, 255), "\tMAIN");

            if (ImGui::Tab("A", "Host", tabs == 0, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 0;

            if (ImGui::Tab("B", "Friends", tabs == 1, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 1;

            //ImGui::Spacing();

            //ImGui::TextColored(ImColor(77, 80, 97, 255), "\tLEGITBOT");

            if (ImGui::Tab("D", "Kick Players", tabs == 3, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 3;

            if (ImGui::Tab("C", "Ban Players", tabs == 4, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 4;

            ImGui::TextColored(ImColor(77, 80, 97, 255), "\tMISC");

            if (ImGui::Tab("E", "Console", tabs == 5, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 5;

            if (ImGui::Tab("F", "About", tabs == 6, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 6;

            //ImGui::Spacing();

            //ImGui::TextColored(ImColor(77, 80, 97, 255), "\tVISUALS");                 

            //if (ImGui::Tab("C", "World", tabs == 8, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 8;

            //if (ImGui::Tab("E", "More", tabs == 9, ImVec2(200, 37), true, ImGuiButtonFlags_None)) tabs = 9;

        }
        ImGui::EndGroup();

       tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
        if (tab_alpha == 0.f && tab_add == 0.f) active_tab = tabs;

        subtab_alpha = ImClamp(subtab_alpha + (4.f * ImGui::GetIO().DeltaTime * (sub_tabs == active_subtab ? 1.f : -1.f)), 0.f, 1.f);
        if (subtab_alpha == 0.f && subtab_add == 0.f) active_subtab = sub_tabs;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * s->Alpha);

        ImGui::SetCursorPos(ImVec2(205, 62));

        active_tab = tabs;

        ImGui::BeginChild("General-CHILD", ImVec2(566, 495), true, ImGuiWindowFlags_NoBackground);
        {


            if (active_tab == 0)//page0
            {

                ImGui::BeginGroup(); // START GROUP
                {
                    ImGui::BeginChildPos(settings_img, "Main", ImVec2(272, 340), true);
                    {

                        ImGui::Checkbox("Force Host", &check_0);
                        ImGui::Checkbox("OneShotSnipers", &check_0);
                        ImGui::Checkbox("Anti-Noob", &check_0);
                        ImGui::Checkbox("DisableStreaks", &check_0);
                        ImGui::Checkbox("DisableEquipment", &check_0);
                        ImGui::Checkbox("DisablePainKiller", &check_0);

                    }
                    ImGui::EndChildPos();
                }
                ImGui::EndGroup();
                ImGui::SameLine(0, 10); // 1, 2
                ImGui::BeginGroup();
                {

                    ImGui::BeginChildPos(settings_img, "Visual", ImVec2(272, 340), true);
                    {
                        ImGui::SliderInt("FPS", &slider_one, 125, 333, "%d");
                        ImGui::SliderFloat("FOV", &slider_three, 65, 120, "%d%");
                        ImGui::InputTextWithHint("Spoof Name", "Enter name.", input, 64);
                        ImGui::Combo("Change Map", &selectedItem, items, IM_ARRAYSIZE(items), 5);
                        if (ImGui::Button("Change Map", ImVec2(90, 25))); ImGui::SameLine(); ImGui::Button("Fast Restart", ImVec2(90, 25));



                    }
                    ImGui::EndChildPos();

                    /*ImGui::BeginChildPos(settings_img, "POS1", ImVec2(272, 340), true);
                    {

                        ImGui::Checkbox("POS2", &check_0);

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::InputTextWithHint("Textfield", "indoor millionaire", input, 64);

                        ImGui::Combo("Combobox", &selectedItem, items, IM_ARRAYSIZE(items), 5);

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "POS3", ImVec2(272, 132), true, true);
                    {

                        if (ImGui::Button("Button", ImVec2(227, 37)));

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Other", ImVec2(272, 260), true);
                    {

                        ImGui::Checkbox("Checkbox one", &check_1);

                        ImGui::Checkbox("Checkbox two", &check_0);

                        ImGui::Checkbox("Esp preview", &esp_preview);



                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Exploit", ImVec2(272, 207), true);
                    {

                        ImGui::Combo("Combobox ", &selectedItem1, items1, 4);

                    }
                    ImGui::EndChildPos();
                }
                ImGui::EndGroup();

                ImGui::SameLine(0, 10); // 1, 2

                ImGui::BeginGroup();
                {

                    ImGui::BeginChildPos(settings_img, "Main", ImVec2(272, 242), true);
                    {

                        ImGui::Checkbox("Checkbox ", &check_1);

                        ImGui::ColorEdit4("Colorpicker##0", (float*)&c::accent_color, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);

                        ImGui::Keybind("Binderbox", &key0, true);

                        if (ImGui::Button("Button", ImVec2(227, 37)));

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Settings", ImVec2(272, 230), true);
                    {

                        ImGui::SliderInt("Slider", &slider_two, 1, 100, "%du/s");

                        ImGui::MultiCombo("Multi Combo", multi_items_count, multi_items, 5);

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Header", ImVec2(272, 260), true);
                    {

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "World", ImVec2(272, 205), true);
                    {

                    }
                    ImGui::EndChildPos();


                }
                */
                }
                ImGui::EndGroup(); // END GROUP*/
            }
            if (active_tab == 1) //page1
            {
                ImGui::BeginGroup(); // START GROUP
                {
                    ImGui::BeginChildPos(settings_img, "General", ImVec2(272, 340), true);
                    {

                        ImGui::Checkbox("Checkbox", &check_0);

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::InputTextWithHint("Textfield", "indoor millionaire", input, 64);

                        ImGui::Combo("Combobox", &selectedItem, items, IM_ARRAYSIZE(items), 5);

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Misc", ImVec2(272, 132), true);
                    {

                        if (ImGui::Button("Button", ImVec2(227, 37)));

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Other", ImVec2(272, 260), true);
                    {

                        ImGui::Checkbox("Checkbox one", &check_1);

                        ImGui::Checkbox("Checkbox two", &check_0);

                        ImGui::Checkbox("Esp preview", &esp_preview);



                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Exploit", ImVec2(272, 207), true);
                    {

                        ImGui::Combo("Combobox ", &selectedItem1, items1, 4);

                    }
                    ImGui::EndChildPos();
                }
                ImGui::EndGroup();

                ImGui::SameLine(0, 10); // 1, 2

                ImGui::BeginGroup();
                {

                    ImGui::BeginChildPos(settings_img, "Main", ImVec2(272, 242), true);
                    {

                        ImGui::Checkbox("Reful Ammo ", &check_1);

                        ImGui::ColorEdit4("Colorpicker##0", (float*)&c::accent_color, ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs);

                        ImGui::Keybind("Binderbox", &key0, true);

                        if (ImGui::Button("CLick me", ImVec2(227, 37)));

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Settings", ImVec2(272, 230), true);
                    {

                        ImGui::SliderInt("Slider", &slider_two, 1, 100, "%du/s");

                        ImGui::MultiCombo("Multi Combo", multi_items_count, multi_items, 5);

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "Header", ImVec2(272, 260), true);
                    {

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                        ImGui::SliderInt("Slider", &slider_one, 1, 100, "%d%%");

                    }
                    ImGui::EndChildPos();


                    ImGui::BeginChildPos(settings_img, "World", ImVec2(272, 205), true);
                    {

                    }
                    ImGui::EndChildPos();


                }
                ImGui::EndGroup(); // END GROUP*/
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}


LRESULT CALLBACK WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParam,
	LPARAM longParam
)
{
	// toggle menu
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		gui::open = !gui::open;
        menu = true;
	}
    if (gui::open)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = true;
        *reinterpret_cast<int**>(0x6427D3D) = nullptr; // Unhook mouse
        menu = false;
    }
    else
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDrawCursor = false;
        *reinterpret_cast<int*>(0x6427D3D) = 1; // Release mouse
    }
	// Pass Messages to Imgui
	if (gui::open && ImGui_ImplWin32_WndProcHandler(
		window,
		message,
		wideParam,
		longParam
	)) return 1L;

	return CallWindowProc(
		gui::originalWindowProcess,
		window,
		message,
		wideParam,
		longParam
	);
}