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
#include <memory>
#include <libsoup/soup.h>
#include <map>
#include <list>

#undef SPOON_DEBUG

class SpoonMessage;

/**
 *  a swallow cover for libsoup to bring it into the c++ world.
 *   I hope it is understood that synchronous requests are not nice.
 *   So we use the asynchronous kind.
 *  Using a shared_ptr approach to make messages
 *   to live long enough to be around when the
 *   response needs to be delivered (avoids new/delete)
 *    but still some pointer magic is required.
 */
class SpoonSession
{
public:
    SpoonSession(const Glib::ustring& user_agent);
    virtual ~SpoonSession();

    void send(std::shared_ptr<SpoonMessage> msg);
    static void callback(GObject *source, GAsyncResult *result, gpointer user_data);
    std::shared_ptr<SpoonMessage> get_remove_msg(SpoonMessage* ptr);
private:
    SoupSession *m_session;
    std::list<std::shared_ptr<SpoonMessage>> m_requests;
};

class SpoonMessage
{
public:
    SpoonMessage(const Glib::ustring& host, const Glib::ustring& path);
    SpoonMessage(const SpoonMessage& msg) = default;
    virtual ~SpoonMessage() = default;
    using type_signal_receive = sigc::signal<void(const Glib::ustring& error, int status, SpoonMessage* message)>;
    // on notification check in this order
    //  - error anything went seriously wrong with soup
    //  - status anything went wrong on the remote side usually status <> OK
    //  - message->get_bytes unlikely? this should be available even if it may be the wrong format
    //  - (message) if this is not set you get no notification -> check console messages
    type_signal_receive signal_receive();
    void addQuery(const Glib::ustring& name, const Glib::ustring& value);
    Glib::ustring get_url();
    void emit(const Glib::ustring& error, int status, const Glib::RefPtr<Glib::ByteArray>& data);
    Glib::RefPtr<Glib::ByteArray> get_bytes() {
        return m_gbytes;
    }
    void set_spoon_session(SpoonSession* spoonSession) {
        m_spoonSession = spoonSession;
    }
    SpoonSession* get_spoon_session() {
        return m_spoonSession;
    }
    static constexpr const int OK{SOUP_STATUS_OK};
    // Override this if you need a cancelable message (and use the return of SpoonMessage::send)
    GCancellable* get_cancelable();
protected:
    type_signal_receive m_signal_receive;
private:
    Glib::ustring m_host;
    Glib::ustring m_path;
    std::map<Glib::ustring, Glib::ustring> m_query;
    Glib::RefPtr<Glib::ByteArray> m_gbytes;
    SpoonSession* m_spoonSession;
};