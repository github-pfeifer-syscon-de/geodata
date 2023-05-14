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

#pragma once

#include <glibmm.h>

// pack the locale set/restore into a type
class LocaleContext
{
public:
    LocaleContext(int category);
    virtual ~LocaleContext();

    bool set(const char* lang);
    // parse double for a locale use integer as fallback
    double parseDouble(const char* lang, const Glib::ustring& text);

    static constexpr auto en_US = "en_US.utf8";
private:
    int m_category;
    const char *m_prev_loc{nullptr};
};

