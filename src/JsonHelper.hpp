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
#include <gio/gio.h>
#include <json-glib/json-glib.h>
#include <exception>
#include <vector>

class JsonException : public std::exception
{
public:
    JsonException(const Glib::ustring& msg);

    virtual const char * what() const noexcept;
private:
    Glib::ustring swhat;
};

// simplify usage of c-glib-json
class JsonHelper
{
public:
    JsonHelper();
    JsonHelper(const JsonHelper& orig) = delete;
    virtual ~JsonHelper();

    void load_from_file(const Glib::ustring& file);
    void load_data(const Glib::RefPtr<Glib::ByteArray>& data);
    JsonObject* get_root_object();
    JsonArray* get_root_array();
    JsonArray* get_array(JsonObject* obj, const Glib::ustring& name);
    JsonObject* get_array_object(JsonArray* obj, int idx);
    JsonObject* get_object(JsonObject* obj, const Glib::ustring& name);
    JsonArray* get_array_array(JsonArray* arr, int idx);
    std::vector<Glib::ustring> get_keys(JsonObject* obj);
private:
    JsonParser* m_parser;
    Glib::ustring m_file;
};


