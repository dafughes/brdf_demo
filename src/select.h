#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

template<typename T>
class Select
{
public:
    Select()
    {}
    template<typename... Args>
    Select(std::string label, Args&&... args) : m_label(label), m_selected(0)
    {
        m_items.reserve(sizeof...(Args));
        (m_items.emplace_back(std::forward<Args>(args)), ...);
    }

    bool update()
    {
        bool result = false;

        auto current = to_string(m_items[m_selected]);
        if (ImGui::BeginCombo(m_label.c_str(), current.c_str()))
        {
            for (int n = 0; n < m_items.size(); n++)
            {
                bool is_selected = (m_selected == n);
                auto item = to_string(m_items[n]);
                if (ImGui::Selectable(item.c_str(), is_selected))
                {
                    m_selected = n;
                    result = true;
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        return result;
    }

    T& selected()
    {
        return m_items[m_selected];
    }

private:
    static std::string to_string(T& item)
    {
        std::ostringstream ss;
        ss << item;
        return ss.str();
    }

    std::string m_label;
    std::vector<T> m_items;
    int m_selected;
};