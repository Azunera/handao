#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstddef>
#include <filesystem>
#include "utils.hpp"

namespace hanzi_faxian {
    
// The entrie sthat will be isnide the dictionary
struct Entry {
    std::string simplified_character;
    std::string traditional_character;
    std::string pinyin;
    std::string definitions;
};

class Dictionary 
{
    protected:
        std::unordered_map<std::string, std::vector<const Entry*>> dictionary;
        // This has to modify the dictionary property
        virtual void parse_dictionary_from_file(const char* dict_address) = 0;
    public:
        Dictionary() = default;

};

class CedictDictionary : public Dictionary 
{
    private:
        void parse_dictionary_from_file(const char* dict_address) override;
    
    public:
        CedictDictionary(const char* dict_address)
        {
            parse_dictionary_from_file(dict_address);
        }

        // Reuturns the number of the entries of the dictionary
        std::size_t get_size() const;
        // Gets an specific entry
        std::optional<std::vector<const Entry*>> lookup(const std::string& hanzi) const;
        // Given a string of hanzi of any length, check all entries available since the first character accumulating all the way to the last. permitting the sub-entries a greater word may have. 
        std::optional<std::vector<const Entry*>> lookup_composed(std::string lookup_content) const;

};

}

#endif