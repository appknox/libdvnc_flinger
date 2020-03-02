#include <cstdint> //for uint32_t
#include <iostream>
#include <stdio.h>
#include <unistd.h> //sleep

#include <binder/ProcessState.h> //ThreadPool

#include <gui/SurfaceComposerClient.h> //SurfaceComposerClient

#include <ui/DisplayInfo.h> //DisplayInfo

#include "ScreenFrame.h"
#define DEFAULT_DISPLAY_ID 0


struct DisplayInfo {
    uint32_t width;
    uint32_t height;
    float fps;
    float density;
    float xdpi;
    float ydpi;
    float size;
    uint8_t orientation;
    bool secure;
};

enum Orientation {
    ORIENTATION_0    = 0,
    ORIENTATION_90   = 1,
    ORIENTATION_180  = 2,
    ORIENTATION_270  = 3,
};

static const char*
error_name(int32_t err) {
    switch (err) {
        case android::NO_ERROR: // also android::OK
            return "NO_ERROR";
        case android::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        case android::NO_MEMORY:
            return "NO_MEMORY";
        case android::INVALID_OPERATION:
            return "INVALID_OPERATION";
        case android::BAD_VALUE:
            return "BAD_VALUE";
        case android::BAD_TYPE:
            return "BAD_TYPE";
        case android::NAME_NOT_FOUND:
            return "NAME_NOT_FOUND";
        case android::PERMISSION_DENIED:
            return "PERMISSION_DENIED";
        case android::NO_INIT:
            return "NO_INIT";
        case android::ALREADY_EXISTS:
            return "ALREADY_EXISTS";
        case android::DEAD_OBJECT: // also android::JPARKS_BROKE_IT
            return "DEAD_OBJECT";
        case android::FAILED_TRANSACTION:
            return "FAILED_TRANSACTION";
        case android::BAD_INDEX:
            return "BAD_INDEX";
        case android::NOT_ENOUGH_DATA:
            return "NOT_ENOUGH_DATA";
        case android::WOULD_BLOCK:
            return "WOULD_BLOCK";
        case android::TIMED_OUT:
            return "TIMED_OUT";
        case android::UNKNOWN_TRANSACTION:
            return "UNKNOWN_TRANSACTION";
        case android::FDS_NOT_ALLOWED:
            return "FDS_NOT_ALLOWED";
        default:
            return "UNMAPPED_ERROR";
    }
}

int get_display_info(int32_t displayId, DisplayInfo* info) {
    android::sp<android::IBinder> dpy = android::SurfaceComposerClient::getBuiltInDisplay(displayId);

    android::Vector<android::DisplayInfo> configs;
    android::status_t err = android::SurfaceComposerClient::getDisplayConfigs(dpy, &configs);

    if (err != android::NO_ERROR) {
    L("SurfaceComposerClient::getDisplayInfo() failed: %s (%d)\n", error_name(err), err);
        return err;
    }

    int activeConfig = android::SurfaceComposerClient::getActiveConfig(dpy);
    if(static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n", activeConfig, configs.size());
        return android::BAD_VALUE;
    }

    android::DisplayInfo dinfo = configs[activeConfig];

    info->width = dinfo.w;
    info->height = dinfo.h;
    info->orientation = dinfo.orientation;
    info->fps = dinfo.fps;
    info->density = dinfo.density;
    info->xdpi = dinfo.xdpi;
    info->ydpi = dinfo.ydpi;
    info->secure = dinfo.secure;
    info->size = sqrt(pow(dinfo.w / dinfo.xdpi, 2) + pow(dinfo.h / dinfo.ydpi, 2));

    return 0;
}

int showInfo(uint32_t displayId){
    DisplayInfo info;

    if (get_display_info(displayId, &info) != 0) {
        L("Unable to get display info\n");
        return -1;
    }

    int rotation;
    switch (info.orientation) {
    case ORIENTATION_0:
      rotation = 0;
      break;
    case ORIENTATION_90:
      rotation = 90;
      break;
    case ORIENTATION_180:
      rotation = 180;
      break;
    case ORIENTATION_270:
      rotation = 270;
      break;
    }

    std::cout.precision(2);
    std::cout.setf(std::ios_base::fixed, std::ios_base::floatfield);

    std::cout << "{"                                         << std::endl
              << "    \"id\": "       << displayId    << "," << std::endl
              << "    \"width\": "    << info.width   << "," << std::endl
              << "    \"height\": "   << info.height  << "," << std::endl
              << "    \"xdpi\": "     << info.xdpi    << "," << std::endl
              << "    \"ydpi\": "     << info.ydpi    << "," << std::endl
              << "    \"size\": "     << info.size    << "," << std::endl
              << "    \"density\": "  << info.density << "," << std::endl
              << "    \"fps\": "      << info.fps     << "," << std::endl
              << "    \"secure\": "   << (info.secure ? "true" : "false") << "," << std::endl
              << "    \"rotation\": " << rotation            << std::endl
              << "}"                                         << std::endl;
    return 0;
}

int capture(uint32_t displayId) {
    android::ProcessState::self()->startThreadPool();
    android::sp<ScreenFrame> sf;
    int err;
    sf = new ScreenFrame();
    err = sf->check();
    if(err != 0) {
        return err;
    }
    sf->initialize(displayId);
    // int i = 10;
    sf->updateFrame();
    sf->waitForFrame();
    sf->updateFrame();
    L("getWidth(%d)\n", (int)sf->getWidth());
    L("getHeight(%d)\n", (int)sf->getHeight());
    L("getFormat(%d)\n", (int)sf->getFormat());
    L("getStride(%d)\n", (int)sf->getStride());
    L("bitsPerPixel(%d)\n", (int)sf->bitsPerPixel());
    L("bytesPerPixel(%d)\n", (int)sf->bytesPerPixel());
    L("getSize(%d)\n", (int)sf->getSize());

    FILE *pFile;
    pFile = fopen("test.dat", "w+b");
    fwrite(sf->getPixels(), sf->getSize(), 1, pFile);
    fclose(pFile);
    // while (i > 0) {
    //     i--;
    //     sf->updateFrame();
    //     width = sf->getWidth();
    //     L("Sleeping...Width(%d)\n", width);
    //     sleep(2);
    // }
    return 0;
}

int main(int argc, char* const argv[]) {
    uint32_t displayId = DEFAULT_DISPLAY_ID;
    if (argc == 1) {
        L("One arg\n");
    }
    for (int i = 0; argv[i] != NULL; i++) {
        L(" %s", argv[i]);
    }
    showInfo(displayId);
    capture(displayId);
    return 0;
}



