#ifndef LIBDVNC_FLINGER_SCREENFRAME_H
#define LIBDVNC_FLINGER_SCREENFRAME_H

#include <sys/types.h>

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <android/log.h>
#include <gui/ConsumerBase.h> //ConsumerBase
#include <gui/CpuConsumer.h> //CpuConsumer
#include <gui/SurfaceComposerClient.h> //SurfaceComposerClient


#include <ui/DisplayInfo.h> //DisplayInfo
#include <ui/Rect.h> //Rect
#include <ui/PixelFormat.h> //PixelFormat

#define L(...) __android_log_print(ANDROID_LOG_INFO,"VNCserver",__VA_ARGS__);printf(__VA_ARGS__);

class ScreenFrame: public android::ConsumerBase::FrameAvailableListener{
public:
    ScreenFrame() : mFrameAvailable(false),
        mBufferAvailable(false),
        mTimeout(std::chrono::milliseconds(100))
    {
        memset(&mBuffer, 0, sizeof(mBuffer));
    }
    int initialize(int32_t displayId);
    int check();
    int getDisplayInfo(int32_t displayId, android::DisplayInfo* info);
    int updateFrame();
    int waitForFrame();
    void const* getPixels() const;
    android::PixelFormat getFormat() const;
    uint32_t getWidth() const;
    uint32_t getHeight() const;
    uint32_t getStride() const;
    // size of allocated memory in bytes
    uint32_t bitsPerPixel() const;
    uint32_t bytesPerPixel() const;
    size_t getSize() const;

private:
    ScreenFrame(const ScreenFrame&);
    ScreenFrame& operator=(const ScreenFrame&);

    // Destruction via RefBase.
    virtual ~ScreenFrame();

    // (overrides GLConsumer::FrameAvailableListener method)
    virtual void onFrameAvailable(const android::BufferItem& item);
    bool mFrameAvailable;
    bool mBufferAvailable;
    std::mutex mMutex;
    std::condition_variable mEventCond;
    std::chrono::milliseconds mTimeout;
    mutable android::sp<android::CpuConsumer> mCPUConsumer;
    android::CpuConsumer::LockedBuffer mBuffer;

};

#endif /*LIBDVNC_FLINGER_SCREENFRAME_H*/
