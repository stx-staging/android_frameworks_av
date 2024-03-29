/*
**
** Copyright (C) 2008 The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef ANDROID_VIDEO_FRAME_H
#define ANDROID_VIDEO_FRAME_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>

namespace android {

// Represents a color converted (RGB-based) video frame with bitmap
// pixels stored in FrameBuffer.
// In a VideoFrame struct stored in IMemory, frame data and ICC data
// come after the VideoFrame structure. Their locations can be retrieved
// by getFlattenedData() and getFlattenedIccData();
class VideoFrame
{
public:
    // Construct a VideoFrame object with the specified parameters,
    // will calculate frame buffer size if |hasData| is set to true.
    VideoFrame(uint32_t width, uint32_t height,
            uint32_t displayWidth, uint32_t displayHeight,
            uint32_t displayLeft, uint32_t displayTop,
            uint32_t tileWidth, uint32_t tileHeight,
            uint32_t angle, uint32_t bpp, uint32_t bitDepth, bool hasData, size_t iccSize):
        mWidth(width), mHeight(height),
        mDisplayWidth(displayWidth), mDisplayHeight(displayHeight),
        mDisplayLeft(displayLeft), mDisplayTop(displayTop),
        mTileWidth(tileWidth), mTileHeight(tileHeight), mDurationUs(0),
        mRotationAngle(angle), mBytesPerPixel(bpp), mIccSize(iccSize),
        mBitDepth(bitDepth) {
            uint32_t multVal;
            mRowBytes = __builtin_mul_overflow(bpp, width, &multVal) ? 0 : multVal;
            mSize = __builtin_mul_overflow(multVal, height, &multVal) ? 0 : multVal;
            if (hasData && (mRowBytes == 0 || mSize == 0)) {
                ALOGE("Frame rowBytes/ size overflow %dx%d bpp %d", width, height, bpp);
                android_errorWriteLog(0x534e4554, "233006499");
            }
    }

    void init(const VideoFrame& copy, const void* iccData, size_t iccSize) {
        *this = copy;
        if (mIccSize == iccSize && iccSize > 0 && iccData != NULL) {
            memcpy(getFlattenedIccData(), iccData, iccSize);
        } else {
            mIccSize = 0;
        }
    }

    // Calculate the flattened size to put it in IMemory
    size_t getFlattenedSize() const {
        return sizeof(VideoFrame) + mSize + mIccSize;
    }

    // Get the pointer to the frame data in a flattened VideoFrame in IMemory
    uint8_t* getFlattenedData() const {
        return (uint8_t*)this + sizeof(VideoFrame);
    }

    // Get the pointer to the ICC data in a flattened VideoFrame in IMemory
    uint8_t* getFlattenedIccData() const {
        return (uint8_t*)this + sizeof(VideoFrame) + mSize;
    }

    // Intentional public access modifier:
    uint32_t mWidth;           // Decoded image width before rotation
    uint32_t mHeight;          // Decoded image height before rotation
    uint32_t mDisplayWidth;    // Display width before rotation
    uint32_t mDisplayHeight;   // Display height before rotation
    uint32_t mDisplayLeft;     // Display left (column coordinate) before rotation
    uint32_t mDisplayTop;      // Display top (row coordinate) before rotation
    uint32_t mTileWidth;       // Tile width (0 if image doesn't have grid)
    uint32_t mTileHeight;      // Tile height (0 if image doesn't have grid)
    int64_t  mDurationUs;      // Frame duration in microseconds
    int32_t  mRotationAngle;   // Rotation angle, clockwise, should be multiple of 90
    uint32_t mBytesPerPixel;   // Number of bytes per pixel
    uint32_t mRowBytes;        // Number of bytes per row before rotation
    uint32_t mSize;            // Number of bytes of frame data
    uint32_t mIccSize;         // Number of bytes of ICC data
    uint32_t mBitDepth;        // number of bits per R / G / B channel
};

}; // namespace android

#endif // ANDROID_VIDEO_FRAME_H
