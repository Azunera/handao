#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <optional>
#include <vector>

// Method to get the base directory of the .exe
std::string get_base_directory();

// Helper function to select with precision the unicode characters of a string
std::optional<std::string> return_unicode_at(const std::string &s, unsigned int pos);

// Converts utf8 into a wstring, the utf8 must be passed as a str, like std::string::c_str()
std::wstring wstring_from_utf8(const char* str);

// Get a vector with the length of every unicode character inside an utf8
std::vector<int> get_unicode_chac_len_vector_from_utf8(const char* str);

// Removes certain characters from outputted string, due to imprecision and confusions of Tesseract
void removeCharsInPlace(std::string& str);

// As the titel says
void GetDesktopResolution(int& horizontal, int& vertical);

#endif