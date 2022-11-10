#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <filesystem>
#include <Windows.h>
#include <KnownFolders.h>
#include <shlobj.h>
#include <sstream>
#include "Walnut/Image.h"
#include <iostream>
#include <fstream>
#include <string>

namespace fs = std::filesystem;
using namespace std;
namespace GUI_Cfg {
	bool bTaskInProgress;
	int iCurrentTab;
	int iSelectedTask;
	bool b_wConfirmQuit;
	int netCheck = system("ping youtube.com -n 1");
	vector<std::string> tasks;
	bool bJustLaunched{ true };
	bool bNeedInfo{ false };
}



ImVec2 GetRes()
{
	RECT desktop;
	const HWND hDesktop = GetDesktopWindow();
	GetWindowRect(hDesktop, &desktop);
	int x = desktop.right;
	int y = desktop.bottom;
	return ImVec2(float(x),float(y));
}


void HELPERS_EraseSubStr(std::string& mainStr, const std::string& toErase)
{
	size_t pos = mainStr.find(toErase);
	if (pos != std::string::npos)
	{
		mainStr.erase(pos, toErase.length());
	}
}


[[nodiscard]] static std::filesystem::path MISC_GetFolder(){
	std::filesystem::path path{};
	if (PWSTR pathToRoaming; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathToRoaming))) {
		path = pathToRoaming;
		CoTaskMemFree(pathToRoaming);
	}
	path /= "AltSPU";
	std::error_code ec; std::filesystem::create_directory(path, ec);
	return path;
}

vector<std::string> MISC_TasksInput(std::string taskname) {
	std::ifstream infile(MISC_GetFolder()/=taskname);
	vector<std::string> contents;
	std::string line;
	while(std::getline(infile, line))
		if (line.size() > 0)
			contents.push_back(line);
	return contents;
}

void GUI_PreTask(vector<std::string> vInfo) {
	ImGui::SetCursorPos(ImVec2(20, 95));
		for (int i = 0; i < 5; i++) {
			ImGui::SetCursorPos(ImVec2(8, ImGui::GetCursorPosY() + 15));
			ImGui::Text((vInfo[GUI_Cfg::netCheck + i].substr(0, vInfo[GUI_Cfg::netCheck + i].find("-"))).c_str());
			ImGui::SetCursorPosX(20);
			ImGui::Text((vInfo[GUI_Cfg::netCheck + i].substr(vInfo[GUI_Cfg::netCheck + i].find("-") + 1)).c_str());
		};

	ImGui::Text(std::to_string(vInfo.size()/5).c_str());
	ImGui::SetCursorPos(ImVec2(8, GetRes().y - 115));
	if (ImGui::Button("<", ImVec2(125, 50)))
		if (GUI_Cfg::netCheck != 0)
		GUI_Cfg::netCheck-=5;
	ImGui::SetCursorPos(ImVec2(GetRes().x-375, GetRes().y - 115));
	if (ImGui::Button(">", ImVec2(125, 50)))
		if (GUI_Cfg::netCheck != vInfo.size()-5)
			GUI_Cfg::netCheck+=5;

}

void GUI_ConfirmQuit() {
	if (!GUI_Cfg::b_wConfirmQuit)
		return;
	ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(GetRes().x / 2.f - 200.f, GetRes().y / 2.f - 75.f));
	ImGui::Begin(u8"Вы уверены, что хотите выйти?", &GUI_Cfg::b_wConfirmQuit, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	ImGui::Text(u8"Вы потеряете весь прогресс в упражнении"); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.2f, 0.2f, 1.0f, 1.0f), (std::to_string(GUI_Cfg::iSelectedTask + 1)).c_str());
	ImGui::SetCursorPos(ImVec2(8, 82));
	if (ImGui::Button(u8"Выйти", ImVec2(188, 60)))
		system("taskkill /f /im sloo.exe");
	ImGui::SameLine();
	if (ImGui::Button(u8"Отмена",ImVec2(188, 60))){
		GUI_Cfg::b_wConfirmQuit = false;}
	ImGui::End();
}

void GUI_CantConnect() {
	system("cls");
	std::cout << "No internet\n\n\n";
	system("pause");
	return;
}

void GUI_TaskListInfo() {
	if (!GUI_Cfg::bNeedInfo)
		return;
	ImGui::SetCursorScreenPos(ImVec2(GetRes().x - 140, 140));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));
	ImGui::BeginChild("Task_Info", ImVec2(0, 0), true);
	ImGui::PopStyleColor();
	ImGui::Text(u8"Задание #"); ImGui::SameLine(); ImGui::Text((std::to_string(GUI_Cfg::iSelectedTask+1)).c_str());
	ImGui::Button(u8"Приступить\nк\nвыполнению", ImVec2(130, 80));
	ImGui::EndChild();
}

void GUI_TaskList() {
	GUI_TaskListInfo();
	ImGui::SetCursorScreenPos(ImVec2(GetRes().x - 140, 10));
	if (ImGui::Button(u8"Перезагрузить", ImVec2(130, 130)) || GUI_Cfg::bJustLaunched) {
		GUI_Cfg::tasks.clear();
		for (const auto& entry : fs::directory_iterator(MISC_GetFolder()))
				GUI_Cfg::tasks.push_back(entry.path().u8string());
		GUI_Cfg::bJustLaunched = false;
	}
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.f);
	ImGui::SetCursorPos(ImVec2(8, 8));
	for (int task = 0; task<GUI_Cfg::tasks.size(); task++)
	{
		std::string taskName = GUI_Cfg::tasks[task].c_str();
		HELPERS_EraseSubStr(taskName, (MISC_GetFolder().u8string()+"\\"));
		if (ImGui::Button(taskName.c_str(), ImVec2(132, 132)))
		{
			GUI_Cfg::bNeedInfo = true;
			GUI_Cfg::iSelectedTask = task;
		}
		if ((task + 1) / 6 != (task + 1) % 6)
		{
			ImGui::SameLine();
		}
		else ImGui::SetCursorPosX(8);
	}
	ImGui::PopStyleVar();
}

void GUI_Options() {
	ImGui::Text((u8"Папка: " + MISC_GetFolder().u8string()).c_str());
	int wrd{ 0 };
	ImGui::SliderInt(u8"Словаа", &wrd, 0, 100);
	ImGui::Text(MISC_TasksInput("t1.txt")[wrd].c_str());
	GUI_PreTask(MISC_TasksInput("t1.txt"));
}
void GUI_Web() {
}
class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f,0.f));
		ImGui::SetNextWindowSize(ImVec2(GetRes().x, GetRes().y-40.f), ImGuiCond_Once);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
		ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoBringToFrontOnFocus |ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);
		ImGui::SetCursorPos(ImVec2(GetRes().x - 27.f, -1));

		
		ImGui::SetCursorPos(ImVec2(-2,0));		//sidebar bleh
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.65f, 0.65f, 0.70f, 1.0f));
		ImGui::BeginChild("Sidebar", ImVec2(GetRes().x/6.f,GetRes().y-40.f), true);
		ImGui::PopStyleColor();

		ImGui::SetCursorPosX(7);
		ImGui::Text(u8"словарь");

		ImGui::SetCursorPosY(40.f);
		if (ImGui::Button(u8"Список упражнений", ImVec2(GetRes().x / 6.f, 100)))
			GUI_Cfg::iCurrentTab = 0;

		ImGui::SetCursorPosY(140.f);
		if (ImGui::Button(u8"Поиск упражнений\nв интернете", ImVec2(GetRes().x / 6.f, 100)))
			GUI_Cfg::iCurrentTab = 1;

		ImGui::SetCursorPosY(240.f);
		if (ImGui::Button(u8"Настройки", ImVec2(GetRes().x / 6.f, 100)))
			GUI_Cfg::iCurrentTab = 2;

		ImGui::SetCursorPos(ImVec2(0, GetRes().y - 140));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.55f, 0.55f, 0.55f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
		if (ImGui::Button(u8"Выйти", ImVec2(GetRes().x / 6.f, 100))) //close button duh
			GUI_Cfg::b_wConfirmQuit=true;
		ImGui::PopStyleColor(2);
		ImGui::EndChild();

		ImGui::SetCursorPos(ImVec2(GetRes().x / 6.f + 8.f, 8));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 3.f);
		ImGui::BeginChild("MainChild", ImVec2(GetRes().x - GetRes().x / 6.f-16, GetRes().y - 56.f), true);
		ImGui::PopStyleVar();
		switch (GUI_Cfg::iCurrentTab) {
		case 0:GUI_TaskList(); break;
		case 2:GUI_Options(); break;
		case 1:GUI_Web(); break;
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::End();
		GUI_ConfirmQuit();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Base";
	spec.Height = 1;
	spec.Width = 1;
	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
/*	if (GUI_Cfg::netCheck != 0)
		GUI_CantConnect();
	else */return app;
}