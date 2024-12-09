#pragma once

#include <vector>
#include <stdexcept>

class PYIN {
public:
    PYIN(int sampleRate, int bufferSize);
    float getPitch(const float* buffer, int size);

private:
    void computeYinBuffer(const float* buffer, int size);
    float probabilisticYinStep(int tau);

    int mSampleRate;
    int mBufferSize;
    float mThreshold;
    std::vector<float> mYinBuffer;

    static constexpr int kMinBufferSize = 64;
    static constexpr int kMaxBufferSize = 16384;
};