#include "GUI.hpp"

/*****************************************************************************
* GUI Initialization *********************************************************
*  - utilizes Dear ImGui by ocornut and ImGuiFileDialog by aiekick on Github *
******************************************************************************/

const char* glsl_version = "#version 330";

// Context derived from aiekick's example
void GUI::initIMGUIcontext(GLFWwindow *window) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontAllowUserScaling = true; // zoom wiht ctrl + mouse wheel 

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// load icon font file (CustomFont.cpp)
	ImGui::GetIO().Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);

	// Our state - moved below
	//bool show_demo_window = true;
	//bool show_another_window = false;
	//ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	igfd::ImGuiFileDialog::Instance()->SetExtentionInfos(".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]"); // add an text for a filter type

#ifdef USE_BOOKMARK
	// load bookmarks
	std::ifstream docFile("bookmarks.conf", std::ios::in);
	if (docFile.is_open())
	{
		std::stringstream strStream;
		strStream << docFile.rdbuf();//read the file
		igfd::ImGuiFileDialog::Instance()->DeserializeBookmarks(strStream.str());
		docFile.close();
	}
#endif

}


/**********************************************************************
* ImGuiFileDialog Demo Setup ******************************************
*  - directly from aiekick on Github, utilizing Dear ImGui by ocornut *
***********************************************************************/

static bool canValidateDialog = false;

inline void InfosPane(std::string vFilter, igfd::UserDatas vUserDatas, bool *vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");

	ImGui::Text("Selected Filter : %s", vFilter.c_str());

	const char* userDatas = (const char*)vUserDatas;
	if (userDatas)
		ImGui::Text("User Datas : %s", userDatas);

	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

	if (vCantContinue)
		*vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, bool active, bool disabled)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	float w = CalcItemWidth();
	if (w == window->ItemWidthDefault)	w = 0.0f; // no push item width
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	ImVec2 bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
	bb_size.x = ImMax(w, bb_size.x);

	const ImRect check_bb(
		window->DC.CursorPos,
		window->DC.CursorPos + bb_size);
	ItemSize(check_bb, style.FramePadding.y);

	if (!ItemAdd(check_bb, id))
		return false;

	// check
	bool pressed = false;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(check_bb, id, &hovered, &held);

		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
		if (active)
		{
			const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
		}
	}

	// circle shadow + bg
	if (style.FrameBorderSize > 0.0f)
	{
		window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
		window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
	}

	if (label_size.x > 0.0f)
	{
		RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
	}

	return pressed;
}


// Their variables - TODO: delete later
bool show_demo_window = true;
bool show_another_window = false;
ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void GUI::imguiFileDemo(GLFWwindow *window) {
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that [ImGuiFileDialog writers] create [themselves]. We use a Begin/End pair to created a named window.
	{
		ImGui::Begin("imGuiFileDialog Demo");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Separator();

		ImGui::Text("imGuiFileDialog Demo %s : ", IMGUIFILEDIALOG_VERSION);
		ImGui::Indent();
		{
#ifdef USE_EXPLORATION_BY_KEYS
			static float flashingAttenuationInSeconds = 1.0f;
			if (ImGui::Button("R##resetflashlifetime"))
			{
				flashingAttenuationInSeconds = 1.0f;
				igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(200);
			if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f))
				igfd::ImGuiFileDialog::Instance()->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
			ImGui::PopItemWidth();
#endif
			static bool _UseWindowContraints = true;
			ImGui::Separator();
			ImGui::Checkbox("Use file dialog constraint", &_UseWindowContraints);
			ImGui::Text("Constraints is used here for define min/max file dialog size");
			ImGui::Separator();
			static bool standardDialogMode = false;
			ImGui::Text("Open Mode : ");
			ImGui::SameLine();
			if (RadioButtonLabeled("Standard", standardDialogMode, false)) standardDialogMode = true;
			ImGui::SameLine();
			if (RadioButtonLabeled("Modal", !standardDialogMode, false)) standardDialogMode = false;

			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog"))
			{
				const char *filters = ".*,.cpp,.h,.hpp";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters"))
			{
				const char *filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".");
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items"))
			{
				const char *filters = ".*,.cpp,.h,.hpp";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 5);
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection"))
			{
				const char *filters = ".*,.cpp,.h,.hpp";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open All file types with filter .*"))
			{
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", 5);
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", 5);
			}
			if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane"))
			{
				const char *filters = "C++ File (*.cpp){.cpp}";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_SAVE " Choose a File", filters,
						".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
							std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_SAVE " Choose a File", filters,
						".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
							std::placeholders::_3), 350, 1, igfd::UserDatas("SaveFile"));
			}
			if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with Confirm Dialog For Overwrite File if exist"))
			{
				const char* filters = "C++ File (*.cpp){.cpp}";
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey",
						ICON_IGFD_SAVE " Choose a File", filters,
						".", "", 1, igfd::UserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
						ICON_IGFD_SAVE " Choose a File", filters,
						".", 1, igfd::UserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog"))
			{
				// set filters to 0 for open directory chooser
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
			}
			if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items"))
			{
				// set filters to 0 for open directory chooser
				if (standardDialogMode)
					igfd::ImGuiFileDialog::Instance()->OpenDialog("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
				else
					igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
						ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".", 5);
			}
			ImVec2 minSize = ImVec2(0, 0);
			ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

			if (_UseWindowContraints)
			{
				maxSize = ImVec2((float)display_w, (float)display_h);
				minSize = maxSize * 0.5f;
			}

			// you can define your flags and min/max window size (theses three settings ae defined by default :
			// flags => ImGuiWindowFlags_NoCollapse
			// minSize => 0,0
			// maxSize => FLT_MAX, FLT_MAX (defined is float.h)

			static std::string filePathName = "";
			static std::string filePath = "";
			static std::string filter = "";
			static std::string userDatas = "";
			static std::vector<std::pair<std::string, std::string>> selection = {};

			if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey",
				ImGuiWindowFlags_NoCollapse, minSize, maxSize))
			{
				if (igfd::ImGuiFileDialog::Instance()->IsOk)
				{
					filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
					filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
					filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
					// here convert from string because a string was passed as a userDatas, but it can be what you want
					if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
						userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
					auto sel = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection
					selection.clear();
					for (auto s : sel)
					{
						selection.emplace_back(s.first, s.second);
					}
					// action
				}
				igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
			}

			if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseDirDlgKey",
				ImGuiWindowFlags_NoCollapse, minSize, maxSize))
			{
				if (igfd::ImGuiFileDialog::Instance()->IsOk)
				{
					filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
					filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
					filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
					// here convert from string because a string was passed as a userDatas, but it can be what you want
					if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
						userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
					auto sel = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection
					selection.clear();
					for (auto s : sel)
					{
						selection.emplace_back(s.first, s.second);
					}
					// action
				}
				igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseDirDlgKey");
			}

			ImGui::Separator();

			ImGui::Text("ImGuiFileDialog Return's :\n");
			ImGui::Indent();
			{
				ImGui::Text("GetFilePathName() : %s", filePathName.c_str());
				ImGui::Text("GetFilePath() : %s", filePath.c_str());
				ImGui::Text("GetCurrentFilter() : %s", filter.c_str());
				ImGui::Text("GetUserDatas() (was a std::string in this sample) : %s", userDatas.c_str());
				ImGui::Text("GetSelection() : ");
				ImGui::Indent();
				{
					static int selected = false;
					if (ImGui::BeginTable("##GetSelection", 2,
						ImGuiTableFlags_SizingPolicyFixed | ImGuiTableFlags_RowBg |
						ImGuiTableFlags_ScrollY))
					{
						ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
						ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch, -1, 0);
						ImGui::TableSetupColumn("File Path name", ImGuiTableColumnFlags_WidthAuto, -1, 1);
						ImGui::TableHeadersRow();

						ImGuiListClipper clipper;
						clipper.Begin((int)selection.size(), ImGui::GetTextLineHeightWithSpacing());
						while (clipper.Step())
						{
							for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
							{
								const auto& sel = selection[i];
								ImGui::TableNextRow();
								if (ImGui::TableSetColumnIndex(0)) // first column
								{
									ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
									selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
									if (ImGui::Selectable(sel.first.c_str(), i == selected, selectableFlags)) selected = i;
								}
								if (ImGui::TableSetColumnIndex(1)) // second column
								{
									ImGui::Text("%s", sel.second.c_str());
								}
							}
						}
						clipper.End();

						ImGui::EndTable();
					}
				}
				ImGui::Unindent();
			}
			ImGui::Unindent();
		}
		ImGui::Unindent();


		ImGui::Separator();
		ImGui::Text("Window mode :");
		ImGui::Separator();

		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	//glfwSwapBuffers(window);
}


/*******************
* Space Management *
*******************/
glm::vec2 GUI::getXRange() {
	return xRange;
}

glm::vec2 GUI::getYRange() {
	return yRange;
}

glm::vec2 GUI::getZRange() {
	return zRange;
}

glm::vec2 GUI::getARange() {
	return aRange;
}

bool GUI::getDye() {
	return dye;
}

void updateRange(int min, int max, glm::vec2* vec, float range) {
	float r2 = range / 2.0f;
	// TODO Might not need the extra scene_spacing multiplication in the middle
	vec->x = (min * Splats::spacing() - r2 + (-1.0f + 0.000005f) * Splats::spacing()) / Splats::scale() * 2.0f;
	vec->y = (max * Splats::spacing() - r2 + 0.000005f * Splats::spacing()) / Splats::scale() * 2.0f;
}

void GUI::updateXRange(int min, int max) {
	updateRange(min, max, &xRange, Splats::width());
}

void GUI::updateYRange(int min, int max) {
	updateRange(min, max, &yRange, Splats::length());
}

void GUI::updateZRange(int min, int max) {
	updateRange(min, max, &zRange, Splats::height());
}

void GUI::updateARange(int min, int max) {
	aRange.x = min;
	aRange.y = max;
}

void GUI::resetRanges() {
	xRange = glm::vec2(-Splats::width() / Splats::scale(),
		Splats::width() / Splats::scale());
	yRange = glm::vec2(-Splats::length() / Splats::scale(),
		Splats::length() / Splats::scale());
	zRange = glm::vec2(-Splats::height() / Splats::scale(),
		Splats::height() / Splats::scale());
	aRange = glm::vec2(-100, 1024);

	begin_W = 1; 
	begin_L = 1;
	begin_H = 1;
	begin_A = -100;
	end_W = Splats::width() / Splats::spacing();
	end_L = Splats::length() / Splats::spacing();
	end_H = Splats::height() / Splats::spacing();
	end_A = 1024;
}

/*******************
* Custom GUI Setup *
*******************/

bool GUI::DragIntRangeCustom(bool* drag, const char* name, int* v_current_min, int* v_current_max,
	int v_min, int v_max, int color, const char* format, const char* format_max, ImGuiSliderFlags flags)
{
	ImGuiWindow* win = ImGui::GetCurrentWindow();
	if (win->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGui::PushID(name);

	// Add color to the sliders
	ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(color / 7.0f, 0.5f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(color / 7.0f, 0.6f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(color / 7.0f, 0.7f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, (ImVec4)ImColor::HSV(color / 7.0f, 0.9f, 0.9f));

	ImGui::BeginGroup();
	ImGui::PushMultiItemsWidths(2, ImGui::GetContentRegionMax().x - g.Style.ItemSpacing.x);

	// Reset mins and maxes - ImGui sometimes went out of range
	*v_current_min = std::max(v_min, std::min(*v_current_min, *v_current_max));
	*v_current_max = std::min(std::max(*v_current_min, *v_current_max), v_max);

	// Min-Defining Slider
	int min_min = v_min;
	int min_max = ImMin(v_max, *v_current_max);
	bool value_changed = ImGui::SliderInt("##min", v_current_min, 
		min_min, min_max, format, flags);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);

	// Max-Defining Slider
	int max_min = ImMax(v_min, *v_current_min);
	int max_max = v_max;
	value_changed |= ImGui::SliderInt("##max", v_current_max, 
		max_min, max_max, format_max ? format_max : format, flags);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
	ImGui::EndGroup();

	// See if the user is still holding down, even if away from the window
	if (ImGui::IsItemActive()) {
		*drag = true;
		selectedFile = -1;
	}

	ImGui::PopStyleColor(4);
	ImGui::PopID();

	return value_changed;
}

bool GUI::SliceSlider(bool* drag, const char* name, int* value, int* toAdjust,
	int min, int max, int color, const char* format, ImGuiSliderFlags flags) {
	
	ImGuiContext& g = *GImGui;
	ImGui::PushItemWidth(ImGui::GetContentRegionMax().x - g.Style.ItemSpacing.x);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, -3.5f));
	
	// Add color to the slider
	ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(color / 7.0f, 0.3f, 0.2f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImVec4)ImColor::HSV(color / 7.0f, 0.4f, 0.3f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImVec4)ImColor::HSV(color / 7.0f, 0.7f, 0.5f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, (ImVec4)ImColor::HSV(color / 7.0f, 0.9f, 0.9f));
	
	bool value_changed = ImGui::SliderInt(name, value, min, max, "", 
		ImGuiSliderFlags_AlwaysClamp);

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar();
	ImGui::PopItemWidth();

	if (ImGui::IsItemActive()) {
		*drag = true;
		selectedFile = -1;
	}

	if (value_changed) { *toAdjust = *value; }
	return value_changed;
}

bool GUI::myGUI(GLFWwindow *window, GLuint * program, const unsigned int PROG_BOID) {

	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	bool newResults = false;
	bool createdNewScene = false;

	// FILE DIALOG - Lets the user select DICOM(s)
	if (ImGui::BeginMainMenuBar())
	{
		// Setting up the Menu Buttons
		if (ImGui::BeginMenu(ICON_IGFD_FOLDER_OPEN " Load DICOM"))
		{
			// For selecting by file
			if (ImGui::MenuItem("Select DICOM File(s)"))
			{
				const char *filters = ".dcm";
				igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey",
					ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", 0);
			}
			ImGui::Separator();

			// For selecting a single directory
			if (ImGui::MenuItem("Select DICOM Directory"))
			{
				// set filters to 0 for open directory chooser
				igfd::ImGuiFileDialog::Instance()->OpenModal("ChooseDirDlgKey",
					ICON_IGFD_FOLDER_OPEN " Choose a Directory", 0, ".");
			}

			ImGui::EndMenu();
		}

		// Defining possible window size
		/// flags => ImGuiWindowFlags_NoCollapse // the default
		ImVec2 maxSize = ImVec2((float)display_w, (float)display_h);
		ImVec2 minSize = maxSize * 0.5f;

		// Outputs of the file dialog
		static std::string filePathName = "";
		static std::string filePath = "";
		static std::string filter = "";
		static std::string userDatas = "";
		static std::vector<std::pair<std::string, std::string>> selection = {};

		// Linking the file dialogs to each button
		// For Files:
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseFileDlgKey",
			ImGuiWindowFlags_NoCollapse, minSize, maxSize))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk)
			{
				filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
				filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
				// here convert from string because a string was passed as a userDatas, but it can be what you want
				if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
					userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
				auto sel = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection
				selection.clear();
				for (auto s : sel)
				{
					selection.emplace_back(s.first, s.second);
				}
				// action
				ITKReader::loadFiles(std::move(selection));//selection);
				newResults = true;
				createdNewScene = true;
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseFileDlgKey");
		}

		// For a Directory:
		if (igfd::ImGuiFileDialog::Instance()->FileDialog("ChooseDirDlgKey",
			ImGuiWindowFlags_NoCollapse, minSize, maxSize))
		{
			if (igfd::ImGuiFileDialog::Instance()->IsOk)
			{
				filePathName = igfd::ImGuiFileDialog::Instance()->GetFilePathName();
				filePath = igfd::ImGuiFileDialog::Instance()->GetCurrentPath();
				filter = "";
				//igfd::ImGuiFileDialog::Instance()->Filter
				//filter = igfd::ImGuiFileDialog::Instance()->GetCurrentFilter();
				// here convert from string because a string was passed as a userDatas, but it can be what you want
				if (igfd::ImGuiFileDialog::Instance()->GetUserDatas())
					userDatas = std::string((const char*)igfd::ImGuiFileDialog::Instance()->GetUserDatas());
				auto sel = igfd::ImGuiFileDialog::Instance()->GetSelection(); // multiselection
				selection.clear();
				for (auto s : sel)
				{
					selection.emplace_back(s.first, s.second);
				}
				// Process the files
				// TODO Compare if last /.../ entry of PathName = last entry of Path, if not - append
				ITKReader::loadDirectory(filePath);
				newResults = true;
				createdNewScene = true; // TODO maybe merge these two booleans
			}
			igfd::ImGuiFileDialog::Instance()->CloseDialog("ChooseDirDlgKey");
		}

		ImGui::EndMainMenuBar();
	}

	ImGuiContext& g = *GImGui;

	// EDITOR WINDOW Allows the user to control which ranges to view
	{
		// Initialize the right position ... 
		ImGui::SetNextWindowPos(ImVec2(display_w - 300, 50), ImGuiCond_Once);
		
		ImGui::Begin("View Editor");
		editWindow = ImGui::GetCurrentWindow();

		// ... and size
		ImGui::SetWindowSize(ImVec2(260, 195), ImGuiCond_Once);

		ImGui::Text(("Scene Spacing: " + std::to_string(Splats::spacing())).c_str());

		// To see if any are currently being dragged
		bool drag = false;

		// The X-Range Sliders
		DragIntRangeCustom(&drag, "XRange", &begin_W, &end_W, 1, 
			Splats::width() / Splats::spacing(), 0, 
			"Min X: %d units", "Max X: %d units");
		if (SliceSlider(&drag, "##singleX", &begin_W, &end_W, 1,
			Splats::width() / Splats::spacing(), 0)) {
			// TODO: for each of these, check that it wasn't in slideMode or something before
			begin_L = 1;
			begin_H = 1;
			end_L = Splats::length() / Splats::spacing();
			end_H = Splats::height() / Splats::spacing();
		}

		// The Y-Range Sliders
		DragIntRangeCustom(&drag, "YRange", &begin_L, &end_L, 1, 
			Splats::length() / Splats::spacing(), 2, 
			"Min Y: %d units", "Max Y: %d units");
		if (SliceSlider(&drag, "##singleY", &begin_L, &end_L, 1,
			Splats::length() / Splats::spacing(), 2)) {
			begin_W = 1;
			begin_H = 1;
			end_W = Splats::width() / Splats::spacing();
			end_H = Splats::height() / Splats::spacing();
		}

		// The Z-Range Sliders
		DragIntRangeCustom(&drag, "ZRange", &begin_H, &end_H, 1,
			Splats::height() / Splats::spacing(), 4,
			"Min Z: %d units", "Max Z: %d units");
		if (SliceSlider(&drag, "##singleZ", &begin_H, &end_H, 1,
			Splats::height() / Splats::spacing(), 4)) {
			begin_W = 1;
			begin_L = 1;
			end_W = Splats::width() / Splats::spacing();
			end_L = Splats::length() / Splats::spacing();
		}

		// The Alpha-Range Sliders, using min and max possible DCM values
		DragIntRangeCustom(&drag, "ARange", &begin_A, &end_A, -1024,
			3072, 1, "Range: %d", "to %d");
		isDragging = drag;

		// TODO maybe move to IsItemActive() conditional inside DragIntRangeCustom
		updateXRange(begin_W, end_W);
		updateYRange(begin_L, end_L);
		updateZRange(begin_H, end_H);
		updateARange(begin_A, end_A);

		ImGui::BeginGroup();
		ImGui::PushMultiItemsWidths(3, ImGui::GetContentRegionMax().x - g.Style.ItemSpacing.x);

		// When clicked, points the camera at the center of (visible) mass
		if (ImGui::Button("Center View")) {
			CameraControls::centerView((xRange.x + xRange.y) / 2.0,
				(yRange.x + yRange.y) / 2.0,
				(zRange.x + zRange.y) / 2.0);
			CameraControls::updateCamera(program, PROG_BOID, display_w, display_h);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);

		// When clicked, resets all ranges
		if (ImGui::Button("Reset")) {
			resetRanges();
			// Also Center View
			CameraControls::centerView((xRange.x + xRange.y) / 2.0,
				(yRange.x + yRange.y) / 2.0,
				(zRange.x + zRange.y) / 2.0);
			CameraControls::updateCamera(program, PROG_BOID, display_w, display_h);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);

		// When clicked, gives values over 1680 a whiter color (TODO give user color choice)
		if (ImGui::Checkbox("Show Dye", &dye)) {
			loadDye(program, PROG_BOID);
		}
		ImGui::PopItemWidth();
		ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
		ImGui::EndGroup();

		ImGui::End();
	}

	// DICOM WINDOW Allows the user to see which files were selected
	if (ITKReader::hasFiles()) {
		ImGui::Begin("DICOM Files");
		
		//int numFiles = ITKReader::numFiles();
		//for (int i = 0; i < numFiles; i++) {
		// TODO - Replace w/ above, This is for the low res mode
		int numFiles = std::max(1.0f, ITKReader::numFiles() / Splats::spacing());
		// TODO: the numfiles might not be filled completely at this point?
		
		// Initialize the right position when resetting
		if (newResults && listWindow != nullptr) {
			ImGui::SetWindowPos(ImVec2(listWindow->Pos.x, listWindow->Pos.y));
			ImGui::SetWindowSize(ImVec2(listWindow->Size.x, listWindow->Size.y));
		}
		else if (listWindow == nullptr) {
			ImGui::SetWindowPos(ImVec2(15, 35), ImGuiCond_Once);
			ImGui::SetWindowSize(ImVec2(150, 300), ImGuiCond_Once);
			listWindow = ImGui::GetCurrentWindow();
		}

		// Initialize the right size
		if ((newResults && listWindow != nullptr) || listWindow == nullptr) {
			// Calculating the size of each file label for reference
			std::string exampleLabel = std::to_string(numFiles) +
				": " + ITKReader::fileNameAt(0);
			ImVec2 textSize = ImGui::CalcTextSize(exampleLabel.c_str(), NULL, true);

			// Get the dimensions
			float h = (textSize.y + g.Style.ItemInnerSpacing.y) * numFiles +
				g.Style.WindowPadding.y * 3.0f + 16.0f + 20.0f;
			float w = textSize.x + g.Style.ItemInnerSpacing.x * 2.0f;

			float maxHeight = display_h - listWindow->Pos.y - 15.0f;
			if (h > maxHeight) {
				w += g.Style.ScrollbarSize;
				h = maxHeight;
			}

			// Clamping
			h = std::max(20.0f, h);
			w = std::max(w, listWindow->Size.x);

			listWindow = ImGui::GetCurrentWindow();

			ImGui::SetWindowSize(ImVec2(w, h));

			listWindow->Collapsed = false;  // Open window by default
			newResults = false;
		}

		// Inner width for the scrollable list and button
		float childWidth = listWindow->Size.x - g.Style.WindowPadding.x * 2.0f;

		// MAKE A LIST OF FILES
		ImGui::BeginGroup();
		ImGui::BeginChild("Files", ImVec2(childWidth, listWindow->Size.y - 36.0f - g.Style.WindowPadding.y * 3.0f));
		ImGui::BeginGroup();

		for (int i = 0; i < numFiles; i++) {
			int idx = numFiles - i;
			const char * fname = ("file" + std::to_string(i)).c_str();

			ImGui::BeginChild(fname);
			ImGui::PushID(fname);
			
			bool selected = (idx >= begin_H && idx <= end_H);
			if (!selected) { 
				ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor::HSV(0.0f, 0.5f, 0.5f));
			}

			// TODO - add more spaces for less digits
			std::string label = std::to_string(i + 1) + ": " + 
				ITKReader::fileNameAt(int(i * Splats::spacing()));
			bool fileLabel = ImGui::Selectable(label.c_str(), true, 
				ImGuiSelectableFlags_SelectOnClick);

			// Highlight the selected files
			if (fileLabel) {
				if (ImGui::GetIO().KeyShift && selectedFile != -1) {
					begin_H = std::min(idx, selectedFile);
					end_H = std::max(idx, selectedFile);
				}
				else
				{
					selectedFile = idx;
					begin_H = idx;
					end_H = idx;
				}
				updateZRange(begin_H, end_H);
				loadRanges(program, PROG_BOID);
			}

			if (!selected) {
				ImGui::PopStyleColor();
			}
			ImGui::PopID();
			ImGui::EndChild();
		}
		ImGui::EndGroup();
		ImGui::EndChild();

		// MAKE A BUTTON THAT SELECTS ALL FILES
		ImGui::BeginChild("ButtonSelect", ImVec2(childWidth, 20.0f));

		if (ImGui::Button("Select All", ImVec2(childWidth, 20.0f))) {
			begin_H = 1; 
			end_H = Splats::height() / Splats::spacing();
			updateZRange(begin_H, end_H);
			loadRanges(program, PROG_BOID);
		}
		ImGui::EndChild();

		//// Auto resize the height of overall window, might delete l8r
		float maxH = display_h - listWindow->Pos.y - 15.0f;
		if (listWindow->Size.y > maxH) {
			ImGui::SetWindowSize(ImVec2(listWindow->Size.x, maxH));
		}
		ImGui::EndGroup();

		ImGui::End();
	}
	// Dicom Window 2 - tells user they selected nothing
	else {
		ImGui::Begin("No Files Selected");

		// Initialize the right position and size when resetting
		if (newResults && listWindow != nullptr) {
			ImGui::SetWindowPos(ImVec2(listWindow->Pos.x, listWindow->Pos.y));
			ImGui::SetWindowSize(ImVec2(listWindow->Size.x, 0));
			newResults = false;

			listWindow = ImGui::GetCurrentWindow();
			listWindow->Collapsed = true; // Close window by default
		}
		else if (listWindow == nullptr) {
			ImGui::SetWindowPos(ImVec2(15, 35), ImGuiCond_Once);
			ImGui::SetWindowSize(ImVec2(150, 0), ImGuiCond_Once);

			listWindow = ImGui::GetCurrentWindow();
			listWindow->Collapsed = true;
		}
		listWindow = ImGui::GetCurrentWindow();
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	glViewport(0, 0, display_w, display_h);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	return createdNewScene;
}

// Make sure the popup's origin is at the upper right of the screen
// TODO Maybe make it follow the nearest side instead
void GUI::screenResize(int prevWidth, int currWidth) {
	if (editWindow != nullptr) {
		ImVec2 editorPos = editWindow->Pos;
		editWindow->Pos = ImVec2(currWidth - (prevWidth - editorPos.x), 
								 editorPos.y);
	}
}

void GUI::loadRanges(GLuint * program, const unsigned int PROG_BOID) {
	GLint location;
	glUseProgram(program[PROG_BOID]);

	if ((location = glGetUniformLocation(program[PROG_BOID], "u_xRange")) != -1) {
		glUniform2fv(location, 1, &xRange[0]);
	}
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_yRange")) != -1) {
		glUniform2fv(location, 1, &yRange[0]);
	}
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_zRange")) != -1) {
		glUniform2fv(location, 1, &zRange[0]);
	}
	if ((location = glGetUniformLocation(program[PROG_BOID], "u_aRange")) != -1) {
		glUniform2fv(location, 1, &aRange[0]);
	}
}

void GUI::loadDye(GLuint * program, const unsigned int PROG_BOID) {
	GLint location;
	glUseProgram(program[PROG_BOID]);

	if ((location = glGetUniformLocation(program[PROG_BOID], "u_tint")) != -1) {
		glUniform1i(location, GUI::getDye());
	}
}

// Check to see if mouse is in window, update uniform if so
bool GUI::inWindow(int x, int y, 
				   GLuint * program, const unsigned int PROG_BOID) {

	bool inWin =  editWindow != nullptr  && (isDragging ||
		   x >= editWindow->Pos.x &&
		   x <= editWindow->Pos.x + editWindow->Size.x &&
		   y >= editWindow->Pos.y && 
		   y <= editWindow->Pos.y + editWindow->Size.y);

	// TODO - move this to a function in ShaderStruct(to make) and call from 
	// inside IsItemActive() conditional in DragIntRangeCustom
	if (inWin) {
		loadRanges(program, PROG_BOID);
	}

	return inWin;
}