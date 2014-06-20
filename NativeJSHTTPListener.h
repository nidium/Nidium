/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef nativejshttplistener_h__
#define nativejshttplistener_h__

#include "NativeJSExposer.h"
#include "NativeHTTPListener.h"

class NativeJSHTTPListener :    public NativeJSExposer<NativeJSHTTPListener>,
                                public NativeHTTPListener
{
public:
    NativeJSHTTPListener(uint16_t port, const char *ip = "0.0.0.0");
    virtual ~NativeJSHTTPListener();
    virtual void onClientDisconnect(NativeHTTPClientConnection *client);
    virtual void onData(NativeHTTPClientConnection *client, const char *buf, size_t len);
    virtual bool onEnd(NativeHTTPClientConnection *client);

    static void registerObject(JSContext *cx);
private:
};

#endif