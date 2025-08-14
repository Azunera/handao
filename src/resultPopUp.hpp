
#ifndef RESULTPOPUP_HPP
#define RESULTPOPUP_HPP

// #define NOMINMAX
#include <windows.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System/Vector2.hpp>
// #include <Windows.h>
// #undef max
#include <string>
#include <iostream>
#include <optional>

#include "dictionary.hpp"


using std::optional;

namespace hanzi_faxian {
    
// Same as the struct Entry but with wide characters utf-16. Intended for proper display in windows sfml.
struct EntryData {
    std::wstring simplified_character;
    std::wstring traditional_character;
    std::wstring pinyin;
    std::wstring definition;

    // Make conversion from the non_wide to the wide one
    static EntryData fromEntry(const Entry& non_wide_entry_data)
    {
        return EntryData{
            wstring_from_utf8(non_wide_entry_data.simplified_character.c_str()),
            wstring_from_utf8(non_wide_entry_data.traditional_character.c_str()),
            wstring_from_utf8(non_wide_entry_data.pinyin.c_str()),
            wstring_from_utf8(non_wide_entry_data.definitions.c_str())
        };
  
    }
};

// Class containing the sf elements of the texts, containing values such as posiitions and offsets
class EntryStruct {
    private:
        sf::Vector2f offSet;
        sf::Vector2f global_definition_brl;
        const sf::Font& font;
        constexpr static float font_size = 24.f;

    public:
        sf::Text simplified_hanzi;
        sf::Text traditional_hanzi;
        sf::Text pinyin;
        std::vector<sf::Text> definitions;

        EntryStruct(const sf::Font& font, EntryData entry_data, sf::Vector2f offSet);
        void wrapText(float width_limit);
        void positionText();
        float get_max_x() const;
        float get_max_y() const;
};

// The pop up object, due to architecture design, it doens't contain a function for the main running loop. Instead it should be implemented manually inside main.cpp
class ResultPopUp {
    private:
        sf::RenderWindow window;
        sf::RenderWindow* pwindow;
        sf::Color lightGreen;
        sf::Vector2i windowPos; 
        sf::Font font;

        std::vector<EntryData> entriesData;
        std::vector<EntryStruct> entriesTexts;
        float window_right;
        float window_bottom;
        
        int m_width_limit;
        int m_height_limit;
        const int font_size = 32;

    protected:
        void correctWindowOverflow();

    public:
        ResultPopUp();
        void update_position(std::pair<int, int> new_pos);
        void update_text(std::vector<EntryData> entries_data);;
        void poll_events();
        void display();
        void draw_elements();

        // Displayabitlity
        void hide_window();
        void reveal_window();
        // bool checks
        bool is_open();

    };

}

#endif 