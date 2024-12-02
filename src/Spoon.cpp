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
#include <Log.hpp>
#include <StringUtils.hpp>

#include "Spoon.hpp"

SpoonSession::SpoonSession(const Glib::ustring& user_agent)
: m_session{soup_session_new()}
{
    #ifdef SPOON_DEBUG_INTERNAL
    SoupLogger* log = soup_logger_new(SOUP_LOGGER_LOG_MINIMAL);
    soup_session_add_feature(m_session, SOUP_SESSION_FEATURE(log));
    #endif
    if (!user_agent.empty()) {
        soup_session_set_user_agent(m_session, user_agent.c_str());
    }
}

SpoonSession::~SpoonSession()
{
    if (m_session) {
        soup_session_abort(m_session); // any dangling request is bad!
        g_object_unref(m_session);
    }
}

void
SpoonSession::send(std::shared_ptr<SpoonMessage> spoonmsg)
{
    spoonmsg->set_spoon_session(this);

    m_requests.push_back(spoonmsg);
    spoonmsg->send();
}

std::shared_ptr<SpoonMessage>
SpoonSession::get_remove_msg(SpoonMessage* ptr)
{
    std::shared_ptr<SpoonMessage> spoonmsg;
    for (auto it = m_requests.begin(); it != m_requests.end();) {
        auto shard = *it;
        if (shard.get() == ptr) {
            spoonmsg = shard;
            it = m_requests.erase(it);
            break;
        }
        else {
            ++it;
        }
    }
    return spoonmsg;
}

SpoonMessage::SpoonMessage(const Glib::ustring& host, const Glib::ustring& path)
: m_host{host}
, m_path{path}
, m_query{}
{
}

void
SpoonMessage::addQuery(const Glib::ustring& name, const Glib::ustring& value)
{
    m_query.insert(std::make_pair(name, value));
}

Glib::ustring
SpoonMessage::get_url()
{
    Glib::ustring url{m_host};
    url.reserve(256);
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
            Glib::ustring escaped_value = Glib::uri_escape_string(entry.second, {}, true);   // "," will not be understood if escaped in our case
            query += escaped_name + "=" + escaped_value;
        }
        url += query;
    }
    return url;
}



GCancellable*
SpoonMessage::get_cancelable()
{
    return nullptr;
}

// return most likely Http codes
const char*
SpoonMessage::decodeStatus(int status)
{
    switch (status) {
    case SOUP_STATUS_NONE:
        return "No status available. (Eg, the message has not been sent yet)";
    case SOUP_STATUS_CONTINUE:
        return "Continue (HTTP)";
    case SOUP_STATUS_SWITCHING_PROTOCOLS:
        return "Switching Protocols (HTTP)";
    case SOUP_STATUS_PROCESSING:
        return "Processing (WebDAV)";
    case SOUP_STATUS_OK:
        return "200 Success (HTTP)";
    case SOUP_STATUS_UNAUTHORIZED:
        return "Unauthorized (HTTP)";
    case SOUP_STATUS_PAYMENT_REQUIRED:
        return "Payment Required (HTTP)";
    case SOUP_STATUS_FORBIDDEN:
        return "Forbidden (HTTP)";
    case SOUP_STATUS_NOT_FOUND:
        return "Not Found (HTTP)";
    case SOUP_STATUS_METHOD_NOT_ALLOWED:
        return "Method Not Allowed (HTTP)";
    case SOUP_STATUS_NOT_ACCEPTABLE:
        return "Not Acceptable (HTTP)";
    case SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED:
        return "Proxy Authentication Required (HTTP)";
    case SOUP_STATUS_REQUEST_TIMEOUT:
        return "Request Timeout (HTTP)";
    case SOUP_STATUS_CONFLICT:
        return "Conflict (HTTP)";
    case SOUP_STATUS_GONE:
        return "Gone (HTTP)";
    case SOUP_STATUS_LENGTH_REQUIRED:
        return "Length Required (HTTP)";
    case SOUP_STATUS_PRECONDITION_FAILED:
        return "Precondition Failed (HTTP)";
    case SOUP_STATUS_REQUEST_ENTITY_TOO_LARGE:
        return "Request Entity Too Large (HTTP)";
    case SOUP_STATUS_REQUEST_URI_TOO_LONG:
        return "Request-URI Too Long (HTTP)";
    case SOUP_STATUS_UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported Media Type (HTTP)";
    case SOUP_STATUS_REQUESTED_RANGE_NOT_SATISFIABLE:
        return "Requested Range Not Satisfiable (HTTP)";
    case SOUP_STATUS_EXPECTATION_FAILED:
        return "Expectation Failed (HTTP)";
    case SOUP_STATUS_MISDIRECTED_REQUEST:
        return "Misdirected Request.";
    case SOUP_STATUS_UNPROCESSABLE_ENTITY:
        return "Unprocessable Entity (WebDAV)";
    case SOUP_STATUS_LOCKED:
        return "Locked (WebDAV)";
    case SOUP_STATUS_FAILED_DEPENDENCY:
        return "Failed Dependency (WebDAV)";
    case SOUP_STATUS_INTERNAL_SERVER_ERROR:
        return "Internal Server Error (HTTP)";
    case SOUP_STATUS_NOT_IMPLEMENTED:
        return "Not Implemented (HTTP)";
    case SOUP_STATUS_BAD_GATEWAY:
        return "Bad Gateway (HTTP)";
    case SOUP_STATUS_SERVICE_UNAVAILABLE:
        return "Service Unavailable (HTTP)";
    case SOUP_STATUS_GATEWAY_TIMEOUT:
        return "Gateway Timeout (HTTP)";
    default:
        return "Unknown";
    }
}

SpoonMessageDirect::SpoonMessageDirect(const Glib::ustring& host, const Glib::ustring& path)
: SpoonMessage(host, path)
{
}

SpoonMessageDirect::type_signal_receive
SpoonMessageDirect::signal_receive()
{
    return m_signal_receive;
}

void
SpoonMessageDirect::callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    psc::log::Log::logAdd(psc::log::Level::Debug, "SpoonMessageDirect::callback");
    std::shared_ptr<SpoonMessageDirect> spoonmsg;
    SpoonMessageDirect* ptr = (SpoonMessageDirect *)user_data;
    if (ptr) {
        SpoonSession* sess = ptr->get_spoon_session();
        if (sess) {
            spoonmsg = std::dynamic_pointer_cast<SpoonMessageDirect>(sess->get_remove_msg(ptr));
            if (!spoonmsg) {
                psc::log::Log::logAdd(psc::log::Level::Error, "SpoonSession::callback no pointer/wrong type");
            }
        }
        else {
            psc::log::Log::logAdd(psc::log::Level::Error, "SpoonSession::callback no session on message");
        }
    }
    else {
        psc::log::Log::logAdd(psc::log::Level::Error, "SpoonSession::callback no ptr on message");
    }
    GError *error = nullptr;
    SoupStatus status = SOUP_STATUS_NONE;
    Glib::RefPtr<Glib::ByteArray> data;
    GBytes* bytes = soup_session_send_and_read_finish(SOUP_SESSION(source), result, &error);
    if (error) {
        psc::log::Log::logAdd(psc::log::Level::Error, [&] {
            return std::format("error session {}", error->message);
        });
        if (spoonmsg) {
            spoonmsg->emit(error->message, status, data);
        }
        g_error_free(error);
    }
    else {
        if (bytes) {
            psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
                return std::format("callback bytes {}", static_cast<void*>(bytes));
            });
            GByteArray *bytearr = g_bytes_unref_to_array(bytes);
            data = Glib::wrap(bytearr);
        }
        if (spoonmsg) {
            SoupMessage* msg = soup_session_get_async_result_message(SOUP_SESSION(source), result);
            status = soup_message_get_status(msg);
            psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
                return std::format("Got {} url {}", static_cast<int>(status), spoonmsg->get_url());
            });
            spoonmsg->emit({}, status, data);
        }
    }
}

void
SpoonMessageDirect::send()
{
    SoupMessage* msg = soup_message_new(get_method(), get_url().c_str());
    GCancellable* cancellable = get_cancelable();
    psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
        return std::format("send {} url {} msg {}", get_method(), get_url(), static_cast<void*>(msg));
    });
    soup_session_send_and_read_async(
           m_spoonSession->get_session(), msg, G_PRIORITY_DEFAULT, cancellable, SpoonMessageDirect::callback, this);
    g_object_unref(msg);
}


void
SpoonMessageDirect::emit(const Glib::ustring& error, int status, const Glib::RefPtr<Glib::ByteArray>& data)
{
    m_gbytes = data;
    m_signal_receive.emit(error, status, this);
}

SpoonMessageStream::SpoonMessageStream(const Glib::ustring& host, const Glib::ustring& path)
: SpoonMessage(host, path)
{
}

SpoonMessageStream::type_signal_receive
SpoonMessageStream::signal_receive()
{
    return m_signal_receive;
}


void
SpoonMessageStream::callback(GObject *source, GAsyncResult *result, gpointer user_data)
{
    psc::log::Log::logAdd(psc::log::Level::Debug, "SpoonMessageStream::callback");
    std::shared_ptr<SpoonMessageStream> spoonmsg;
    SpoonMessageStream* ptr = (SpoonMessageStream *)user_data;
    if (ptr) {
        SpoonSession* sess = ptr->get_spoon_session();
        if (sess) {
            spoonmsg = std::dynamic_pointer_cast<SpoonMessageStream>(sess->get_remove_msg(ptr));
            if (!spoonmsg) {
                psc::log::Log::logAdd(psc::log::Level::Error, "SpoonMessageStream::callback no pointer/wrong type");
            }
        }
        else {
            psc::log::Log::logAdd(psc::log::Level::Error, "SpoonMessageStream::callback no session on message");
        }
    }
    else {
        psc::log::Log::logAdd(psc::log::Level::Error, "SpoonMessageStream::callback no ptr on message");
    }
    GError *error = nullptr;
    SoupStatus status = SOUP_STATUS_NONE;
    GInputStream* stream = soup_session_send_finish(SOUP_SESSION(source), result, &error);
    if (error) {
        psc::log::Log::logAdd(psc::log::Level::Error, [&] {
            return std::format("error session {}", error->message);
        });
        if (spoonmsg) {
            spoonmsg->emit(error->message, status, stream);
        }
        g_error_free(error);
    }
    else {
        if (spoonmsg) {
            SoupMessage* msg = soup_session_get_async_result_message(SOUP_SESSION(source), result);
            status = soup_message_get_status(msg);
            psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
                return std::format("Got {} url {} stream {}", static_cast<int>(status), spoonmsg->get_url(), static_cast<void*>(stream));
            });
            spoonmsg->emit({}, status, stream);
        }
    }
    if (stream) {
        g_object_unref(stream);
    }
}

void
SpoonMessageStream::send()
{
    SoupMessage* msg = soup_message_new(get_method(), get_url().c_str());
    psc::log::Log::logAdd(psc::log::Level::Debug, [&] {
        return std::format("send {} url {} msg {}", get_method(), get_url(), static_cast<void*>(msg));
    });
    GCancellable* cancellable = get_cancelable();
    soup_session_send_async(
           m_spoonSession->get_session(), msg, G_PRIORITY_LOW, cancellable, SpoonMessageStream::callback, this);
    g_object_unref(msg);
}


void
SpoonMessageStream::emit(const Glib::ustring& error, int status, GInputStream* stream)
{
    m_stream = stream;
    m_signal_receive.emit(error, status, this);
}
