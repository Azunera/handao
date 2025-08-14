#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System/Vector2.hpp>
#include <windows.h>
// #include <string>
#include <iostream>
#include <optional>
#include <vector>
#include "resultPopUp.hpp"
#include "utils.hpp"
using std::optional;

namespace hanzi_faxian {

// <------------- DEFINING THE ENTRY DATA STRUCTURE ------------->

EntryStruct::EntryStruct(const sf::Font& font, EntryData entry_data, sf::Vector2f offSet) 
    : simplified_hanzi(font, entry_data.simplified_character, font_size * 1.5),
    traditional_hanzi(font, entry_data.traditional_character, font_size * 1.5),
    pinyin(font, entry_data.pinyin, font_size * 1.4),   
    offSet(offSet),
    global_definition_brl({0,0}),
    font(font)
{
    if (entry_data.simplified_character == entry_data.traditional_character)
    {
        traditional_hanzi.setString("");
    }
    sf::Text first_def(font, entry_data.definition, font_size);
    first_def.setPosition(offSet);
    definitions.push_back(first_def); 
    simplified_hanzi.setFillColor(sf::Color::Black);
    traditional_hanzi.setFillColor(sf::Color::Black);
    pinyin.setFillColor(sf::Color::Black);
    
    pinyin.setStyle(sf::Text::Italic);

}

void EntryStruct::wrapText(float width_limit) 
{
    std::string current_line, current_word;
    sf::FloatRect definition_bounds = this->definitions[0].getGlobalBounds();
    // Base length
    float base_len = definition_bounds.position.x;
    // Max len before wrapping
    float max_prewrap_len = definition_bounds.position.x + definition_bounds.size.x;
    // Current length in itneration for wrapping
    float current_len = base_len;

    if (max_prewrap_len > width_limit) 
    {   
        std::string entryString = static_cast<std::string>(this->definitions[0].getString());
        this->definitions.clear();
        auto endp = entryString.end();
        
        // The next line that will be added in the wrapping
        std::string next_line;
        // Word that will be constructed progressively and added to the next line
        std::string word = "";
        // Necessray for offsetting the next line when one is cleared but current word length hasn't been added to the next line yet
        float word_len = 0;
        // The advanced of a glyph, its width, gotten later by getGlpyh().advance
        float chac_advance = 0;

        for (auto it = entryString.begin(); it != entryString.end(); ++it)
        {
            // If previous itineration beat width limit, push the line to the definitions, clear current line and update to the new lew
            if (current_len > width_limit)
            {
                this->definitions.push_back(sf::Text(font, next_line, font_size));
                next_line = "";
                current_len = base_len + word_len;
            }
            // Acquire the width of the glypth to sum to the lenses
            chac_advance = font.getGlyph(*it, font_size, 0).advance;
            current_len += chac_advance;
            word_len += chac_advance;
            // Adding the chac to the length
            word += *it;
            
            // If the chac is empty, add the word to the line, cleaning the word and the length of the word
            if (*it == ' ') 
            {
                next_line += word; 
                word = "";
                word_len = 0;
            }
        }   
        // If next line is not empty, and the last line
        if (!next_line.empty()) 
        {
            next_line += word;
            this->definitions.push_back(sf::Text(font, next_line, font_size));
        }
    }
    // Set color black to all lines
    for (int i = 0; i < this->definitions.size(); i++) 
    {
        this->definitions[i].setFillColor(sf::Color::Black);
    }
}

void EntryStruct::positionText() 
{
    float y = offSet.y;
    float x = offSet.x;
    
    // If there is a traditional hanzi, add the offset +15, if not, its for avoiding pinyin to be pushed too far away
    float trad_hanzi_offset = this->traditional_hanzi.getString() != "" ? 15 : 0;
    
    this->simplified_hanzi.setPosition(offSet);
    sf::FloatRect s_hanzi_bounds = this->simplified_hanzi.getGlobalBounds();
    float s_hanzi_right = s_hanzi_bounds.position.x + s_hanzi_bounds.size.x;
    float s_hanzi_bottom = s_hanzi_bounds.position.y + s_hanzi_bounds.size.y;
    
    // Its selected a little bit ot the right of simplified hanzi
    this->traditional_hanzi.setPosition({s_hanzi_right + trad_hanzi_offset, offSet.y});
    sf::FloatRect t_hanzi_bounds = this->traditional_hanzi.getGlobalBounds();
    float t_hanzi_right = t_hanzi_bounds.position.x + t_hanzi_bounds.size.x;
    
    // Pinyin are right from trad hanzi
    this->pinyin.setPosition({t_hanzi_right + 30, offSet.y});
    sf::FloatRect pinyin_bounds = this->pinyin.getGlobalBounds();
    float pinyin_right = pinyin_bounds.position.x + pinyin_bounds.size.x;


    for (int i = 0; i < this->definitions.size(); ++i)
    {
        // If its the first line
        if (i == 0) 
        {
            this->definitions[i].setPosition({offSet.x, s_hanzi_bottom + (font.getLineSpacing(font_size) * 0.5f)});
        }
        else 
        {
            sf::FloatRect prev_definition_bounds = definitions[i-1].getGlobalBounds();
            float prev_definition_bounds_bottom = prev_definition_bounds.position.y + prev_definition_bounds.size.y;
            this->definitions[i].setPosition({offSet.x, prev_definition_bounds_bottom + (font.getLineSpacing(font_size) * 0.5f)});
        }
        // CAN BE OPTIMIZED TO NOT COMPUTE OR GET THE PREV DEFINIITON BOUNDS SO OFTEN
        sf::FloatRect definition_bounds = definitions[i].getGlobalBounds();
        float local_definition_most_right = definition_bounds.position.x + definition_bounds.size.x;
        float local_definition_most_bottom = definition_bounds.position.y + definition_bounds.size.y;
        if (local_definition_most_right > global_definition_brl.x) global_definition_brl.x = local_definition_most_right;
        if (local_definition_most_bottom > global_definition_brl.y) global_definition_brl.y = local_definition_most_bottom;
    }
}

float EntryStruct::get_max_x() const
{
    return (std::max)(this->global_definition_brl.x, pinyin.getGlobalBounds().position.x + pinyin.getGlobalBounds().size.x);
}
float EntryStruct::get_max_y() const
{
    return this->global_definition_brl.y;
}

/// <------------- DEFINING THE RESULT POP UP ------------->
ResultPopUp::ResultPopUp() 
    :   window(sf::VideoMode({400, 300}), "", sf::Style::None),
        pwindow(&window),
        m_width_limit(static_cast<int>(sf::VideoMode::getDesktopMode().size.x)),
        m_height_limit(static_cast<int>(sf::VideoMode::getDesktopMode().size.y)),
        // font(get_base_directory() + "/SourceHanSans-Normal.ttf"),
        lightGreen({144, 238, 144}),
        windowPos({100, 100})
{
    window.setFramerateLimit(15);
    HWND hwnd = window.getNativeHandle();

    // Adding additional attributes to the window
    LONG exStyle = GetWindowLongPtrA(hwnd, GWL_EXSTYLE); 
    exStyle |= WS_EX_LAYERED          // Required for transparency
            | WS_EX_TRANSPARENT       // Make it lick-through
            | WS_EX_TOOLWINDOW        // Hide from taskbar and Alt+Tab
            | WS_EX_TOPMOST;          // Alwaystay on top
    SetWindowLongPtrA(hwnd, GWL_EXSTYLE, exStyle);
    // Set transparency level (alpha blending), max is 256, so 240 is about 94%~
    SetLayeredWindowAttributes(hwnd, 0, 240, LWA_ALPHA); 
    // Making the window at the most top
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE
    );

    // Load font
    if (!font.openFromFile(get_base_directory() + "/SourceHanSans-Normal.ttf")) {
        std::cerr << "Failed to load font. A file named SourceHanSans-Normal.ttf may be missing, you may have deleted accidentally or you are dealing with a tempered version. You can get the font used on the github.\n";
        exit(1);
    }
}   

void ResultPopUp::update_text(std::vector<EntryData> entries_data) {

    entriesTexts.clear();
    sf::Vector2f previousEntryOffset = {25.f, 25.f};
    float max_x_found = 0;
    for (int i = 0; i < entries_data.size(); i++ )  
    {
        if (i > 0) 
        {
            previousEntryOffset.y = entriesTexts[i-1].get_max_y() + 10.f;
        }
        EntryStruct entry_texts(font, entries_data[i], {previousEntryOffset});
        entry_texts.wrapText(1200.f);
        entry_texts.positionText();
        entriesTexts.push_back(entry_texts);

        float temporal_max_x = entriesTexts[i].get_max_x() + 50.f;
        
        if (temporal_max_x > max_x_found) {max_x_found = temporal_max_x; };

        if (i == entries_data.size() - 1 )
        {
            float last_max_bottom = entriesTexts[i].get_max_y() + 25.f;
            (last_max_bottom < m_height_limit - 25.f) ? window_bottom = last_max_bottom : window_bottom = m_height_limit;
        }
    }
    (max_x_found < m_width_limit - 25.f) ? window_right = max_x_found : window_right = m_width_limit;

    window.setSize({static_cast<unsigned>(window_right), static_cast<unsigned>(window_bottom)});
    correctWindowOverflow();
}   

// Need to rename or separate this fucntion later, because does not just correct Window overflow but is the way to set the window size
void ResultPopUp::correctWindowOverflow() 
{
    sf::Vector2i screen_resolution = {static_cast<int>(sf::VideoMode::getDesktopMode().size.x), static_cast<int>(sf::VideoMode::getDesktopMode().size.y)};
    // windowPos = {windowPos.x - static_cast<int>(window_right/2.f), windowPos.y - static_cast<int>(window_bottom * 1.2f)};
    windowPos = {windowPos.x - 10, windowPos.y + 40};

    sf::Vector2i final_pos = windowPos;
    sf::Vector2i bottom_right = { static_cast<int>(window.getSize().x) + windowPos.x, static_cast<int>(window.getSize().y) + windowPos.y };

    if (bottom_right.x >= screen_resolution.x) { final_pos.x -= (bottom_right.x - screen_resolution.x); }
    if (bottom_right.y >= screen_resolution.y) { final_pos.y -= (bottom_right.y - screen_resolution.y); }
    window.setPosition(final_pos);
}

// Updating the variable of the position of the mouse 
void ResultPopUp::update_position(std::pair<int, int> new_pos) {
    windowPos = {new_pos.first, new_pos.second};
};

void ResultPopUp::draw_elements() 
{
    window.clear(lightGreen);
    for (EntryStruct entry : entriesTexts) 
    {
        window.draw(entry.simplified_hanzi);
        window.draw(entry.traditional_hanzi);
        window.draw(entry.pinyin);
        for (sf::Text definition_text : entry.definitions) 
        {
            window.draw(definition_text);
        }
    }
}

void ResultPopUp::poll_events() 
{
    // Enables correct fix of the text propotion  when page resized
    while (optional event = pwindow->pollEvent()) {
        if (auto* resizeEvent = event->getIf<sf::Event::Resized>()) {
            {
                sf::FloatRect visibleArea({0, 0}, {static_cast<float>(resizeEvent->size.x), static_cast<float>(resizeEvent->size.y)});
                window.setView(sf::View(visibleArea));
            }
        }
    }
}

void ResultPopUp::display()
{
    window.display();
}

bool ResultPopUp::is_open()
{
    return window.isOpen();
} 

void ResultPopUp::hide_window()
{
    window.setVisible(false);
}

void ResultPopUp::reveal_window()
{
    window.setVisible(true);
}


}