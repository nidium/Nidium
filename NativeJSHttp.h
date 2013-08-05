/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

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

#ifndef nativejshttp_h__
#define nativejshttp_h__

#include "NativeJSExposer.h"
#include <native_netlib.h>
#include "ape_array.h"
#include "NativeHTTP.h"

class NativeJSHttp : public NativeJSExposer<NativeJSHttp>, public NativeHTTPDelegate
{
  public:
    static void registerObject(JSContext *cx);
    NativeJSHttp();
    virtual ~NativeJSHttp();

    jsval request;
    NativeHTTP *refHttp;
    JSObject *jsobj;

    void onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onProgress(size_t offset, size_t len,
        NativeHTTP::HTTPData *h, NativeHTTP::DataType);
    void onError(NativeHTTP::HTTPError err);
    void onHeader(){};
};


#endif
