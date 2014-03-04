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

#include "NativeHTTPStream.h"
#include "NativeUtils.h"
#include "NativeJS.h"

NativeHTTPStream::NativeHTTPStream(const char *location) : 
    NativeBaseStream(location)
{
    NativeHTTPRequest *req = new NativeHTTPRequest(location);
    m_Http = new NativeHTTP(req, NativeJS::getNet());
}

NativeHTTPStream::~NativeHTTPStream()
{
    delete m_Http;
}

void NativeHTTPStream::onStart(size_t packets, size_t seek)
{

}

const unsigned char *NativeHTTPStream::onGetNextPacket(size_t *len, int *err)
{
 
}

void NativeHTTPStream::stop()
{
    /*
        Do nothing
    */
}

void NativeHTTPStream::getContent()
{
 
}

size_t NativeHTTPStream::getFileSize()
{

}

void NativeHTTPStream::seek(size_t pos)
{

}

//////////////////
//////////////////
//////////////////


void NativeHTTPStream::onRequest(NativeHTTP::HTTPData *h, NativeHTTP::DataType)
{

}

void NativeHTTPStream::onProgress(size_t offset, size_t len, NativeHTTP::HTTPData *h,
    NativeHTTP::DataType)
{

}

void NativeHTTPStream::onError(NativeHTTP::HTTPError err)
{

}

void NativeHTTPStream::onHeader()
{

}