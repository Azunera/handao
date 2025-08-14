
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <cstddef>
#include <filesystem>
#include <optional>
#include "utils.hpp"
#include "Dictionary.hpp"

namespace hanzi_faxian {
    
std::size_t CedictDictionary::get_size() const
{
    return this->dictionary.size();
}

void CedictDictionary::parse_dictionary_from_file(const char* dict_address)

{
    std::string line;
    std::ifstream fileDict(dict_address);

    while (getline (fileDict, line)) {
        if (line[0] == '#') {
            continue;
        }
        std::string trad_hanzi, simp_hanzi, pinyin, a;
        std::vector<std::string> translations;

        std::istringstream iss(line);
        iss >> trad_hanzi;
        iss >> simp_hanzi;

        // Retrieving pinyin
        size_t start = line.find('[');
        size_t end = line.find(']', start);

        pinyin = line.substr(start + 1, end - start - 1);

        // Finding  definitions
        std::vector<size_t> index_vector;
        size_t pos = 0;

        // Finding every '/' and assigning it to index_vetor
        while (pos < line.length()) {
            int new_slash_index = line.find("/", pos);
            if (new_slash_index == line.npos) {
                break;
            }
            index_vector.push_back(new_slash_index);
            pos = new_slash_index + 1;
        }
        // Knowing the position of the / i retrieve the definitions
        std::vector<std::string> definitions;
        for (int i = 0; i < index_vector.size() - 1; i++) {
            size_t istart = index_vector.at(i);
            size_t iend = index_vector.at(i+1);
            definitions.push_back(line.substr(istart+1, iend - istart -1));
        }        
        // You optionally may want them have seperated, but for the current purposes all the sub-definitions of an specific
        // character-sound entry must be in a single string.
        std::string compressed_definitions = "";

        for (std::string definition : definitions) 
        {   
            if (compressed_definitions.size() != 0)
            {
                compressed_definitions += "; ";
            }
            compressed_definitions += definition;
        }

        const Entry* new_entry = new Entry{simp_hanzi, trad_hanzi, pinyin, compressed_definitions};
        if (simp_hanzi == trad_hanzi) {
            dictionary[simp_hanzi].push_back(new_entry);
        } else {
            dictionary[simp_hanzi].push_back(new_entry);
            dictionary[trad_hanzi].push_back(new_entry);
        }
    }

    fileDict.close(); 
}

// Gets an specific entry
std::optional<std::vector<const Entry*>> CedictDictionary::lookup(const std::string& word) const
{
    std::vector<const Entry*> entries_found;

    if (this->dictionary.find(word) != this->dictionary.end()) 
    {
        entries_found = this->dictionary.at(word);
    }
    else
    {
        return std::nullopt;
    }
    return entries_found;
}

// Given a string of hanzi of any length, check all entries available since the first character accumulating all the way to the last. permitting the sub-entries a greater word may have. 
std::optional<std::vector<const Entry*>> CedictDictionary::lookup_composed(std::string lookup_content) const
{
    // All the entries will be scattered by order of being found.
    std::vector<const Entry*> found_entries;

    // Vector necessary for having the number of character sneed to remove from the end of the string.
    // Thinking of renaming to avoid confusion. (!!!)
    std::vector<int> unicode_len_vector = get_unicode_chac_len_vector_from_utf8(lookup_content.c_str());

    while (lookup_content.size() > 0)
    {
        if (auto new_entry = lookup(lookup_content))
        {
            for (const auto* entry : new_entry.value()) 
            {
                found_entries.push_back(entry);
            }
        }
        // Get how many characters are contained in the last unicode character as utf-8 then remove it from the utf-8 chacs.
        int last_unicode_char_len = unicode_len_vector.back();
        unicode_len_vector.pop_back();

        for (int i = 0; i < last_unicode_char_len; i++)
        {
            lookup_content.pop_back();
        }
    }

    if (found_entries.empty())
    {
        return std::nullopt;
    }
    
    return found_entries;
};

}