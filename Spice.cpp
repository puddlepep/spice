#include "Spice.hpp"

fs::path DisplayPath(fs::path filepath, bool _asMenu)
{
    if (_asMenu)
    {
        std::string temp_fp = filepath.string() + "/";
        StringReplace(&temp_fp, "\\", "/");

        ImGui::Text(temp_fp.c_str());
        ImGui::Separator();
    }

    for (const auto& entry : fs::directory_iterator(filepath))
    {
        if (entry.is_directory())
        {
            std::string path_name = entry.path().filename().string();
            if (_asMenu)
            {
                if (ImGui::BeginMenu((path_name + "/").c_str()))
                {
                    fs::path p = DisplayPath(entry.path(), true);
                    ImGui::EndMenu();
                    return p;
                }
            }
            else
            {
                if (ImGui::TreeNode((path_name + "/").c_str()))
                {
                    fs::path p = DisplayPath(entry.path(), false);
                    ImGui::TreePop();
                    return p;
                }
            }
        }
        else
        {
            if (_asMenu)
            {
                if (ImGui::MenuItem(entry.path().filename().string().c_str()))
                    return entry.path();
            }
            else
            {
                if (ImGui::SmallButton(entry.path().filename().string().c_str()))
                    return entry.path();
            }
            
        }

    }

    return "";
}
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    xScroll = xoffset;
    yScroll = yoffset;
}

namespace Spice
{

    #pragma region UI

    void Engine::ShowMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (!m_GameMode)
                {
                    if (ImGui::MenuItem("Save", "CTRL+S"))
                    {
                        SaveEngine();
                    }
                }
                else
                    ImGui::MenuItem("Save", "CTRL+S", false, false);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                
                ImGui::MenuItem("Game View", "", &m_ShowGameView);
                ImGui::MenuItem("File Browser", "", &m_ShowFileBrowser);
                ImGui::MenuItem("Object View", "", &m_ShowObjectView);
                ImGui::MenuItem("Project Settings", "", &m_ShowProjectSettings);
                ImGui::MenuItem("ImGui Demo", "", &m_ShowImGuiDemo);
                ImGui::EndMenu();
            }



            ImGui::EndMainMenuBar();
        }
    }

    void Engine::ShowFileBrowser()
    {
        if (m_ShowFileBrowser)
        {
            ImGui::Begin("Files", &m_ShowFileBrowser);
            std::string path = DisplayPath("source", false);

            if (path != "")
            {
                system(("xdg-open " + path).c_str());
            }

            ImGui::End();
        }
    }

    void Engine::ShowGameView()
    {
        if (m_ShowGameView)
        {

            ImGuiWindowFlags window_flags = 0;
            if (m_GameMode)
            {
                window_flags |= ImGuiWindowFlags_NoResize;
                window_flags |= ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoCollapse;
            }

            ImVec2 win_min(91, 140), win_max(FLT_MAX, FLT_MAX);
            ImGui::SetNextWindowSizeConstraints(win_min, win_max);
            ImGui::Begin((m_WindowTitle + "##gameview").c_str(), (m_GameMode ? 0 : &m_ShowGameView), window_flags);
            
            ImVec2 vec = ImGui::GetWindowPos();
            ImVec2 min = ImGui::GetWindowContentRegionMin();
            ImVec2 max = ImGui::GetWindowContentRegionMax();
            
            float sx = max.x - min.x;
            float sy = sx * 0.5625;

            if (sx < 50) sx = 50;
            if (sy < 50) sy = 50;

            m_GameViewPos = glm::vec2(vec.x, vec.y);
            m_GameViewContentMin = glm::vec2(min.x, min.y);
            m_GameViewContentMax = glm::vec2(sx, sy);


            ImGui::Image((void*)m_GameTXO, { sx, sy }, { 0, 1 }, { 1, 0 });
            
            ImGui::Separator();

            if (ImGui::Checkbox("Play", &m_GameMode))
            {
                if (m_GameMode)
                    SaveEngine();

                for (Object* obj : Objects)
                {
                    ScriptComponent* script = obj->GetComponent<ScriptComponent>();
                    if (script != nullptr)
                    {
                        if (m_GameMode)
                        {
                            script->ScriptData->Reload();
                        }
                        else
                        {
                            script->ScriptData->RunClose();
                        }
                    }
                }

                if (!m_GameMode)
                {
                    ReloadProjectSettings();
                    ReloadObjects();
                }
            }

            ImGui::End();
        }
    }

    void Engine::ShowObjectView()
    {
        if (m_ShowObjectView)
        {
            
            ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
            ImGui::Begin("Object View", &m_ShowObjectView, ImGuiWindowFlags_MenuBar);

            // Create Object
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("Create"))
                {

                    if (ImGui::MenuItem("Empty Game Object"))
                    {
                        Object* obj = new Object("Empty Object");
                        Objects.push_back(obj);
                    }

                    if (ImGui::MenuItem("Transform Object"))
                    {
                        Object* obj = new Object("Object");
                        obj->AddComponent<Transform>();
                        Objects.push_back(obj);
                    }

                    if (ImGui::MenuItem("Shape Renderer"))
                    {
                        Object* obj = new Object("Object");
                        obj->AddComponent<Transform>();
                        obj->AddComponent<ShapeRenderer>();
                        Objects.push_back(obj);
                    }

                    if (ImGui::MenuItem("Script Object"))
                    {
                        Object* obj = new Object("Script Object");
                        obj->AddComponent<Transform>();
                        obj->AddComponent<ShapeRenderer>();
                        obj->AddComponent<ScriptComponent>();
                        Objects.push_back(obj);
                    }

                    if (ImGui::MenuItem("Text Renderer"))
                    {
                        Object* obj = new Object("Text");
                        obj->AddComponent<Transform>();
                        obj->AddComponent<TextRenderer>();
                        Objects.push_back(obj);
                    }

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            static int selected = 0;

            if (Objects.size() <= selected)
                selected = 0;

            ImGui::BeginChild("left pane", ImVec2(150, 0), true);
            for (int i = 0; i < Objects.size(); i++)
            {
                Object* obj = Objects[i];

                std::string title = obj->Name + "##" + std::to_string(i);

                if (!obj->Active)
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4, 0.4, 0.4, 1));

                if (ImGui::Selectable(title.c_str(), selected == i))
                {
                    selected = i;
                }

                if (!obj->Active)
                    ImGui::PopStyleColor();
            }
            ImGui::EndChild();
            ImGui::SameLine();

            ImGui::BeginGroup();
            ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

            if (Objects.size() > 0)
            {
                ImGui::InputText("", &Objects[selected]->Name);
                ImGui::SameLine();
                ImGui::Checkbox("active", &Objects[selected]->Active);
                ImGui::SameLine();
                ShowHelpMarker("To modify via script, use 'object.active = true/false'\n(you can access a script's parent object with 'script.parent')");

                ImGui::Separator();
                if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
                {
                    static std::string help_desc = "";

                    if (ImGui::BeginTabItem("Components"))
                    {
                        help_desc = "To access components via script, use 'object:GetComponent(name)'";

                        ImVec2 avail_region = ImGui::GetContentRegionAvail();
                        avail_region.x -= 40;
                        
                        std::vector<float> y_positions;

                        ImGui::BeginChild("component_frame", avail_region, false);
                        for (int i = 0; i < Objects[selected]->Components.size(); i++)
                        {
                            Component* component = Objects[selected]->Components[i];
                            y_positions.push_back(ImGui::GetCursorPosY());

                            if (ImGui::CollapsingHeader(component->GetType().c_str()))
                            {
                                for (int i = 0; i < component->Parameters.size(); i++)
                                {
                                    if (ShowParameter(component->Parameters[i], false, i))
                                        component->Callback();
                                }
                            }
                        }
                        ImGui::EndChild();
                        ImGui::EndTabItem();

                        ImVec2 new_avail_region = ImGui::GetContentRegionAvail();

                        ImGui::SameLine();
                        ImGui::BeginChild("component_delete_frame", new_avail_region, false);
                        
                        for (int i = 0; i < Objects[selected]->Components.size(); i++)
                        {
                            ImGui::SetCursorPosY(y_positions[i]);
                            if (ImGui::Button(("X##" + std::to_string(i)).c_str()))
                            {
                                Objects[selected]->RemoveComponent(i);
                            }
                        }

                        ImGui::EndChild();
                    }

                    if (ImGui::BeginTabItem("Storage"))
                    {
                        help_desc = "To access storage items via script, use\n 'object.storage['storage_item_name']'";
                        ImGui::SameLine();
                        ShowHelpMarker(help_desc);

                        for (int i = 0; i < Objects[selected]->Storage.size(); i++)
                        {
                            if (ShowParameter(Objects[selected]->Storage[i], true, i))
                            {
                                Objects[selected]->RemoveStorageItem(i);
                            }
                        }

                        if (ImGui::Button("+##storage"))
                            ImGui::OpenPopup("##storage_add_item");
                        if (ImGui::BeginPopup("##storage_add_item"))
                        {
                            ImGui::Text("select a type");
                            ImGui::Separator();
                            
                            for (int i = 0; i < DataTypeNames.size(); i++)
                            {
                                std::string name = DataTypeNames[(DataType)i] + "##storage_add_item";
                                if (ImGui::MenuItem(name.c_str()))
                                {
                                    Objects[selected]->PushStorageItem(Parameter(DataTypeNames[(DataType)i], (DataType)i, nullptr, { true }));
                                }
                            }

                            ImGui::EndPopup();
                        }

                        ImGui::EndTabItem();
                    }
                    else
                    {
                        ImGui::SameLine();
                        ShowHelpMarker(help_desc);
                    }

                    ImGui::EndTabBar();
                }
            }

            ImGui::EndChild();

            if (Objects.size() > 0)
            {
                if (ImGui::Button("delete")) 
                {
                    delete Objects[selected];
                    Objects.erase(Objects.begin() + selected);
                    selected = 0;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("add component"))
                ImGui::OpenPopup("add_component_popup");
            if (ImGui::BeginPopup("add_component_popup"))
            {
                ImGui::Text("select component");
                ImGui::Separator();
                if (ImGui::MenuItem("transform##add_component"))
                {
                    Objects[selected]->AddComponent<Transform>();
                }
                if (ImGui::MenuItem("shape renderer##add_component"))
                {
                    Objects[selected]->AddComponent<ShapeRenderer>();
                }
                if (ImGui::MenuItem("text renderer##add_component"))
                {
                    Objects[selected]->AddComponent<TextRenderer>();
                }
                if (ImGui::MenuItem("script component##add_component"))
                {
                    Objects[selected]->AddComponent<ScriptComponent>();
                }

                ImGui::EndPopup();
            }

            ImGui::EndGroup();

            ImGui::End();
        }
    }

    void Engine::ShowProjectSettings()
    {
        if (m_ShowProjectSettings)
        {
            ImGui::Begin("Project Settings", &m_ShowProjectSettings);

            ShowHelpMarker("Toggles shipping mode. Upon restart, the game will run without the editor tools. To revert, set 'shipping_mode' to false in 'save/data.lua'");
            ImGui::SameLine();
            ImGui::Text("shipping mode");
            ImGui::SameLine();
            ImGui::Checkbox("", &m_ShippingModeToggle);
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(450.0f);
                ImGui::TextUnformatted("**WARNING**\nREAD HELP TOOLTIP BEFORE TOGGLING");
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }

            ImGui::Separator();
            ImGui::Spacing();

            ShowHelpMarker("To modify via script, use 'SetBackgroundColor(color)'");
            ImGui::SameLine();
            ImGui::Text("background color");
            ImGui::SameLine();
            ImGui::ColorEdit4("##backgroundcolor", &m_BackgroundColor.r, ImGuiColorEditFlags_NoInputs);

            ShowHelpMarker("Allows the game window to be resized");
            ImGui::SameLine();
            ImGui::Text("can resize game window");
            ImGui::SameLine();
            ImGui::Checkbox("##resizeablewindow", &m_ResizeableWindow);

            ShowHelpMarker("To modify via script, use 'SetWindowTitle(string)'");
            ImGui::SameLine();
            ImGui::Text("window title");
            ImGui::SameLine();
            static std::string temp_title = m_WindowTitle;
            if (ImGui::InputText("##windowtitle", &temp_title, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_WindowTitle = temp_title;
            }
            
            ImGui::End();
        }
    }

    bool Engine::ShowParameter(Parameter& parameter, bool editable, int index)
    {
        std::string name = parameter.Name;
        DataType type = parameter.Type;
        DataSettings settings = parameter.Settings;

        bool return_value = false;
        bool deleted_item = false;
        
        if (editable)
        {
            static std::string buffer = "";
            static int selected_index = -1;

            if (ImGui::Button(("X##" + std::to_string(index)).c_str()))
                deleted_item = true;
            ImGui::SameLine();

            if (selected_index == index)
            {
                ImGui::PushItemWidth(150);
                if (ImGui::InputText(("##" + std::to_string(index)).c_str(), &buffer, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (buffer == "") buffer = parameter.Name;
                    else parameter.Name = buffer;

                    buffer = "";
                    selected_index = -1;
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
            }
            else
            {
                if (ImGui::Button((name + "##" + std::to_string(index)).c_str()))
                {
                    selected_index = index;
                    buffer = name;
                }
                ImGui::SameLine();
            }

        }
        else
        {
            if (parameter.Settings.KillData)
            {
                ImGui::Text(name.c_str());
                ImGui::SameLine();
            }
        }

        if (type == DataType::Bool)
        {
            bool* ptr = static_cast<bool*>(parameter.Data);
            if (ImGui::Checkbox(("##" + name + std::to_string(index)).c_str(), ptr))
                return_value = true;
        }

        else if (type == DataType::Float)
        {
            float* ptr = static_cast<float*>(parameter.Data);
            if (ImGui::DragFloat(("##" + name).c_str(), ptr, settings.DragSpeed, settings.MinimumLimit, settings.MaximumLimit))
                return_value = true;
        }

        else if (type == DataType::Vec2)
        {
            glm::vec2* vec = static_cast<glm::vec2*>(parameter.Data);
            if (ImGui::DragFloat2(("##" + name).c_str(), (float*)vec, settings.DragSpeed, settings.MinimumLimit, settings.MaximumLimit))
                return_value = true;
        }

        else if (type == DataType::String)
        {
            std::string* str = static_cast<std::string*>(parameter.Data);
            if (ImGui::InputText(("##" + name).c_str(), str, ImGuiInputTextFlags_EnterReturnsTrue))
                return_value = true;
        }

        else if (type == DataType::Filepath)
        {
            std::string* str = static_cast<std::string*>(parameter.Data);
            if (ImGui::InputText(("##" + name).c_str(), str, ImGuiInputTextFlags_EnterReturnsTrue))
                return_value = true;

            ImGui::SameLine();
            if (ImGui::Button("..."))
                ImGui::OpenPopup(("##popup_" + name).c_str());
            if (ImGui::BeginPopup(("##popup_" + name).c_str()))
            {
                
                std::string name = DisplayPath("source", true).string();
                if (name != "")
                {
                    *str = name;
                    return_value = true;
                }

                ImGui::EndPopup();
            }
        }

        else if (type == DataType::Color)
        {
            Color* col = static_cast<Color*>(parameter.Data);
            if (ImGui::ColorEdit4(("##" + name).c_str(), &col->r, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar))
                return_value = true;
        }

        else if (type == DataType::Separator)
        {
            ImGui::Separator();
        }

        else if (type == DataType::Vec2ArrayRenderer)
        {
            std::vector<glm::vec2> points = *static_cast<std::vector<glm::vec2>*>(parameter.Data);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();      // ImDrawList API uses screen coordinates!
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();   // Resize canvas to what's available

            canvas_sz.x /= 2;
            canvas_sz.y /= 2;

            if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
            if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;

            canvas_p0.x += canvas_sz.x / 4;

            ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

            draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));

            if (points.size() > 1)
            {
                ImGui::PushClipRect(canvas_p0, canvas_p1, true);

                ImVec2 offset = { canvas_p0.x + (canvas_sz.x / 2), canvas_p0.y + (canvas_sz.y / 2) };

                for (int i = 1; i < points.size(); i++)
                {
                    ImVec2 point = { points[i].x + offset.x, -points[i].y + offset.y };
                    ImVec2 old_point = { points[i - 1].x + offset.x, -points[i - 1].y + offset.y };

                    draw_list->AddLine(point, old_point, IM_COL32(255, 255, 255, 255));

                    if (i == points.size() - 1)
                    {
                        draw_list->AddLine(point, { points[0].x + offset.x, -points[0].y + offset.y }, IM_COL32(255, 255, 255, 255));
                        draw_list->AddText({ points[0].x + offset.x, -points[0].y + offset.y }, IM_COL32(0, 0, 0, 255), "1");

                    }

                    draw_list->AddText(point, IM_COL32(0, 0, 0, 255), std::to_string(i + 1).c_str());
                }

                ImGui::PopClipRect();
            }

            ImGui::InvisibleButton("render", canvas_sz);
        }

        else if (type == DataType::Vec2Array)
        {
            std::vector<glm::vec2>& points = *static_cast<std::vector<glm::vec2>*>(parameter.Data);

            if (ImGui::Button(("+##" + std::to_string(index)).c_str()))
            {
                points.push_back({ 0, 0 });
                return_value = true;
            }

            for (int i = 0; i < points.size(); i++)
            {
                ImGui::Text((std::to_string(i + 1) + ":").c_str());
                ImGui::SameLine();
                if (ImGui::DragFloat2(("##" + std::to_string(index) + "|" + std::to_string(i)).c_str(), (float*)&points[i], settings.DragSpeed, settings.MinimumLimit, settings.MaximumLimit))
                {
                    return_value = true;
                }

                if (points.size() > 1 || points[0] != glm::vec2(0, 0))
                {
                    ImGui::SameLine();
                    if (ImGui::Button(("X##" + std::to_string(index) + "|" + std::to_string(i)).c_str()))
                    {
                        if (points.size() > 1)
                        {
                            points.erase(points.begin() + i);
                            return_value = true;
                        }
                        else
                        {
                            points[0] = { 0, 0 };
                            return_value = true;
                        }
                    }
                }
            }
        }

        else if (type == DataType::None)
            ImGui::NewLine();

        if (editable)
        {
            return deleted_item;
        }
        else
        {
            return return_value;
        }

    }

    void Engine::ShowHelpMarker(std::string _desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(450.0f);
            ImGui::TextUnformatted(_desc.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    #pragma endregion

    #pragma region Input

    InputState Engine::GetKey(unsigned int key)
    {
        if (m_InputMap.find(key) != m_InputMap.end())
        {
            return m_InputMap[key];
        }

        return InputState();
    }
    glm::vec2 Engine::GetCursorPos()
    {
        double xpos, ypos;
        glfwGetCursorPos(m_Window, &xpos, &ypos);

        if (!m_ShippingMode)
        {
            glm::vec2 real_pos(0, 0);

            real_pos.x = xpos - m_GameViewPos.x - m_GameViewContentMin.x;
            real_pos.y = m_GameViewContentMax.y - (ypos - m_GameViewPos.y - m_GameViewContentMin.y);

            glm::vec2 perc = real_pos / m_GameViewContentMax;
            glm::vec2 final_pos = glm::vec2(m_WindowWidth, m_WindowHeight) * perc;

            return final_pos;
        }
        else
        {
            return glm::vec2(xpos, m_WindowHeight - ypos);
        }

    }

    void Engine::RegisterMouseButton(char b)
    {
        if (m_InputMap.find(b) == m_InputMap.end())
            m_InputMap[b] = InputState();

        InputState& button = m_InputMap[b];
        unsigned int state = glfwGetMouseButton(m_Window, b);

        if (state == GLFW_PRESS)
        {
            if (button.Held)
            {
                button.Held = true;
                button.Pressed = false;
                button.Released = false;
            }
            else
            {
                button.Held = true;
                button.Pressed = true;
                button.Released = false;
            }
        }
        else
        {
            if (button.Held)
            {
                button.Held = false;
                button.Pressed = false;
                button.Released = true;
            }
            else
            {
                button.Held = false;
                button.Pressed = false;
                button.Released = false;
            }
        }
    }
    void Engine::RegisterKey(unsigned int k)
    {
        if (m_InputMap.find(k) == m_InputMap.end())
            m_InputMap[k] = InputState();

        InputState& key = m_InputMap[k];
        unsigned int state = glfwGetKey(m_Window, k);

        if (state == GLFW_PRESS)
        {
            if (key.Held)
            {
                key.Held = true;
                key.Pressed = false;
                key.Released = false;
            }
            else
            {
                key.Held = true;
                key.Pressed = true;
                key.Released = false;
            }
        }
        else
        {
            if (key.Held)
            {
                key.Held = false;
                key.Pressed = false;
                key.Released = true;
            }
            else
            {
                key.Held = false;
                key.Pressed = false;
                key.Released = false;
            }
        }
    }
    void Engine::RegisterInput()
    {
        RegisterMouseButton(MOUSE_LEFT);
        RegisterMouseButton(MOUSE_RIGHT);
        RegisterMouseButton(MOUSE_MIDDLE);
        RegisterKey(SPACE);
        RegisterKey(ARROW_RIGHT);
        RegisterKey(ARROW_LEFT);
        RegisterKey(ARROW_UP);
        RegisterKey(ARROW_DOWN);
        RegisterKey(LEFT_CONTROL);
        RegisterKey(RIGHT_CONTROL);

        for (unsigned int i = 65; i <= 90; i++)
        {
            RegisterKey(i);
        }
    }

    void Engine::lua_RegisterInput(lua_State* L)
    {
        lua_RegisterKeys(L);

        lua_register(L, "GetKey", Engine::lua_GetKey);
        lua_register(L, "GetMouseButton", Engine::lua_GetKey);
        lua_register(L, "GetCursorPos", Engine::lua_GetCursorPos);
    }

    void Engine::lua_RegisterKeys(lua_State* L)
    {
        lua_newtable(L);
        lua_setglobal(L, "Key");
        lua_getglobal(L, "Key");

        lua_pushstring(L, "Left");
        lua_pushnumber(L, GLFW_KEY_LEFT);
        lua_settable(L, -3);

        lua_pushstring(L, "Right");
        lua_pushnumber(L, GLFW_KEY_RIGHT);
        lua_settable(L, -3);

        lua_pushstring(L, "Up");
        lua_pushnumber(L, GLFW_KEY_UP);
        lua_settable(L, -3);

        lua_pushstring(L, "Down");
        lua_pushnumber(L, GLFW_KEY_DOWN);
        lua_settable(L, -3);

        lua_pushstring(L, "Space");
        lua_pushnumber(L, GLFW_KEY_SPACE);
        lua_settable(L, -3);

        lua_pushstring(L, "LeftControl");
        lua_pushnumber(L, GLFW_KEY_LEFT_CONTROL);
        lua_settable(L, -3);

        lua_pushstring(L, "RightControl");
        lua_pushnumber(L, GLFW_KEY_RIGHT_CONTROL);
        lua_settable(L, -3);
    }

    int Engine::lua_GetKey(lua_State* L)
    {
        if (lua_gettop(L) == 1)
        {
            int type = lua_type(L, 1);
            if (type == LUA_TNUMBER || type == LUA_TSTRING)
            {
                unsigned int key = (type == LUA_TNUMBER) ? lua_tonumber(L, 1) : lua_tostring(L, 1)[0];

                lua_getglobal(L, "__engineptr");
                Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

                InputState is = engine->GetKey(key);
                
                lua_newtable(L);
                
                lua_pushstring(L, "Held");
                lua_pushboolean(L, is.Held);
                lua_settable(L, -3);

                lua_pushstring(L, "Pressed");
                lua_pushboolean(L, is.Pressed);
                lua_settable(L, -3);

                lua_pushstring(L, "Released");
                lua_pushboolean(L, is.Released);
                lua_settable(L, -3);
                
                return 1;
            }
            else
            {
                lua_newtable(L);

                lua_pushstring(L, "Held");
                lua_pushboolean(L, false);
                lua_settable(L, -3);

                lua_pushstring(L, "Pressed");
                lua_pushboolean(L, false);
                lua_settable(L, -3);

                lua_pushstring(L, "Released");
                lua_pushboolean(L, false);
                lua_settable(L, -3);

                return 1;
            }
        }
        else
        {
            LuaThrowSyntaxError(L, "GetKey(key) requires 1 parameter!");
            return 0;
        }
    }

    int Engine::lua_GetCursorPos(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));
        
        glm::vec2 cursor_pos = engine->GetCursorPos();
        
        lua_pop(L, lua_gettop(L));
        lua_pushnumber(L, cursor_pos.x);
        lua_pushnumber(L, cursor_pos.y);

        LuaVector2::lua_New(L);
        return 1;
    }

    #pragma endregion

    #pragma region Helpers

    float Engine::Tween(float _v1, float _v2, float _progress, EaseType _type)
    {
        float t = _progress;

        switch (_type)
        {

        case EaseType::LINEAR:
            break;

        case EaseType::EASE_IN_SINE:
            t = 1 - cos((_progress * PI) / 2);
            break;

        case EaseType::EASE_OUT_SINE:
            t = sin((_progress * PI) / 2);
            break;

        case EaseType::EASE_IN_OUT_SINE:
            t = -(cos(PI * _progress) - 1) / 2;
            break;

        case EaseType::EASE_IN_QUAD:
            t = _progress * _progress;
            break;

        case EaseType::EASE_OUT_QUAD:
            t = 1 - (1 - _progress) * (1 - _progress);
            break;

        case EaseType::EASE_IN_OUT_QUAD:
            t = _progress < 0.5 ? 2 * _progress * _progress : 1 - pow(-2 * _progress + 2, 2) / 2;
            break;

        case EaseType::EASE_IN_CUBIC:
            t = _progress * _progress * _progress;
            break;

        case EaseType::EASE_OUT_CUBIC:
            t = 1 - pow(1 - _progress, 3);
            break;

        case EaseType::EASE_IN_OUT_CUBIC:
            t = _progress < 0.5 ? 4 * _progress * _progress * _progress : 1 - pow(-2 * _progress + 2, 3) / 2;
            break;

        case EaseType::EASE_IN_QUART:
            t = _progress * _progress * _progress * _progress;
            break;

        case EaseType::EASE_OUT_QUART:
            t = 1 - pow(1 - _progress, 4);
            break;

        case EaseType::EASE_IN_OUT_QUART:
            t = _progress < 0.5 ? 8 * _progress * _progress * _progress * _progress : 1 - pow(-2 * _progress + 2, 4) / 2;
            break;

        case EaseType::EASE_IN_EXPO:
            t = _progress == 0 ? 0 : pow(2, 10 * _progress - 10);
            break;

        case EaseType::EASE_OUT_EXPO:
            t = _progress == 1 ? 1 : 1 - pow(2, -10 * _progress);
            break;

        case EaseType::EASE_IN_OUT_EXPO:
            t = _progress == 0
                ? 0
                : _progress == 1
                ? 1
                : _progress < 0.5 ? pow(2, 20 * _progress - 10) / 2
                : (2 - pow(2, -20 * _progress + 10)) / 2;
            break;

        case EaseType::EASE_IN_BACK:
            t = (1.70158f + 1.0f) * _progress * _progress * _progress - 1.70158f * _progress * _progress;
            break;

        case EaseType::EASE_OUT_BACK:
            t = 1 + (1.70158f + 1.0f) * pow(_progress - 1, 3) + 1.70158f * pow(_progress - 1, 2);
            break;

        case EaseType::EASE_IN_OUT_BACK:
            t = _progress < 0.5
                ? (pow(2 * _progress, 2) * (((1.70158f * 1.525f) + 1) * 2 * _progress - (1.70158f * 1.525f))) / 2
                : (pow(2 * _progress - 2, 2) * (((1.70158f * 1.525f) + 1) * (_progress * 2 - 2) + (1.70158f * 1.525f)) + 2) / 2;
            break;

        case EaseType::EASE_IN_ELASTIC:
            t = _progress == 0
                ? 0
                : _progress == 1
                ? 1
                : -pow(2, 10 * _progress - 10) * sin((_progress * 10 - 10.75) * ((2.0f * PI) / 3.0f));
            break;

        case EaseType::EASE_OUT_ELASTIC:
            t = _progress == 0
                ? 0
                : _progress == 1
                ? 1
                : pow(2, -10 * _progress) * sin((_progress * 10 - 0.75) * ((2.0f * PI) / 3.0f)) + 1;
            break;

        case EaseType::EASE_IN_OUT_ELASTIC:
            t = _progress == 0
                ? 0
                : _progress == 1
                ? 1
                : _progress < 0.5
                ? -(pow(2, 20 * _progress - 10) * sin((20 * _progress - 11.125) * ((2.0 * PI) / 4.5f))) / 2
                : (pow(2, -20 * _progress + 10) * sin((20 * _progress - 11.125) * ((2.0 * PI) / 4.5f))) / 2 + 1;
            break;

        }

        return t * (_v2 - _v1) + _v1;

    }

    void Engine::lua_RegisterHelpers(lua_State* L)
    {
        lua_register(L, "GetWindowWidth", lua_GetWindowWidth);
        lua_register(L, "GetWindowHeight", lua_GetWindowHeight);
        lua_register(L, "GetWindowCenter", lua_GetWindowCenter);
        lua_register(L, "GetDelta", lua_GetDelta);
        lua_register(L, "GetTimeElapsed", lua_GetTimeElapsed);
        lua_register(L, "FindObject", lua_FindObject);
        lua_register(L, "FindObjects", lua_FindObjects);
        lua_register(L, "SetBackgroundColor", lua_SetBackgroundColor);

        lua_RegisterEaseTypes(L);
        lua_register(L, "Tween", lua_Tween);

        lua_RegisterRandom(L);
    }

    void Engine::lua_RegisterEaseTypes(lua_State* L)
    {
        lua_newtable(L);
        lua_setglobal(L, "EaseType");
        lua_getglobal(L, "EaseType");

        lua_pushstring(L, "Linear");
        lua_pushnumber(L, 0);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInSine");
        lua_pushnumber(L, 1);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutSine");
        lua_pushnumber(L, 2);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutSine");
        lua_pushnumber(L, 3);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInQuad");
        lua_pushnumber(L, 4);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutQuad");
        lua_pushnumber(L, 5);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutQuad");
        lua_pushnumber(L, 6);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInCubic");
        lua_pushnumber(L, 7);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutCubic");
        lua_pushnumber(L, 8);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutCubic");
        lua_pushnumber(L, 9);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInQuart");
        lua_pushnumber(L, 10);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutQuart");
        lua_pushnumber(L, 11);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutQuart");
        lua_pushnumber(L, 12);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInExpo");
        lua_pushnumber(L, 13);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutExpo");
        lua_pushnumber(L, 14);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutExpo");
        lua_pushnumber(L, 15);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInBack");
        lua_pushnumber(L, 16);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutBack");
        lua_pushnumber(L, 17);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutBack");
        lua_pushnumber(L, 18);
        lua_settable(L, -3);


        lua_pushstring(L, "EaseInElastic");
        lua_pushnumber(L, 19);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseOutElastic");
        lua_pushnumber(L, 20);
        lua_settable(L, -3);

        lua_pushstring(L, "EaseInOutElastic");
        lua_pushnumber(L, 21);
        lua_settable(L, -3);
    }

    int Engine::lua_GetWindowWidth(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

        lua_pushnumber(L, engine->GetWindowWidth());
        return 1;
    }

    int Engine::lua_GetWindowHeight(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

        lua_pushnumber(L, engine->GetWindowHeight());
        return 1;
    }

    int Engine::lua_GetWindowCenter(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

        lua_pop(L, lua_gettop(L));
        glm::vec2 center = engine->GetWindowCenter();
        lua_pushnumber(L, center.x);
        lua_pushnumber(L, center.y);
        LuaVector2::lua_New(L);
        return 1;
    }

    int Engine::lua_GetDelta(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

        lua_pushnumber(L, engine->GetDelta());
        return 1;
    }

    int Engine::lua_GetTimeElapsed(lua_State* L)
    {
        lua_getglobal(L, "__engineptr");
        Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

        lua_pushnumber(L, engine->GetTimeElapsed());
        return 1;
    }

    int Engine::lua_Tween(lua_State* L)
    {
        int argAmt = lua_gettop(L);

        if (argAmt > 2 && argAmt < 5)
        {
            float n1 = lua_tonumber(L, 1);
            float n2 = lua_tonumber(L, 2);
            float t = lua_tonumber(L, 3);
            EaseType type = EaseType::LINEAR;

            if (argAmt > 3)
                type = (EaseType)lua_tonumber(L, 4);

            lua_pushnumber(L, Tween(n1, n2, t, type));
            return 1;
        }

        return 0;
    }

    int Engine::lua_FindObject(lua_State* L)
    {
        if (lua_gettop(L) == 1)
        {
            if (lua_isstring(L, 1))
            {
                lua_getglobal(L, "__engineptr");
                Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

                std::string str = lua_tostring(L, 1);
                for (int i = 0; i < engine->Objects.size(); i++)
                {
                    if (engine->Objects[i]->Name == str)
                    {
                        LuaObject::lua_New(L, engine->Objects[i]);
                        return 1;
                    }
                }

                return 0;
            }
            else
            {
                LuaThrowSyntaxError(L, "FindObject(name)'s only argument must be a string!");
                return 0;
            }
        }
        else
        {
            LuaThrowSyntaxError(L, "FindObject(name) requires 1 argument!");
            return 0;
        }
    }

    int Engine::lua_FindObjects(lua_State* L)
    {
        if (lua_gettop(L) == 1)
        {
            if (lua_isstring(L, 1))
            {
                lua_getglobal(L, "__engineptr");
                Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));

                std::string str = lua_tostring(L, 1);
                lua_newtable(L);
                for (int i = 0; i < engine->Objects.size(); i++)
                {
                    if (engine->Objects[i]->Name == str)
                    {
                        lua_len(L, -1);
                        unsigned int len = lua_tonumber(L, -1);
                        lua_pop(L, 1);

                        LuaObject::lua_New(L, engine->Objects[i]);
                        lua_rawseti(L, -2, len + 1);
                    }
                }

                return 1;
            }
            else
            {
                LuaThrowSyntaxError(L, "FindObject(name)'s only argument must be a string!");
                return 0;
            }
        }
        else
        {
            LuaThrowSyntaxError(L, "FindObject(name) requires 1 argument!");
            return 0;
        }
    }

    int Engine::lua_SetBackgroundColor(lua_State* L)
    {
        if (lua_gettop(L) == 1)
        {
            if (lua_type(L, 1) == LUA_TUSERDATA)
            {
                lua_getglobal(L, "__engineptr");
                Engine* engine = static_cast<Engine*>(lua_touserdata(L, -1));
                lua_pop(L, 1);

                Color* color = static_cast<LuaColor*>(lua_touserdata(L, 1))->ColorPtr;
                engine->m_BackgroundColor = *color;
                return 0;
            }
            else
            {
                LuaThrowSyntaxError(L, "SetBackgroundColor(color)'s parameter must be a color!");
                return 0;
            }
        }
        else
        {
            LuaThrowSyntaxError(L, "SetBackgroundColor(color) requires 1 parameter!");
            return 0;
        }
    }

    int Engine::lua_RandomNew(lua_State* L)
    {
        unsigned int seed;
        if (lua_isnumber(L, 1))
        {
            seed = lua_tonumber(L, 1);
        }
        else
        {
            seed = (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1)) % 10000000;
        }

        Random* rng = static_cast<Random*>(lua_newuserdata(L, sizeof(Random)));
        new (rng) Random(seed);

        luaL_getmetatable(L, "RandomMTT");
        lua_setmetatable(L, -2);

        return 1;
    }

    int Engine::lua_RandomIndex(lua_State* L)
    {
        Random* rng = static_cast<Random*>(lua_touserdata(L, 1));
        std::string index = lua_tostring(L, 2);

        if (index == "seed")
        {
            lua_pushnumber(L, rng->LehmerSeed);
            return 1;
        }
        else
        {
            lua_getglobal(L, "Random");
            lua_pushstring(L, index.c_str());
            lua_rawget(L, -2);

            return 1;
        }
    }

    int Engine::lua_RandomInteger(lua_State* L)
    {
        if (lua_isuserdata(L, 1))
        {
            Random* rng = static_cast<Random*>(lua_touserdata(L, 1));

            if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
            {
                int min = lua_tonumber(L, 2);
                int max = lua_tonumber(L, 3);
                int result = rng->Lehmer32(min, max);

                lua_pushnumber(L, result);
            }
            else
            {
                lua_pushnumber(L, rng->Lehmer32());
            }

            return 1;
        }
        else
        {
            LuaThrowSyntaxError(L, "Random.NextInteger(rng, min=0, max=32BIT_MAX)'s first parameter must be a random generator!");
            return 0;
        }

        return 0;
    }

    int Engine::lua_RandomNumber(lua_State* L)
    {
        if (lua_isuserdata(L, 1))
        {
            Random* rng = static_cast<Random*>(lua_touserdata(L, 1));
            double decimal_result = static_cast<double>((rng->Lehmer32() % 1000000000)) / 1000000000.0;

            if (lua_gettop(L) == 3)
            {
                if (lua_isnumber(L, 2) && lua_isnumber(L, 3))
                {
                    int min = lua_tonumber(L, 2);
                    int max = lua_tonumber(L, 3);

                    int int_result = rng->Lehmer32(min, max - 1);
                    double final_result = static_cast<double>(int_result) + decimal_result;

                    lua_pushnumber(L, final_result);
                    return 1;
                }
                else
                {
                    LuaThrowSyntaxError(L, "Random.NextNumber(rng, min=0, max=1)'s min and max parameters must be numbers!");
                    return 0;
                }
            }

            lua_pushnumber(L, decimal_result);
            return 1;
        }
        else
        {
            LuaThrowSyntaxError(L, "Random.NextNumber(rng, min=0, max=1)'s first parameter must be a random number generator!");
            return 0;
        }

        return 0;
    }

    void Engine::lua_RegisterRandom(lua_State* L)
    {
        static const luaL_Reg RandomLib[] =
        {
            {"new", lua_RandomNew},
            {"NextInteger", lua_RandomInteger},
            {"NextNumber", lua_RandomNumber},
            {NULL, NULL}
        };

        luaL_newlib(L, RandomLib);
        lua_setglobal(L, "Random");

        luaL_newmetatable(L, "RandomMTT");

        lua_pushstring(L, "__index");
        lua_pushcfunction(L, lua_RandomIndex);
        lua_settable(L, -3);
    }

    void Engine::SaveEngine()
    {
        std::cout << "Saving..." << std::endl;

        lua_State* L = m_SaveL;

        lua_newtable(L); // objects
        int tbl_objects = lua_gettop(L);

        for (Object* obj : Objects)
        {
            lua_newtable(L); // obj
            int tbl_obj = lua_gettop(L);

            lua_pushstring(L, "name");
            lua_pushstring(L, obj->Name.c_str());
            lua_rawset(L, tbl_obj);

            lua_pushstring(L, "active");
            lua_pushboolean(L, obj->Active);
            lua_rawset(L, tbl_obj);

            lua_pushstring(L, "storage");
            lua_newtable(L); // storage
            int tbl_storage = lua_gettop(L);

            for (unsigned int i = 1; i <= obj->Storage.size(); i++)
            {
                lua_newtable(L);
                obj->Storage[i - 1].Serialize(L);
                lua_rawseti(L, tbl_storage, i);
            }
            lua_rawset(L, tbl_obj);


            lua_pushstring(L, "components");
            lua_newtable(L); // components
            int tbl_components = lua_gettop(L);

            for (unsigned int i = 1; i <= obj->Components.size(); i++)
            {
                lua_newtable(L); // comp

                obj->Components[i - 1]->Serialize(L);

                lua_rawseti(L, tbl_components, i);
            }
            lua_rawset(L, tbl_obj);

            lua_len(L, tbl_objects);
            unsigned int len = lua_tonumber(L, -1);
            lua_pop(L, 1);

            lua_rawseti(L, tbl_objects, len + 1);
        }

        lua_len(L, tbl_objects);
        int objects_len = lua_tonumber(L, -1);
        lua_pop(L, 1);
        
        std::vector<std::string> object_strings;

        for (int i = 0; i < objects_len; i++)
        {
            std::string obj_str;
            obj_str += "{\n";
            {
                lua_rawgeti(L, tbl_objects, i + 1);
                int tbl_obj = lua_gettop(L);

                lua_pushstring(L, "name");
                lua_rawget(L, tbl_obj);
                std::string name = lua_tostring(L, -1);
                lua_pop(L, 1);

                lua_pushstring(L, "active");
                lua_rawget(L, tbl_obj);
                bool active = lua_toboolean(L, -1);
                lua_pop(L, 1);

                obj_str += "name = \"" + name + "\",\n\n";
                obj_str += "active = " + std::string((active ? "true" : "false")) + ",\n";

                lua_pushstring(L, "storage");
                lua_rawget(L, tbl_obj);
                int tbl_storage = lua_gettop(L);

                obj_str += "storage = {\n";
                {
                    lua_len(L, -1);
                    int storage_len = lua_tonumber(L, -1);
                    lua_pop(L, 1);

                    for (int j = 0; j < storage_len; j++)
                    {
                        obj_str += "{\n";
                        {
                            lua_rawgeti(L, tbl_storage, j + 1);
                            DataType data_type = DataType::None;

                            lua_pushstring(L, "name");
                            lua_rawget(L, -2);
                            obj_str += "name = \"" + std::string(lua_tostring(L, -1)) + "\",\n";
                            lua_pop(L, 1);

                            lua_pushstring(L, "datatype");
                            lua_rawget(L, -2);
                            data_type = static_cast<DataType>(lua_tonumber(L, -1));
                            obj_str += "datatype = " + std::to_string(static_cast<int>(data_type)) + ",\n";
                            lua_pop(L, 1);

                            lua_pushstring(L, "killdata");
                            lua_rawget(L, -2);
                            obj_str += "killdata = " + std::string((lua_toboolean(L, -1) ? "true" : "false")) + ",\n";
                            lua_pop(L, 1);

                            lua_pushstring(L, "data");
                            lua_rawget(L, -2);

                            if (data_type == DataType::None)
                                obj_str += "data = nil,\n";
                            else if (data_type == DataType::Bool)
                                obj_str += "data = " + std::string((lua_toboolean(L, -1) ? "true" : "false")) + ",\n";
                            else if (data_type == DataType::Float)
                                obj_str += "data = " + std::to_string(lua_tonumber(L, -1)) + ",\n";

                            else if (data_type == DataType::Vec2)
                            {
                                float x, y;
                                lua_rawgeti(L, -1, 1);
                                x = lua_tonumber(L, -1);
                                lua_pop(L, 1);

                                lua_rawgeti(L, -1, 2);
                                y = lua_tonumber(L, -1);
                                lua_pop(L, 1);

                                obj_str += "data = { " + std::to_string(x) + ", " + std::to_string(y) + " },\n";
                            }

                            else if (data_type == DataType::Vec2Array)
                            {
                                obj_str += "data = { ";
                                {
                                    lua_pushnil(L);
                                    while (lua_next(L, -2) != 0)
                                    {
                                        obj_str += "{";
                                        {
                                            lua_rawgeti(L, -1, 1);
                                            obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                            lua_pop(L, 1);

                                            lua_rawgeti(L, -1, 2);
                                            obj_str += std::to_string(lua_tonumber(L, -1));
                                            lua_pop(L, 1);
                                        }
                                        obj_str += "}, ";

                                        lua_pop(L, 1);
                                    }
                                }
                                obj_str += "},\n";
                            }

                            else if (data_type == DataType::Color)
                            {
                                obj_str += "data = { ";

                                lua_rawgeti(L, -1, 1);
                                obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                lua_pop(L, 1);

                                lua_rawgeti(L, -1, 2);
                                obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                lua_pop(L, 1);

                                lua_rawgeti(L, -1, 3);
                                obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                lua_pop(L, 1);

                                lua_rawgeti(L, -1, 4);
                                obj_str += std::to_string(lua_tonumber(L, -1)) + " ";
                                lua_pop(L, 1);

                                obj_str += "}\n";
                            }

                            else if (data_type == DataType::String || data_type == DataType::Filepath)
                                obj_str += "data = \"" + std::string(lua_tostring(L, -1)) + "\",\n";
                            else
                                obj_str += "data = nil\n";

                            lua_pop(L, 1);
                        }
                        obj_str += "},\n";
                    }
                }
                obj_str += "},\n";
                lua_pop(L, 1);

                lua_pushstring(L, "components");
                lua_rawget(L, tbl_obj);
                int tbl_components = lua_gettop(L);

                obj_str += "components = {\n";
                {
                    lua_len(L, -1);
                    int components_len = lua_tonumber(L, -1);
                    lua_pop(L, 1);

                    for (int j = 0; j < components_len; j++)
                    {
                        obj_str += "{\n";
                        {
                            lua_rawgeti(L, tbl_components, j + 1);

                            lua_pushnil(L);
                            while (lua_next(L, -2) != 0)
                            {
                                std::string key = lua_tostring(L, -2);
                                int param_type = lua_type(L, -1);

                                if (param_type == LUA_TNUMBER)
                                    obj_str += key + " = " + std::to_string(lua_tonumber(L, -1)) + ",\n";
                                else if (param_type == LUA_TSTRING)
                                    obj_str += key + " = \"" + std::string(lua_tostring(L, -1)) + "\",\n";
                                else if (param_type == LUA_TBOOLEAN)
                                    obj_str += key + " = " + (lua_toboolean(L, -1) == true ? "true" : "false") + ",\n";
                                else if (param_type == LUA_TTABLE)
                                {
                                    int param_table = lua_gettop(L);

                                    lua_pushstring(L, "__type");
                                    lua_rawget(L, param_table);
                                    std::string param_type = lua_tostring(L, -1);
                                    lua_pop(L, 1);
                                    
                                    if (param_type == "vec2array")
                                    {
                                        obj_str += "points = {\n";
                                        {
                                            obj_str += "__type = \"vec2array\",\n";
                                            obj_str += "__data = { ";

                                            lua_pushstring(L, "__data");
                                            lua_rawget(L, param_table);

                                            lua_pushnil(L);
                                            while (lua_next(L, -2) != 0)
                                            {
                                                obj_str += "{";
                                                {
                                                    lua_rawgeti(L, -1, 1);
                                                    obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                                    lua_pop(L, 1);
                                                    
                                                    lua_rawgeti(L, -1, 2);
                                                    obj_str += std::to_string(lua_tonumber(L, -1));
                                                    lua_pop(L, 1);
                                                }
                                                obj_str += "}, ";

                                                lua_pop(L, 1);
                                            }
                                            lua_pop(L, 1);

                                            obj_str += " },\n";
                                        }
                                        obj_str += "},\n";
                                    }

                                    if (param_type == "colorarray")
                                    {
                                        obj_str += "color = {\n";
                                        {
                                            obj_str += "__type = \"colorarray\",\n";
                                            obj_str += "__data = { ";

                                            lua_pushstring(L, "__data");
                                            lua_rawget(L, param_table);

                                            lua_pushnil(L);
                                            while (lua_next(L, -2) != 0)
                                            {
                                                obj_str += std::to_string(lua_tonumber(L, -1)) + ", ";
                                                lua_pop(L, 1);
                                            }
                                            lua_pop(L, 1);
                                            obj_str += "},\n";
                                        }
                                        obj_str += "},\n";
                                    }
                                }

                                lua_pop(L, 1);
                            }

                            lua_pop(L, 1);
                        }
                        obj_str += "},\n";


                    }

                }
                obj_str += "}\n";

                lua_pop(L, 1);
            }
            obj_str += "},\n";

            object_strings.push_back(obj_str);
        }

        std::ofstream data_file("save/data.lua");

        data_file << "project_settings = {\n";
        {
            data_file << "background_color = { " << m_BackgroundColor.r << ", " << m_BackgroundColor.g << ", " << m_BackgroundColor.b << ", " << m_BackgroundColor.a << " },\n";
            data_file << "shipping_mode = " << (m_ShippingModeToggle ? "true" : "false") << ",\n";
            data_file << "resizeable_window = " << (m_ResizeableWindow ? "true" : "false") << ",\n";
        }
        data_file << "}\n";

        data_file << "objects = {\n";
        {

            for (std::string str : object_strings)
                data_file << str;

        }
        data_file << "}";

        data_file.close();

        std::cout << "Saved!" << std::endl;
    }

    void Engine::ReloadObjects()
    {
        for (int i = 0; i < Objects.size();)
        {
            delete Objects[0];
            Objects.erase(Objects.begin());
        }

        TryLua(m_SaveL, luaL_dofile(m_SaveL, "save/data.lua"));

        lua_getglobal(m_SaveL, "objects");
        int obj_table = lua_gettop(m_SaveL);

        if (!lua_isnil(m_SaveL, -1))
        {
            lua_pushnil(m_SaveL);
            while (lua_next(m_SaveL, obj_table) != 0)
            {
                // -2 key, -1 value

                lua_pushstring(m_SaveL, "name");
                lua_gettable(m_SaveL, -2);
                std::string name = lua_tostring(m_SaveL, -1);
                lua_pop(m_SaveL, 1);

                lua_pushstring(m_SaveL, "active");
                lua_gettable(m_SaveL, -2);
                bool active = lua_toboolean(m_SaveL, -1);
                lua_pop(m_SaveL, 1);

                Object* object = new Object(name);
                Objects.push_back(object);
                object->Active = active;

                lua_pushstring(m_SaveL, "storage");
                lua_gettable(m_SaveL, -2);

                lua_len(m_SaveL, -1);
                int storage_len = lua_tonumber(m_SaveL, -1);
                lua_pop(m_SaveL, 1);

                for (int i = 0; i < storage_len; i++)
                {
                    lua_rawgeti(m_SaveL, -1, i + 1);
                    object->PushStorageItem(Parameter::Deserialize(m_SaveL));
                    lua_pop(m_SaveL, 1);
                }
                lua_pop(m_SaveL, 1);



                lua_pushstring(m_SaveL, "components");
                lua_gettable(m_SaveL, -2);

                lua_len(m_SaveL, -1);
                int comp_len = lua_tonumber(m_SaveL, -1);
                lua_pop(m_SaveL, 1);

                for (int i = 0; i < comp_len; i++)
                {
                    lua_rawgeti(m_SaveL, -1, i + 1);

                    lua_pushstring(m_SaveL, "__type");
                    lua_gettable(m_SaveL, -2);
                    std::string type = lua_tostring(m_SaveL, -1);
                    lua_pop(m_SaveL, 1);

                    Component* component = object->AddComponent(type);
                    component->Deserialize(m_SaveL);

                    lua_pop(m_SaveL, 1);
                }
                lua_pop(m_SaveL, 1);

                lua_pop(m_SaveL, 1);
            }
        }
    }

    void Engine::ReloadProjectSettings(bool _init)
    {
        m_BackgroundColor = Color::FromRGB(0.5, 0.5, 0.5, 1.0);

        lua_State* L = m_SaveL;

        TryLua(L, luaL_dofile(L, "save/data.lua"));
        lua_getglobal(L, "project_settings");
        int tbl_proj = lua_gettop(L);

        if (!lua_isnil(L, tbl_proj))
        {
            lua_pushstring(L, "background_color");
            lua_rawget(L, tbl_proj);

            lua_rawgeti(L, -1, 1);
            m_BackgroundColor.r = lua_tonumber(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, -1, 2);
            m_BackgroundColor.g = lua_tonumber(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, -1, 3);
            m_BackgroundColor.b = lua_tonumber(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, -1, 4);
            m_BackgroundColor.a = lua_tonumber(L, -1);
            lua_pop(L, 2);

            lua_pushstring(L, "resizeable_window");
            lua_rawget(L, tbl_proj);
            m_ResizeableWindow = lua_toboolean(L, -1);
            lua_pop(L, 1);

            if (_init == true)
            {
                lua_pushstring(L, "shipping_mode");
                lua_rawget(L, tbl_proj);

                m_ShippingMode = lua_toboolean(L, -1);
                lua_pop(L, 2);
            }
            else
                lua_pop(L, 1);
        }
    }

    void inline Engine::ResetViewport()
    {
        Render::ResetViewport(m_WindowWidth, m_WindowHeight);
    }

    #pragma endregion

    #pragma region Backend

	bool Engine::Init()
	{
        // GLFW/GLEW Init --------
        if (!glfwInit())
            return -1;

        // Load project settings.
        m_SaveL = luaL_newstate();
        ReloadProjectSettings(true);

        if (!m_ResizeableWindow && m_ShippingMode)
            glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_WindowTitle.c_str(), NULL, NULL);
        if (!m_Window)
        {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_Window);
        glfwSetScrollCallback(m_Window, ScrollCallback);

        // Enable VSync
        glfwSwapInterval(1);

        // Enable transparency blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (glewInit() != GLEW_OK)
        {
            std::cout << "[ERROR] GLEW Failed to initialize." << std::endl;
            return false;
        }

        // Setup virtual framebuffer
        if (!m_ShippingMode)
        {
            glGenFramebuffers(1, &m_GameFBO);
            glBindFramebuffer(GL_FRAMEBUFFER, m_GameFBO);

            glGenTextures(1, &m_GameTXO);
            glBindTexture(GL_TEXTURE_2D, m_GameTXO);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_WindowWidth, m_WindowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_GameTXO, 0);
            glBindTexture(GL_TEXTURE_2D, 0);

            int error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (error != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cout << "[OpenGL Error] Framebuffer Incomplete: " << error << std::endl;
                std::cout << std::endl;
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            ImGui::StyleColorsDark();
            ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
            ImGui_ImplOpenGL3_Init((char*)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));

            ImGuiStyle& style = ImGui::GetStyle();
            style.FrameRounding = 4.0f;
            style.IndentSpacing = 10.0f;
        }
        // -----------------------

        pep = new Texture("source/pep.png");
        Render::Setup(m_WindowWidth, m_WindowHeight);

        // Load objects once everything is ready.
        ReloadObjects();

        return true;
	}

    void Engine::Loop()
    {
        while (!glfwWindowShouldClose(m_Window))
        {

            // EVENT HANDLER //
            int event_count = GetEventCount(EventHandleType::EngineHandled);
            for (int i = 0; i < event_count; i++)
            {
                Event e = PullEvent(EventHandleType::EngineHandled);
                if (e.Type == EventType::AddObject)
                {
                    Object* obj = static_cast<Object*>(e.Data);
                    Objects.push_back(obj);
                }
                else if (e.Type == EventType::DestroyObject)
                {
                    Object* obj = static_cast<Object*>(e.Data);
                    for (int i = 0; i < Objects.size(); i++)
                    {
                        if (Objects[i] == obj)
                        {
                            delete obj;
                            Objects.erase(Objects.begin() + i);
                            break;
                        }
                    }
                }
            }
            // EVENT HANDLER //


            // INPUT PROCESSOR //
            glfwPollEvents();
            RegisterInput();

            if ((GetKey(LEFT_CONTROL).Held || GetKey(RIGHT_CONTROL).Held) && GetKey('S').Pressed)
            {
                SaveEngine();
            }

            // Panning
            if (!ImGui::IsAnyItemActive())
            {
                static glm::vec2 original_camera_pos(0, 0);
                float eng_cam_speed = 5.0f;
                glm::vec3 mv_delta(0, 0, 0);

                if (GetKey('A').Held) mv_delta.x += eng_cam_speed;
                if (GetKey('D').Held) mv_delta.x -= eng_cam_speed;
                if (GetKey('W').Held) mv_delta.y -= eng_cam_speed;
                if (GetKey('S').Held) mv_delta.y += eng_cam_speed;

                ImVec2 mouse_delta = { 0, 0 };

                if (GetKey(1).Pressed)
                    original_camera_pos = Render::EditorCamera->Position;

                if (GetKey(1).Held)
                    mouse_delta = ImGui::GetMouseDragDelta(1);

                if (GetKey(1).Released)
                    original_camera_pos = Render::EditorCamera->Position;

                glm::vec2 camera_pos = original_camera_pos + glm::vec2(mouse_delta.x, -mouse_delta.y);
                Render::EditorCamera->Position = glm::vec3(camera_pos.x, camera_pos.y, 0);
            }

            // Scrolling
            if (!ImGui::IsAnyItemHovered())
            {
                float zoom_speed = 0.1f;
                if (yScroll != 0)
                {
                    Render::EditorCamera->Zoom += yScroll * zoom_speed;
                    yScroll = 0;

                    if (Render::EditorCamera->Zoom <= 0.1) Render::EditorCamera->Zoom = 0.1;
                }
            }
            else
                yScroll = 0;
            // INPUT PROCESSOR //


            // VIEWPORT MONITOR //
            int _winWidth, _winHeight;
            glfwGetFramebufferSize(m_Window, &_winWidth, &_winHeight);
            if ((_winWidth != m_WindowWidth || _winHeight != m_WindowHeight) && m_ShippingMode == true)
            {
                m_WindowWidth = _winWidth;
                m_WindowHeight = _winHeight;
                ResetViewport();
            }
            // VIEWPORT MONITOR //


            // GAME RENDERING
            if (!m_ShippingMode)
                glBindFramebuffer(GL_FRAMEBUFFER, m_GameFBO);

            glClearColor(m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
            glClear(GL_COLOR_BUFFER_BIT);

            for (Object* obj : Objects)
            {
                if (obj->Active)
                {
                    for (Component* comp : obj->Components)
                    {
                        if (comp->GetType() == "ScriptComponent")
                        {
                            ScriptComponent* sc = static_cast<ScriptComponent*>(comp);

                            if (sc->ScriptData->HasEnginePtr == false && sc->ScriptData->L != nullptr)
                            {
                                lua_pushlightuserdata(sc->ScriptData->L, this);
                                lua_setglobal(sc->ScriptData->L, "__engineptr");
                                sc->ScriptData->HasEnginePtr = true;
                            }
                        }

                        // PreTick
                        comp->OnTick(false, false);

                        // GameTick
                        if (comp->GetType() != "ScriptComponent" || (m_GameMode || m_ShippingMode))
                            comp->OnTick(true, false);
                    }
                }
            }
            // GAME RENDERING

            // ENGINE RENDERING
            if (!m_ShippingMode)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);

                glClearColor(m_BackgroundColor.r + 0.25, m_BackgroundColor.g + 0.25, m_BackgroundColor.b + 0.25, 1);
                glClear(GL_COLOR_BUFFER_BIT);

                for (Object* obj : Objects)
                {
                    if (obj->Active)
                    {
                        for (Component* comp : obj->Components)
                        {
                            comp->OnTick(false, true);
                        }
                    }
                }

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                if (m_ShowImGuiDemo)
                    ImGui::ShowDemoWindow(&m_ShowImGuiDemo);

                ShowMenuBar();
                ShowFileBrowser();
                ShowGameView();
                ShowObjectView();
                ShowProjectSettings();

                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }
            // ENGINE RENDERING


            // TIME VARIABLES
            m_CurrentTime = glfwGetTime();
            m_Delta = m_CurrentTime - m_LastTime;
            m_LastTime = glfwGetTime();

            if (m_GameMode || m_ShippingMode) m_TimeElapsed += m_Delta; 
            else m_TimeElapsed = 0.0f;
            // TIME VARIABLES


            /* Swap front and back buffers */
            glfwSwapBuffers(m_Window);
        }

        if (!m_ShippingMode && !m_GameMode)
            SaveEngine();
    }

	void Engine::Run(int w, int h, std::string title)
	{
        m_WindowWidth = w;
        m_WindowHeight = h;
        m_WindowTitle = title;

        Init();
        Loop();

        if (!m_ShippingMode)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        glfwTerminate();
	}

    #pragma endregion

    void RegisterScript(lua_State* L, Object* _parent)
    {
        luaL_openlibs(L);
        LuaColor::lua_RegisterColor(L);
        LuaObject::lua_RegisterObject(L);
        LuaComponent::lua_RegisterComponent(L);
        LuaVector2::lua_RegisterVec2(L);
        Shape::lua_RegisterShape(L);
        Engine::lua_RegisterInput(L);
        Engine::lua_RegisterHelpers(L);

        // Register script table
        
        lua_newtable(L);
        lua_setglobal(L, "script");
        lua_getglobal(L, "script");

        lua_pushstring(L, "parent");
        LuaObject::lua_New(L, _parent);

        lua_settable(L, -3);
    }

}
