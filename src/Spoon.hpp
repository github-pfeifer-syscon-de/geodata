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

#include <memory>
#include <map>
#include <list>
#include <glibmm.h>
#include <libsoup/soup.h>

#undef SPOON_DEBUG_INTERNAL

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
    std::shared_ptr<SpoonMessage> get_remove_msg(SpoonMessage* ptr);
    SoupSession *get_session() {
        return m_session;
    }
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
    void addQuery(const Glib::ustring& name, const Glib::ustring& value);
    Glib::ustring get_url();
    void set_spoon_session(SpoonSession* spoonSession) {
        m_spoonSession = spoonSession;
    }
    SpoonSession* get_spoon_session() {
        return m_spoonSession;
    }
    const char* get_method() {
        return SOUP_METHOD_GET;
    }

    static constexpr const int OK{SOUP_STATUS_OK};
    // Override this if you need a cancelable message (and use the return of SpoonMessage::send)
    GCancellable* get_cancelable();
    virtual void send() = 0;
protected:
    Glib::ustring m_host;
    Glib::ustring m_path;
    SpoonSession* m_spoonSession{nullptr};
private:
    std::map<Glib::ustring, Glib::ustring> m_query;
};

// a message for which the content is passed in memory
class SpoonMessageDirect
: public SpoonMessage
{
public:
    SpoonMessageDirect(const Glib::ustring& host, const Glib::ustring& path);
    SpoonMessageDirect(const SpoonMessageDirect& msg) = default;
    virtual ~SpoonMessageDirect() = default;
    using type_signal_receive = sigc::signal<void(const Glib::ustring& error, int status, SpoonMessageDirect* message)>;
    // on notification check in this order
    //  - error anything went seriously wrong with soup
    //  - status anything went wrong on the remote side usually status <> OK
    //  - message->get_bytes unlikely? this should be available even if it may be the wrong format
    //  - (message) if this is not set you get no notification -> check console messages
    type_signal_receive signal_receive();
    void send() override;
    static void callback(GObject *source, GAsyncResult *result, gpointer user_data);
    void emit(const Glib::ustring& error, int status, const Glib::RefPtr<Glib::ByteArray>& data);
    Glib::RefPtr<Glib::ByteArray> get_bytes() {
        return m_gbytes;
    }
protected:
    type_signal_receive m_signal_receive;
private:
    Glib::RefPtr<Glib::ByteArray> m_gbytes;
};

// a message that content is passed as stream
//   reduced memory usage but the reading will be in foreground...
class SpoonMessageStream
: public SpoonMessage
{
public:
    SpoonMessageStream(const Glib::ustring& host, const Glib::ustring& path);
    SpoonMessageStream(const SpoonMessageStream& msg) = default;
    virtual ~SpoonMessageStream() = default;
    using type_signal_receive = sigc::signal<void(const Glib::ustring& error, int status, SpoonMessageStream* message)>;
    // on notification check in this order
    //  - error anything went seriously wrong with soup
    //  - status anything went wrong on the remote side usually status <> OK
    //  - message->get_bytes unlikely? this should be available even if it may be the wrong format
    //  - (message) if this is not set you get no notification -> check console messages
    type_signal_receive signal_receive();
    void send() override;
    static void callback(GObject *source, GAsyncResult *result, gpointer user_data);
    void emit(const Glib::ustring& error, int status, GInputStream* stream);
    GInputStream* get_stream()
    {
        return m_stream;
    }
protected:
    type_signal_receive m_signal_receive;
private:
    GInputStream* m_stream;
};