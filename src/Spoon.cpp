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

#include "Spoon.hpp"

SpoonSession::SpoonSession(const Glib::ustring& user_agent)
: m_session{nullptr}
{
    m_session = soup_session_new();
    if (!user_agent.empty()) {
        soup_session_set_user_agent(m_session, user_agent.c_str());
    }
}

SpoonSession::~SpoonSession()
{
    if (m_session != nullptr) {
        g_object_unref(m_session);
    }
}

void
SpoonSession::send(std::shared_ptr<SpoonMessage> spoonmsg)
{
    #ifdef SPOON_DEBUG
    std::cout << "send " << spoonmsg->get_url() << std::endl;
    #endif
    spoonmsg->set_spoon_session(this);
    m_requests.push_back(spoonmsg);
    SoupMessage* msg = soup_message_new(SOUP_METHOD_GET, spoonmsg->get_url().c_str());
    GCancellable* cancellable = spoonmsg->get_cancelable();
    soup_session_send_and_read_async(
           m_session, msg, G_PRIORITY_DEFAULT, cancellable, SpoonSession::callback, spoonmsg.get());
    g_object_unref(msg);
}

void
SpoonSession::callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    #ifdef SPOON_DEBUG
    std::cout << "SpoonSession::callback" << std::endl;
    #endif
    std::shared_ptr<SpoonMessage> spoonmsg;
    SpoonMessage* ptr = (SpoonMessage *)user_data;
    if (ptr) {
        SpoonSession* sess = ptr->get_spoon_session();
        if (sess) {
            spoonmsg = sess->get_remove_msg(ptr);
        }
    }
    GError *error = nullptr;
    SoupStatus status = SOUP_STATUS_NONE;
    Glib::RefPtr<Glib::ByteArray> data;
    GBytes *bytes = soup_session_send_and_read_finish(SOUP_SESSION(source), result, &error);
    if (error) {
        std::cout << "error session " << error->message << std::endl;
        if (spoonmsg) {
            spoonmsg->emit(error->message, status, data);
        }
        g_error_free(error);
    }
    else {
        if (bytes) {
            #ifdef SPOON_DEBUG
            std::cout << "SpoonSession::callback bytes " << bytes << std::endl;
            #endif
            GByteArray *bytearr = g_bytes_unref_to_array(bytes);
            data = Glib::wrap(bytearr);
        }
        if (spoonmsg) {
            SoupMessage* msg = soup_session_get_async_result_message(SOUP_SESSION(source), result);
            status = soup_message_get_status(msg);
            #ifdef SPOON_DEBUG
            std::cout << "Got " << status
                      << " url " << spoonmsg->get_url() << std::endl;
            #endif
            spoonmsg->emit({}, status, data);
        }
        else {
            std::cout << "Error session callback instance missing!" << std::endl;
        }
    }
}

std::shared_ptr<SpoonMessage>
SpoonSession::get_remove_msg(SpoonMessage* ptr)
{
    std::shared_ptr<SpoonMessage> spoonmsg;
    for (auto it = m_requests.begin(); it != m_requests.end(); it++) {
        auto shard = *it;
        if (shard.get() == ptr) {
            spoonmsg = shard;
            it = m_requests.erase(it);
            break;
        }
    }
    return spoonmsg;
}

SpoonMessage::SpoonMessage(const Glib::ustring& host, const Glib::ustring& path)
: m_host{host}
, m_path{path}
, m_query{}
, m_spoonSession{nullptr}
{
}

void
SpoonMessage::addQuery(const Glib::ustring& name, const Glib::ustring& value)
{
    m_query.insert(std::pair<Glib::ustring, Glib::ustring>(name, value));
}

Glib::ustring
SpoonMessage::get_url()
{
    Glib::ustring url = m_host;
    if (url.at(url.length()-1) != '/' &&
        (!m_path.empty() && m_path.at(0) != '/')) {
        url += '/';
    }
    url += m_path;
    if (!m_query.empty()) {
        Glib::ustring query("?");
        for (auto entry : m_query) {
            if (query.length() > 1) {
                query += "&";
            }
            Glib::ustring escaped_name = Glib::uri_escape_string(entry.first, {}, true);
            Glib::ustring escaped_value = Glib::uri_escape_string(entry.second, {","}, true);   // "," will not be understood if escaped in our case
            query += escaped_name + "=" + escaped_value;
        }
        url += query;
    }
    return url;
}

SpoonMessage::type_signal_receive
SpoonMessage::signal_receive()
{
    return m_signal_receive;
}

void
SpoonMessage::emit(const Glib::ustring& error, int status, const Glib::RefPtr<Glib::ByteArray>& data)
{
    m_gbytes = data;
    m_signal_receive.emit(error, status, this);
}

GCancellable*
SpoonMessage::get_cancelable()
{
    return nullptr;
}
