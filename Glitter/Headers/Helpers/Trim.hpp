#include <boost/algorithm/string.hpp>
#include <string>
#include <iostream>
#include <algorithm>
#include <array>
#include <cstddef>

template <std::size_t N>
std::string cleanChars(const std::array<char, N>& arr) {
    // 1. Convert the std::array<char, N> to std::string.
    // We assume the Lua script content is null-terminated within the array.
    std::string temp(arr.data()); 
    
    // 2. Handle Non-Breaking Spaces (0xA0)
    const char NON_BREAKING_SPACE = 0xA0;
    std::replace(temp.begin(), temp.end(), NON_BREAKING_SPACE, ' ');

    // 3. Use Boost to trim all leading and trailing standard whitespace
    boost::trim(temp); 

    return temp;
}