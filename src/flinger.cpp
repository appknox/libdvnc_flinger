/*
     droid vnc server - Android VNC server
     Copyright (C) 2009 Jose Pereira <onaips@gmail.com>

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Lesser General Public
     License as published by the Free Software Foundation; either
     version 3 of the License, or (at your option) any later version.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Lesser General Public License for more details.

     You should have received a copy of the GNU Lesser General Public
     License along with this library; if not, write to the Free Software
     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
     */

#include <fcntl.h>
#include <binder/ProcessState.h> //ThreadPool

#include "ScreenFrame.h"
#include "flinger.h"
#include "screenFormat.h"

#define DEFAULT_DISPLAY_ID 0

android::sp<ScreenFrame> sf;
int32_t displayId = DEFAULT_DISPLAY_ID;
size_t Bpp = 32;

extern "C" screenFormat getscreenformat_flinger()
{
    screenFormat format;
    android::PixelFormat pFormat;
    pFormat = sf->getFormat();
    Bpp = sf->bitsPerPixel();
    L("Bpp set to %zu\n", Bpp);

    format.bitsPerPixel     = sf->bitsPerPixel();
    format.bytesPerPixel    = sf->bytesPerPixel();
    format.width            = sf->getWidth();
    format.height           = sf->getHeight();
    format.size             = sf->getSize();
    format.rowStride        = sf->getStride();

    format.redShift     = 0;
    format.redMax       = 0;
    format.greenShift   = 0;
    format.greenMax     = 0;
    format.blueShift    = 0;
    format.blueMax      = 0;
    format.alphaShift   = 0;
    format.alphaMax     = 0;
    return format;
}

extern "C" int init_flinger()
{
    int err;
    android::ProcessState::self()->startThreadPool();
    sf = new ScreenFrame();
    err = sf->check();
    if(err != 0) {
        return err;
    }
    err = sf->initialize(displayId);
    if(err != 0 ) {
        return err;
    }
    sf->waitForFrame();
    return sf->updateFrame();
}

extern "C" unsigned int *checkfb_flinger()
{
    sf->updateFrame();
    return (unsigned int*) sf->getPixels();
}

extern "C" unsigned int *readfb_flinger()
{
    sf->updateFrame();
    return (unsigned int*) sf->getPixels();
}

extern "C" void close_flinger()
{
}
