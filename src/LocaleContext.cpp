/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <locale>
#include <string>

#include "LocaleContext.hpp"

LocaleContext::LocaleContext(int category)
: m_category{category}
{
    m_prev_loc = std::setlocale(m_category, nullptr);
}


LocaleContext::~LocaleContext()
{
    if (m_prev_loc) {
        std::setlocale(m_category, m_prev_loc);
    }
}

bool
LocaleContext::set(const char* lang)
{
    const char* locale;
    #ifdef _WIN64
    if (strncmp(lang, "en", 2) == 0) {
        locale = "English";
    }
    #else
        #ifdef _WIN32
        if (strncmp(lang, "en", 2 == 0) {
            locale = "English";
        }
        #else
        locale = lang;
        #endif
    #endif
    return std::setlocale(m_category, locale);
}

double
LocaleContext::parseDouble(const char* lang, const Glib::ustring& text)
{
    if (set(lang)) {
        double val = std::stod(text);
        if (val == 0.0) {
            std::cout << "LocaleContext::parseDouble unparsable ? \"" << text << "\"" << std::endl;
        }
        return val;
    }
    std::cout << "Failed to set " << lang << std::endl;
    return std::stoi(text);  // the integer part is most important
}