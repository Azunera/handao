#include <utility>
#include <iostream>
#include <fstream>
#include <windows.h>
#include <gdiplus.h>
#include <chrono>
#include <thread>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <thread>
#include <atomic>

#include <codecvt>
#include <cstdint>
#include <locale>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

#include "utils.hpp"
#include "resultPopUp.hpp"
#include "dictionary.hpp"

#pragma comment(lib, "gdiplus.lib")

using namespace hanzi_faxian;

// Checks if shortcut LControl + LAlt + F has been pressed for reactivating processing. This function has to be threaded
void check_shortcut(std::atomic<bool>& process_flag, std::atomic<bool>& activation_flag, std::atomic<bool>& terminate_flag, std::atomic<bool>& popup_active_flag) 
{
    bool is_clock_fresh = false;
    std::chrono::steady_clock::time_point start;

    bool searching_content = true;
    bool gkey_green_light = true;
    // Offset for the checking of checking if any key was pressed for deactivating current pop up 
    int key_search_offset = 0;
    constexpr double forgiviness_threshold = 0.5;

    std::set<int> exclude = {VK_CONTROL, VK_MENU, VK_LMENU, VK_LCONTROL, 'F', 'G', 'Y', VK_VOLUME_MUTE, VK_VOLUME_MUTE, VK_VOLUME_UP};
    
    while (true)
    {
        bool ctrl_pressed = GetAsyncKeyState(VK_LCONTROL) && 0x8000;
        bool alt_pressed  = GetAsyncKeyState(VK_LMENU) && 0x8000;
        bool f_pressed    = GetAsyncKeyState('F') && 0x8000;
        bool g_pressed    = GetAsyncKeyState('G') && 0x8000;
        bool y_pressed    = GetAsyncKeyState('Y') && 0x8000;
        bool click        = GetAsyncKeyState(VK_LBUTTON) && 0x800;

        if (ctrl_pressed && alt_pressed)
        {
            if ((f_pressed || click) && searching_content)
            {
                std::cout << "Looking up for new word..." << "\n";
                process_flag = true;
                searching_content = false;  
                // 2 avoids checking for mouse while ctrl+alt+click is active wont dissappear immadiately  
                key_search_offset = 2;     
            }
            if (g_pressed && gkey_green_light)
            {
                activation_flag = !activation_flag;     
                process_flag = false;
            }
            if (y_pressed)
            {
                std::cout << "Terminating program" << "\n";

                terminate_flag = true;
            }
        }
        // May create some debug sometimes if both F and click wanted to be pressed at the same time. But for the sake of simplicity it will stay like this for now
        if (!f_pressed && !click)
        {
            searching_content = true;
            key_search_offset = 1;
        }
        if (!g_pressed)
        {
            gkey_green_light = true;
        }

        if (popup_active_flag)
        {
            if (!is_clock_fresh)
            {
                start = std::chrono::steady_clock::now();
                is_clock_fresh = true;
            }
            auto now = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = now - start;
            if (elapsed.count() <= forgiviness_threshold)
            {
                Sleep(300);
                continue;
            }
            
            for (int vk = key_search_offset; vk < 123; vk++) 
            {
                if (exclude.find(vk) != exclude.end()) continue; 
                if (GetAsyncKeyState(vk) && 0x8000)
                {
                    popup_active_flag = false;
                    is_clock_fresh = false;
                }
            }
        }
        // <!------------ SLEEP ------------->
        Sleep(200);
    }
}

// Get the position of the cursor on screen
std::pair<int,int> get_cursor_pos() {
    POINT point;
    if (GetCursorPos(&point)) {
        return {point.x, point.y};
    } else {
        return {-1,-1};
    }
}

/* Captures the screen region, in this case using it for full resolution screenshots. 
Done through copying the pixels of DC's screen into a DC 
that will draw it into capture screen region */


HBITMAP capture_screen_region(int x, int y, int width, int height) {
    // Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    // ULONG_PTR gdiplusToken;
    // Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Get the DC for the entire window, that's why nullptr not specfying to a specific window
    HDC hdcScreen = GetDC(nullptr);
    // Creates a DC compatible with current screen in memory
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    // Create a bitmap compatible with the hdcSCreen
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);

    // Selects the object of which the memory DC will be pinting towards
    SelectObject(hdcMem, hBitmap);

    // Bit Block Transfer (BitBlt), transferring rect pixels from one DC to another, basically, 
    // Copy (SRCCOPY) from the DC (hdcSCreen) starting at offset x,y the width and height rectangle of pixels,
    // into the DC of memory (hdcMem) with offset 0,0 
    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);

    // Delete and release the in-memory and screen DC
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return hBitmap;
}

// Proccesses the screen bitmap into a pix for tesseract
PIX* HBITMAP_to_gray_scale(HBITMAP hBitmap) {
    BITMAP bmp;
    // Verify bitmap is correct and get it
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) return nullptr;

    // Width of the map in pixels
    int width = bmp.bmWidth;
    // Rows of pixels
    int height = bmp.bmHeight;
    int stride = ((width * 3 + 3) & ~3);  // 24-bit padded to be multiplitive of 4
    BYTE* bits = new BYTE[stride * height];

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); // The respect size of a BITMATINFOHEADER
    bmi.bmiHeader.biWidth = width;       // The total width
    bmi.bmiHeader.biHeight = -height;    // top-down
    bmi.bmiHeader.biPlanes = 1;          // Always at 1, no need other value
    bmi.bmiHeader.biBitCount = 24;       // RGB bit count
    bmi.bmiHeader.biCompression = BI_RGB;// Expands to rgb;

    HDC hdc = GetDC(NULL);
    /* 
    GetDiBits fundamentally passes to the bits variable the bits from hbitmap
    Prams usage:
        First getdbi gets a handle ofr the DC, the hdc,  then the compatible hbitmap to copy bits from, thne 0 is the first scan line to retrieve,
     then height corresponds to the amounts oflines, then bits is the pointer of the buffer to receive the bitmap data, 
     then the pointer to bmi the bitmapinfo to explain the structure data for the desired DIB data., and finally the format for the bmiColors is RGB
     */
    if (!GetDIBits(hdc, hBitmap, 0, height, bits, &bmi, DIB_RGB_COLORS)) {
        ReleaseDC(NULL, hdc);
        delete[] bits;
        return nullptr;
    }
    ReleaseDC(NULL, hdc);

    //Create 32bpp RGB PIX from 24bpp bits
    PIX* pix_rgb = pixCreate(width, height, 32);
    for (int y = 0; y < height; ++y) {
        // Destination row for a leptonica pixel
        l_uint32* line = pixGetData(pix_rgb) + y * pixGetWpl(pix_rgb);
        // Pointer to the source row of the bits, will be traversed for the pixel bytes, being uint8 let me traverse more easily.
        BYTE* row = bits + y * stride;
        for (int x = 0; x < width; ++x) {
            BYTE B = row[x * 3 + 0];
            BYTE G = row[x * 3 + 1];
            BYTE R = row[x * 3 + 2];
            // Composing RGB pixel is the safe way to compose a pixel for a pointer of a PIX's pixelmap
            composeRGBPixel(R, G, B, &line[x]);
        }
    }
    delete[] bits;

    // Converting to 8-bit grayscale
    PIX* pix_gray = pixConvertRGBToLuminance(pix_rgb); 
    pixDestroy(&pix_rgb);
 
    return pix_gray;
}
// Tesseract parses the text.
std::string parse_image_text(tesseract::TessBaseAPI *api, Pix *pix_image) {
        char *outText;
        
        api->SetPageSegMode(tesseract::PSM_SINGLE_BLOCK); 

        api->SetImage(pix_image);
        tesseract::ResultIterator* ri = api->GetIterator();
        tesseract::PageIteratorLevel level = tesseract::RIL_WORD;

        outText = api->GetUTF8Text();

        pixDestroy(&pix_image);
        std::string stringText = static_cast<std::string>(outText);
        removeCharsInPlace(stringText);
        return stringText;
    };
  
std::pair<std::vector<EntryData>, std::pair<int,int>> process_input_logic(tesseract::TessBaseAPI *tesseract_api, CedictDictionary *m_dictionary)
{
    // <---------- LOCATING MOUSE ON SCREEN ---------->  
    std::pair<int, int> cursor_pos = get_cursor_pos();
    constexpr int hanzi_size = 50; // still fine

    // Most part of these constsexpr are temporal, and may be modifiable or customizable later losing constexpr
    // The width is recommended to be at least 7 times the hanzi size. But if it needs to be adjusted, it can be reduced to 4 at least.
    constexpr int detection_width = hanzi_size * 7; 
    constexpr int detection_height = hanzi_size * 1.1;
    constexpr int x_offset = detection_width / 2;
    constexpr int y_offset = detection_height / 2;

    HBITMAP the_map = capture_screen_region(cursor_pos.first - hanzi_size/2, cursor_pos.second - hanzi_size/2, detection_width, detection_height);

    //Retrieving the Leptonica's image type Pix from the hbitmap, for then be procssed in tesseract
    Pix* pix = HBITMAP_to_gray_scale(the_map);
    // Scalling for better precision in tesseract
    pix = pixScale(pix, 2,2);

    // <---------- RECOGNIZING THE TEXT IN TESSERACT ---------->  
    std::string recognized_characters = parse_image_text(tesseract_api, pix);

    // <---------- LOOKUPS IN DICTIONARY ---------->  
    std::vector<const hanzi_faxian::Entry *> u8_entry_data;
    if (auto opt = m_dictionary->lookup_composed(recognized_characters)) 
    {
        u8_entry_data = opt.value();
    }
    else
    {
        std::cerr << "No word found.\n\n";
    }

    std::vector<EntryData> u16_entry_data;
    std::string found_words_print_string = "";

    if (u8_entry_data.size() > 0) 
    {
        for (const Entry* entry : u8_entry_data) 
        {
            found_words_print_string += entry->simplified_character + ", ";
            u16_entry_data.push_back(EntryData::fromEntry(*entry));
        }
        std::cout << "Found words:" << recognized_characters << "\n";
    }
    else 
    {   
        EntryData fallback_entry;
        fallback_entry.simplified_character = L"Could not find a character"; 
        fallback_entry.traditional_character = L"";
        fallback_entry.pinyin = L":(";
        fallback_entry.definition = L"Please try to zoom out more, and select non-obstructed and aligned text";
        u16_entry_data.push_back(fallback_entry);
    }

    return std::make_pair(u16_entry_data, cursor_pos);

}

int main() {
    // Flag for starting processing next definition
    std::atomic<bool> process_flag = false;
    // Flag to check if program is activated or not, enabling other usages for the hotkeys and reducing resources usage
    std::atomic<bool> activation_flag = true;
    // Flag for terminating the program and desconstruct
    std::atomic<bool> termination_flag = false;
    // Flag to check if a current pop up is activate
    std::atomic<bool> popup_active_flag = false;

    std::thread t1(check_shortcut, std::ref(process_flag), std::ref(activation_flag), std::ref(termination_flag), std::ref(popup_active_flag));

    // <---------- PARSING DICTIONARY ---------->  
    std::cout << "Welcome to Handao! For any errors or bugs contact me! Check how you can on the github:\n";
    std::cout << "Please wait for the program to load before starting!:\n\n";
    std::cout << "Loading dictionary...\n";
    CedictDictionary m_dictionary = CedictDictionary((get_base_directory()+"\\"+"cedict_ts.u8").c_str());
    std::cout << "Dictionary has loaded succesfully!\n\n";

    std::cout << "Instructions:\n - Ctrl + Alt + F or Left Click: Find hanzi over the mouse\n - Ctrl + Alt + Y: Close the program, if it doesn't work, type ctrl+c on this terminal\n - Ctrl + Alt + G to temporally disable the program, letting use the other shortcuts for other usages\n\n";

    
    // <----------- INITIALIZING TESSERACT  ---------->  

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if (api->Init((get_base_directory()+"\\"+"tessdata").c_str(), "chi_sim+chi_tra")) {
        fprintf(stderr, "Could not initialize Tesseract. If you per chance find this bug, you may be dealing with a tempered or bugged version of this project.\n In order to fix this problem you have to download the chi_sim and chi_rad tessdata from \"https://tesseract-ocr.github.io/tessdoc/Installation.html\". Then create a folder named tessdata on same directory as the .exe, then copy those training data into that new folder then restart the program.");
        exit(1);
    }

    // <---------- CREATING THE POP UP IN MEMORY FOR THE FIRST TIME ---------->  
    ResultPopUp WindowPopUp;

    while (WindowPopUp.is_open())
    {   
        if (termination_flag)
        {
            api->End();
            delete api;
            t1.join();
            exit(1);
        }
        if (!activation_flag)
        {
            WindowPopUp.hide_window();
            continue;
        }

        else if (process_flag)
        {
            // Before starting process hide current window
            popup_active_flag = false;
            WindowPopUp.hide_window();
            // Reprocess new inputs
            auto output = process_input_logic(api, &m_dictionary);
            std::vector<EntryData> u16_entry_data = output.first;
            std::pair<int,int> cursor_pos = output.second;
            if (u16_entry_data.empty())
            {
                process_flag = false;
                continue;
            }
            WindowPopUp.update_position(cursor_pos);
            WindowPopUp.update_text(u16_entry_data);


            // Repooling normal loop sfml events
            WindowPopUp.poll_events();
            WindowPopUp.draw_elements();
            WindowPopUp.display();
            Sleep(250);
            WindowPopUp.reveal_window();
            process_flag = false;
            popup_active_flag = true;
        }
        
        else if (popup_active_flag)
        {
            WindowPopUp.poll_events();
            WindowPopUp.draw_elements();
            WindowPopUp.display();
        }
        else
        {
            WindowPopUp.hide_window();
        }

    }
    
    return 0;
}

