#include "ScreenFrame.h"

static bool isDeviceRotated(int orientation) {
    return orientation != android::DISPLAY_ORIENTATION_0 &&
            orientation != android::DISPLAY_ORIENTATION_180;
}

static android::status_t setDisplayProjection(const android::sp<android::IBinder>& dpy,
        const android::DisplayInfo& info) {
    // Set the region of the layer stack we're interested in, which in our
    // case is "all of it".  If the app is rotated (so that the width of the
    // app is based on the height of the display), reverse width/height.
    bool deviceRotated = isDeviceRotated(info.orientation);
    uint32_t sourceWidth, sourceHeight;
    if (!deviceRotated) {
        sourceWidth = info.w;
        sourceHeight = info.h;
    } else {
        L("using rotated width/height\n");
        sourceHeight = info.w;
        sourceWidth = info.h;
    }
    android::Rect layerStackRect(sourceWidth, sourceHeight);
    android::Rect displayRect(sourceWidth, sourceHeight);
    android::SurfaceComposerClient::setDisplayProjection(dpy,
        android::DISPLAY_ORIENTATION_0,
        layerStackRect, displayRect);
    return android::NO_ERROR;
}

/*
 * Configures the virtual display.  When this completes, virtual display
 * frames will start arriving from the buffer producer.
 */
static android::status_t prepareVirtualDisplay(const android::DisplayInfo& mainDpyInfo,
        const android::sp<android::IGraphicBufferProducer>& bufferProducer,
        android::sp<android::IBinder>* pDisplayHandle) {
    android::sp<android::IBinder> dpy = android::SurfaceComposerClient::createDisplay(
            android::String8("vncserver"), true /*secure*/);

    android::SurfaceComposerClient::openGlobalTransaction();
    android::SurfaceComposerClient::setDisplaySurface(dpy, bufferProducer);
    setDisplayProjection(dpy, mainDpyInfo);
    android::SurfaceComposerClient::setDisplayLayerStack(dpy, 0);    // default stack
    android::SurfaceComposerClient::closeGlobalTransaction();
    *pDisplayHandle = dpy;
    return android::NO_ERROR;
}

// Callback; executes on arbitrary thread.
void ScreenFrame::onFrameAvailable(const android::BufferItem& /* item */) {
    std::unique_lock<std::mutex> lock(mMutex);
    mFrameAvailable = true;
    mEventCond.notify_one();
}

int ScreenFrame::waitForFrame() {
    std::unique_lock<std::mutex> lock(mMutex);
    while(true) {
        if (mEventCond.wait_for(lock, mTimeout, [this]{return mFrameAvailable;})) {
            L("Got Frame from waiting\n");
            return 0;
        }
    }
    return 0;
}

int ScreenFrame::updateFrame() {
    android::status_t err;
    std::unique_lock<std::mutex> lock(mMutex);
    if(!mFrameAvailable) {
        return android::NO_ERROR;
    }
    if(mBufferAvailable) {
        mCPUConsumer->unlockBuffer(mBuffer);
        memset(&mBuffer, 0, sizeof(mBuffer));
        mBufferAvailable = false;
    }
    err = mCPUConsumer->lockNextBuffer(&mBuffer);
    if(err != android::NO_ERROR) {
        L("Unable to get lockNextBuffer\n");
        return err;
    }
    mBufferAvailable = true;
    mFrameAvailable = false;
    return android::NO_ERROR;
}

int ScreenFrame::initialize(
    int32_t displayId
) {
    uint32_t sourceWidth, sourceHeight;
    uint32_t targetWidth, targetHeight;
    android::status_t err;
    android::sp<android::IGraphicBufferProducer> producer;
    android::sp<android::IGraphicBufferConsumer> consumer;
    android::DisplayInfo info;
    err = getDisplayInfo(displayId, &info);
    if(err != android::NO_ERROR) {
        L("Unable to get display info\n");
        return err;
    }

    bool rotated = isDeviceRotated(info.orientation);
    sourceWidth = rotated ? info.h : info.w;
    sourceHeight = rotated ? info.w : info.h;
    targetWidth = sourceWidth;
    targetHeight = sourceHeight;

    L("Creating buffer queue\n");
    android::BufferQueue::createBufferQueue(&producer, &consumer);
    L("Setting buffer options\n");
    consumer->setDefaultBufferSize(targetWidth, targetHeight);
    consumer->setDefaultBufferFormat(android::PIXEL_FORMAT_RGBA_8888);
    L("Creating CPU consumer\n");
    mCPUConsumer = new android::CpuConsumer(consumer, 1);
    mCPUConsumer->setName(android::String8("vncserver"));
    L("Creating frame waiter\n");
    mCPUConsumer->setFrameAvailableListener(this);
    android::sp<android::IBinder> dpy;
    err = prepareVirtualDisplay(info, producer, &dpy);
    return err;
}

int ScreenFrame::check() {
    android::status_t err;
    L("Creating SurfaceComposerClient\n");
    android::sp<android::SurfaceComposerClient> sc = new android::SurfaceComposerClient();

    L("Performing SurfaceComposerClient init check\n");
    if ((err = sc->initCheck()) != android::NO_ERROR) {
        L("Unable to initialize SurfaceComposerClient\n");
        return err;
    }

    sc = NULL;
    return 0;
}

int ScreenFrame::getDisplayInfo(int32_t displayId, android::DisplayInfo* info) {
    android::sp<android::IBinder> dpy = android::SurfaceComposerClient::getBuiltInDisplay(displayId);

    android::Vector<android::DisplayInfo> configs;
    android::status_t err = android::SurfaceComposerClient::getDisplayConfigs(dpy, &configs);

    if (err != android::NO_ERROR) {
    L("SurfaceComposerClient::getDisplayInfo() failed: %d\n", err);
        return err;
    }

    int activeConfig = android::SurfaceComposerClient::getActiveConfig(dpy);
    if(static_cast<size_t>(activeConfig) >= configs.size()) {
        L("Active config %d not inside configs (size %zu)\n", activeConfig, configs.size());
        return android::BAD_VALUE;
    }

    android::DisplayInfo dinfo = configs[activeConfig];
    *info = dinfo;
    return 0;
}

void const* ScreenFrame::getPixels() const {
    return mBuffer.data;
};

android::PixelFormat ScreenFrame::getFormat() const {
    return mBuffer.format;
};

uint32_t ScreenFrame::getWidth() const {
    return mBuffer.width;
};

uint32_t ScreenFrame::getHeight() const {
    return mBuffer.height;
};

uint32_t ScreenFrame::getStride() const {
    return mBuffer.stride;
};

// size of allocated memory in bytes
uint32_t ScreenFrame::bitsPerPixel() const {
    return android::bitsPerPixel(mBuffer.format);
};

uint32_t ScreenFrame::bytesPerPixel() const {
    return android::bytesPerPixel(mBuffer.format);
};

size_t ScreenFrame::getSize() const {
    return mBuffer.stride * mBuffer.height * bytesPerPixel();
};

ScreenFrame::~ScreenFrame() {
    if (mBufferAvailable) {
        mCPUConsumer->unlockBuffer(mBuffer);
        memset(&mBuffer, 0, sizeof(mBuffer));
        mBufferAvailable = false;
    }
    mCPUConsumer.clear();
}
