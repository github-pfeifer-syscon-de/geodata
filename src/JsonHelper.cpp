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


#include "JsonHelper.hpp"

JsonException::JsonException(const Glib::ustring& error)
: std::exception()
, swhat(error)
{

}

const char *
JsonException::what() const noexcept
{
    return swhat.c_str();
}

JsonHelper::JsonHelper()
{
    m_parser = json_parser_new();
}

JsonHelper::~JsonHelper()
{
    if (m_parser) {
        g_object_unref(m_parser);
    }
}

void
JsonHelper::load_from_file(const Glib::ustring& file)
{
    GError *error = nullptr;
    m_file = file;
    json_parser_load_from_file(m_parser, (const gchar*)file.c_str(), &error);
    if (error) {
        auto msg = Glib::ustring::sprintf("Unable to parse json file %s %s", file, error->message);
        g_error_free(error);
        throw JsonException(msg);
    }
}

void
JsonHelper::load_data(const Glib::RefPtr<Glib::ByteArray>& data)
{
    GError *error = nullptr;
    m_file = "data";
    json_parser_load_from_data(m_parser, (const gchar*)data->get_data(), data->size(), &error);
    if (error) {
        auto msg = Glib::ustring::sprintf("Unable to parse data len %d %s", data->size(), error->message);
        g_error_free(error);
        throw JsonException(msg);
    }
}

JsonObject*
JsonHelper::get_root_object()
{
    JsonNode* root = json_parser_get_root(m_parser);
    JsonObject* rootObj = json_node_get_object(root);
    if (!rootObj) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a object as root", m_file);
        throw JsonException(msg);
    }
    return rootObj;
}

JsonArray*
JsonHelper::get_root_array()
{
    JsonNode* root = json_parser_get_root(m_parser);
    JsonArray* arr = json_node_get_array(root);
    if (!arr) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a array as root", m_file);
        throw JsonException(msg);
    }
    return arr;
}

JsonArray*
JsonHelper::get_array(JsonObject* obj, const Glib::ustring& name)
{
    JsonArray* arr = json_object_get_array_member(obj, name.c_str());
    if (!arr) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a array as expected ", m_file);
        throw JsonException(msg);
    }
    return arr;
}

JsonObject*
JsonHelper::get_array_object(JsonArray* arr, int idx)
{
    JsonObject* obj = json_array_get_object_element(arr, idx);
    if (!obj) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a object at index %d ", m_file, idx);
        throw JsonException(msg);
    }
    return obj;
}

JsonObject*
JsonHelper::get_object(JsonObject* obj, const Glib::ustring& name)
{
    JsonObject* member = json_object_get_object_member(obj, name.c_str());
    if (!member) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a object form name %s ", m_file, name);
        throw JsonException(msg);
    }
    return member;
}

JsonArray*
JsonHelper::get_array_array(JsonArray* arr, int idx)
{
    JsonArray* arri = json_array_get_array_element(arr, idx);
    if (!arri) {
        auto msg = Glib::ustring::sprintf("The json file %s doesn't contain a array at index %d ", m_file, idx);
        throw JsonException(msg);
    }
    return arri;
}


std::vector<Glib::ustring>
JsonHelper::get_keys(JsonObject* obj) {
    std::vector<Glib::ustring> list;
    GList* values = json_object_get_members(obj);
    for (GList* elem = values; elem; elem = elem->next) {
        const gchar* item = (const gchar*)elem->data;
        list.push_back(Glib::ustring(item));
    }
    g_list_free(values);
    return list;
}