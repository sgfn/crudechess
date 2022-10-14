#pragma once

#include <algorithm>
#include <string>

namespace Strfuns {

static inline void lstrip(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void rstrip(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void strip(std::string &s) { lstrip(s); rstrip(s); }

static inline std::string lstrip_copy(std::string s) { lstrip(s); return s; }
static inline std::string rstrip_copy(std::string s) { rstrip(s); return s; }
static inline std::string strip_copy(std::string s) { strip(s); return s; }

} // namespace Strfuns
