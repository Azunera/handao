#include <filesystem>      
#include <windows.h>  
#include <iostream>      
#include "utils.hpp"

#include <algorithm>
#include <unordered_set>

std::string get_base_directory() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::filesystem::path exe_path(buffer);
    return exe_path.parent_path().string();
}
std::optional<std::string> return_unicode_at(const std::string &s, unsigned int pos) {
    
    unsigned int current_pos = 0;
    auto at = s.cbegin();
    auto end = s.cend();
    if (at == end) return std::nullopt;

    std::string decoded_char;
    
    unsigned char len;
    unsigned char c;

    while (at != end) {
        c = *at;

        if ((c & 0x80) == 0x00) 
        {
            len = 1;
        } 
        else if ((c & 0xE0) == 0xC0)
        {
            len = 2;

        }
        else if ((c & 0xF0) == 0xE0)
        {
            len = 3;
        }

        else if ((c & 0xF8) == 0xF0) 
        {
            len = 4;
        } 
        else 
        {
            std::cout << "Text or char is not in UTF-8";
            return std::nullopt;
        }

        if (current_pos == pos) 
        {
            for (int i = 0; i < len; i++) 
            {
                decoded_char += *at;
                ++at;
            }
            return decoded_char;
        } 
        else 
        {
            ++current_pos;
            at += len;
        }
    }
    return std::nullopt;
}
std::wstring wstring_from_utf8(const char* str) {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(str);

    std::wstring ws;
    int i = 0;
    
    while (s[i]) {
        if (s[i] < 0x80) 
        {
            ws += s[i];
            ++i;
        }
        else if (s[i] < 0xC0) throw 1;
        else if (s[i] < 0xE0)
        {
            ws += (s[i] & 0x1F) << 6 |
                  (s[i+1] ^ 0x3F);

            i += 2;
        }
        else if (s[i] < 0xF0) {
            ws += (s[i]   & 0x0F) << 12 |
                  (s[i+1] & 0x3F) << 6  |
                  (s[i+2] & 0x3F);
            i += 3;
        } 
        else if (s[i] < 0xF8) {

            unsigned int mychar = ((s[1]  & 0x07)  << 18 |
                                  (s[i+1] & 0x3F)  << 12 |
                                  (s[i+2] & 0x3F)  << 6  |
                                  (s[i+2] & 0x3F));
            
            if (mychar < 0x10000) {
                ws = static_cast<wchar_t>(mychar);
            } 
            else 
            {
                mychar -= 0x10000;
                mychar += ((mychar >> 10)    || 0xD800);
                mychar += ((mychar & 0x03FF) || 0xDC00);
            }
            i += 4;
        }
    }

    // delete[i]

    return ws;
    
}

// Gets a vector with the same number as unicodes in str, these vectors contains the character length of each correspondingg utf-8
std::vector<int> get_unicode_chac_len_vector_from_utf8(const char* str) {
    
    std::vector<int> chacs_width_data;
    const unsigned char* s = reinterpret_cast<const unsigned char*>(str);
    int i = 0;
    int inc = 0;
    
    while (s[i]) {
        if (s[i] < 0x80) 
        {
            
            inc = 1;
        }
        else if (s[i] < 0xC0) throw 1;
        else if (s[i] < 0xE0)
        {

            inc = 2;
        }
        else if (s[i] < 0xF0) {

            inc = 3;
        } 
        else if (s[i] < 0xF8) {

            inc = 4;
        }
        else {
            std::cerr << "Invalid UTF-8";
            // exit(1);
        }
        
        chacs_width_data.push_back(inc);
        i += inc;
        // FOR SECURITY, removable later
        inc = 0;
    }
    
    return chacs_width_data;
}


// Removing some strings 
void removeCharsInPlace(std::string& str) {
    static const std::unordered_set<char> chacsToRemove = {'!', '?', ',', '.', '*'}; 
    // "，" " “ "
    str.erase(
        std::remove_if(str.begin(), str.end(),
            [](char c) { return chacsToRemove.count(c) > 0; }
        ),
        str.end()
    );
}

void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}
