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

#include "flinger.h"
#include "screenFormat.h"

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <binder/IMemory.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <ui/DisplayInfo.h>
#include <ui/PixelFormat.h>

using namespace android;

static uint32_t DEFAULT_DISPLAY_ID = ISurfaceComposer::eDisplayIdMain;
static const int COMPONENT_YUV = 0xFF;
int32_t displayId = DEFAULT_DISPLAY_ID;
size_t Bpp = 32;
sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(displayId);
ScreenshotClient *screenshotClient=NULL;

struct PixelFormatInformation {
    enum {
        INDEX_ALPHA   = 0,
        INDEX_RED     = 1,
        INDEX_GREEN   = 2,
        INDEX_BLUE    = 3
    };

    enum { // components
        ALPHA   = 1,
        RGB     = 2,
        RGBA    = 3,
        L       = 4,
        LA      = 5,
        OTHER   = 0xFF
    };

    struct szinfo {
        uint8_t h;
        uint8_t l;
    };

    inline PixelFormatInformation() : version(sizeof(PixelFormatInformation)) { }
    size_t getScanlineSize(unsigned int width) const;
    size_t getSize(size_t ci) const {
        return (ci <= 3) ? (cinfo[ci].h - cinfo[ci].l) : 0;
    }
    size_t      version;
    PixelFormat format;
    size_t      bytesPerPixel;
    size_t      bitsPerPixel;
    union {
        szinfo      cinfo[4];
        struct {
            uint8_t     h_alpha;
            uint8_t     l_alpha;
            uint8_t     h_red;
            uint8_t     l_red;
            uint8_t     h_green;
            uint8_t     l_green;
            uint8_t     h_blue;
            uint8_t     l_blue;
        };
    };
    uint8_t     components;
    uint8_t     reserved0[3];
    uint32_t    reserved1;
};

struct Info {
    size_t      size;
    size_t      bitsPerPixel;
    struct {
        uint8_t     ah;
        uint8_t     al;
        uint8_t     rh;
        uint8_t     rl;
        uint8_t     gh;
        uint8_t     gl;
        uint8_t     bh;
        uint8_t     bl;
    };
    uint8_t     components;
};

static Info const sPixelFormatInfos[] = {
         { 0,  0, { 0, 0,   0, 0,   0, 0,   0, 0 }, 0 },
         { 4, 32, {32,24,   8, 0,  16, 8,  24,16 }, PixelFormatInformation::RGBA },
         { 4, 24, { 0, 0,   8, 0,  16, 8,  24,16 }, PixelFormatInformation::RGB  },
         { 3, 24, { 0, 0,   8, 0,  16, 8,  24,16 }, PixelFormatInformation::RGB  },
         { 2, 16, { 0, 0,  16,11,  11, 5,   5, 0 }, PixelFormatInformation::RGB  },
         { 4, 32, {32,24,  24,16,  16, 8,   8, 0 }, PixelFormatInformation::RGBA },
         { 2, 16, { 1, 0,  16,11,  11, 6,   6, 1 }, PixelFormatInformation::RGBA },
         { 2, 16, { 4, 0,  16,12,  12, 8,   8, 4 }, PixelFormatInformation::RGBA },
         { 1,  8, { 8, 0,   0, 0,   0, 0,   0, 0 }, PixelFormatInformation::ALPHA},
         { 1,  8, { 0, 0,   8, 0,   8, 0,   8, 0 }, PixelFormatInformation::L    },
         { 2, 16, {16, 8,   8, 0,   8, 0,   8, 0 }, PixelFormatInformation::LA   },
         { 1,  8, { 0, 0,   8, 5,   5, 2,   2, 0 }, PixelFormatInformation::RGB  },
};

static const Info* gGetPixelFormatTable(size_t* numEntries) {
    if (numEntries) {
        *numEntries = sizeof(sPixelFormatInfos)/sizeof(Info);
    }
    return sPixelFormatInfos;
}

static const uint32_t ORIENTATION_MAP[] = {
    ISurfaceComposer::eRotateNone, // 0 == DISPLAY_ORIENTATION_0
    ISurfaceComposer::eRotate270, // 1 == DISPLAY_ORIENTATION_90
    ISurfaceComposer::eRotate180, // 2 == DISPLAY_ORIENTATION_180
    ISurfaceComposer::eRotate90, // 3 == DISPLAY_ORIENTATION_270
};

status_t getPixelFormatInformation(PixelFormat format, PixelFormatInformation* info)
{
    if (format <= 0)
        return BAD_VALUE;

    if (info->version != sizeof(PixelFormatInformation))
        return INVALID_OPERATION;

    // YUV format from the HAL are handled here
    switch (format) {
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        info->bitsPerPixel = 16;
        goto done;
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
    case HAL_PIXEL_FORMAT_YV12:
        info->bitsPerPixel = 12;
     done:
        info->format = format;
        info->components = COMPONENT_YUV;
        info->bytesPerPixel = 1;
        info->h_alpha = 0;
        info->l_alpha = 0;
        info->h_red = info->h_green = info->h_blue = 8;
        info->l_red = info->l_green = info->l_blue = 0;
        return NO_ERROR;
    }

    size_t numEntries;
    const Info *i = gGetPixelFormatTable(&numEntries) + format;
    bool valid = uint32_t(format) < numEntries;
    if (!valid) {
        return BAD_INDEX;
    }

    info->format = format;
    info->bytesPerPixel = i->size;
    info->bitsPerPixel  = i->bitsPerPixel;
    info->h_alpha       = i->ah;
    info->l_alpha       = i->al;
    info->h_red         = i->rh;
    info->l_red         = i->rl;
    info->h_green       = i->gh;
    info->l_green       = i->gl;
    info->h_blue        = i->bh;
    info->l_blue        = i->bl;
    info->components    = i->components;

    return NO_ERROR;
}

extern "C" screenFormat getscreenformat_flinger()
{
    screenFormat format;
    void* base = NULL;
    Vector<DisplayInfo> configs;
    SurfaceComposerClient::getDisplayConfigs(display, &configs);
    int activeConfig = SurfaceComposerClient::getActiveConfig(display);
    if (static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n",
                activeConfig, configs.size());
        return format;
    }
    uint8_t displayOrientation = configs[activeConfig].orientation;
    uint32_t captureOrientation = ORIENTATION_MAP[displayOrientation];
    sp<GraphicBuffer> outBuffer;
    status_t result = ScreenshotClient::capture(display, Rect(), 0 /* reqWidth */,
        0 /* reqHeight */, INT32_MIN, INT32_MAX, /* all layers */ false, captureOrientation,
        &outBuffer);
    if (result != NO_ERROR) {
        return format;
    }
    result = outBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &base);
    if (base == NULL) {
        return format;
    }
    PixelFormat f;
    f = outBuffer->getPixelFormat();

    PixelFormatInformation pf;
    getPixelFormatInformation(f,&pf);

    Bpp = bytesPerPixel(f);
    L("Bpp set to %zu\n", Bpp);

    format.bitsPerPixel = bitsPerPixel(f);
    format.width        = outBuffer->getWidth();
    format.height       = outBuffer->getHeight();
    format.size         = format.bitsPerPixel*format.width*format.height/CHAR_BIT;
    format.redShift     = pf.l_red;
    format.redMax       = pf.h_red - pf.l_red;
    format.greenShift   = pf.l_green;
    format.greenMax     = pf.h_green - pf.l_green;
    format.blueShift    = pf.l_blue;
    format.blueMax      = pf.h_blue - pf.l_blue;
    format.alphaShift   = pf.l_alpha;
    format.alphaMax     = pf.h_alpha-pf.l_alpha;

    return format;
}


extern "C" int init_flinger()
{
    L("--Initializing access method--\n");
    void* base = NULL;
    Vector<DisplayInfo> configs;
    SurfaceComposerClient::getDisplayConfigs(display, &configs);
    int activeConfig = SurfaceComposerClient::getActiveConfig(display);
    if (static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n",
                activeConfig, configs.size());
        return -1;
    }
    uint8_t displayOrientation = configs[activeConfig].orientation;
    uint32_t captureOrientation = ORIENTATION_MAP[displayOrientation];
    sp<GraphicBuffer> outBuffer;
    status_t result = ScreenshotClient::capture(display, Rect(), 0 /* reqWidth */,
        0 /* reqHeight */, INT32_MIN, INT32_MAX, /* all layers */ false, captureOrientation,
        &outBuffer);
    if (result != NO_ERROR) {
        return -1;
    }
    result = outBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &base);
    if (base == NULL) {
        return -1;
    }
    L("ScreenFormat: %d\n", outBuffer->getPixelFormat());
    L("Screenshot client updated its display on init.\n");
    return 0;
}

extern "C" unsigned int *checkfb_flinger()
{
    void* base = NULL;
    Vector<DisplayInfo> configs;
    SurfaceComposerClient::getDisplayConfigs(display, &configs);
    int activeConfig = SurfaceComposerClient::getActiveConfig(display);
    if (static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n",
                activeConfig, configs.size());
        return 0;
    }
    uint8_t displayOrientation = configs[activeConfig].orientation;
    uint32_t captureOrientation = ORIENTATION_MAP[displayOrientation];
    sp<GraphicBuffer> outBuffer;
    status_t result = ScreenshotClient::capture(display, Rect(), 0 /* reqWidth */,
        0 /* reqHeight */, INT32_MIN, INT32_MAX, /* all layers */ false, captureOrientation,
        &outBuffer);
    if (result != NO_ERROR) {
        return 0;
    }
    result = outBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &base);
    if (base == NULL) {
        return 0;
    }
    return (unsigned int*)base;
}

extern "C" unsigned int *readfb_flinger()
{
    void* base = NULL;
    Vector<DisplayInfo> configs;
    SurfaceComposerClient::getDisplayConfigs(display, &configs);
    int activeConfig = SurfaceComposerClient::getActiveConfig(display);
    if (static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n",
                activeConfig, configs.size());
        return 0;
    }
    uint8_t displayOrientation = configs[activeConfig].orientation;
    uint32_t captureOrientation = ORIENTATION_MAP[displayOrientation];
    sp<GraphicBuffer> outBuffer;
    status_t result = ScreenshotClient::capture(display, Rect(), 0 /* reqWidth */,
        0 /* reqHeight */, INT32_MIN, INT32_MAX, /* all layers */ false, captureOrientation,
        &outBuffer);
    if (result != NO_ERROR) {
        return 0;
    }
    result = outBuffer->lock(GraphicBuffer::USAGE_SW_READ_OFTEN, &base);
    if (base == NULL) {
        return 0;
    }
    uint32_t w, h, s, f;
    w = outBuffer->getWidth();
    h = outBuffer->getHeight();
    s = outBuffer->getStride();
    f = outBuffer->getPixelFormat();
    size_t Bpp = bytesPerPixel(f);
    if (s > w) {
        // If stride is greater than width, then the image is non-contiguous in memory
        // so we have copy it into a new array such that it is
        void *new_base = malloc(w * h * Bpp);
        void *tmp_ptr = new_base;

        for (size_t y = 0; y < h; y++) {
            memcpy(tmp_ptr, base, w * Bpp);
            // Pointer arithmetic on void pointers is frowned upon, apparently.
            tmp_ptr = (void *)((char *)tmp_ptr + w * Bpp);
            base = (void *)((char *)base + s * Bpp);
        }
        return (unsigned int *)new_base;
    }
    return (unsigned int *)base;
}

extern "C" void close_flinger()
{
}
