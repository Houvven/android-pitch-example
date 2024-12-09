#pragma once

#include <atomic>
#include <functional>
#include "pyin.h"
#include "oboe/Oboe.h"

class PitchDetector : public oboe::AudioStreamCallback {
public:
    using PitchCallback = std::function<void(float)>;
    
    explicit PitchDetector(PitchCallback callback);
    ~PitchDetector();

    bool start();
    void stop();

private:
    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream* audioStream,
        void* audioData,
        int32_t numFrames) override;

    void onErrorBeforeClose(oboe::AudioStream* audioStream, oboe::Result error) override;
    void onErrorAfterClose(oboe::AudioStream* audioStream, oboe::Result error) override;

    std::shared_ptr<oboe::AudioStream> mStream;
    std::unique_ptr<PYIN> mPYIN;
    PitchCallback mCallback;
    
    static constexpr int32_t kSampleRate = 22050;
    static constexpr int32_t kChannelCount = 1;
    static constexpr int32_t kBufferSize = 1024;
    static constexpr float kMinValidPitch = 50.0f;
    static constexpr float kMaxValidPitch = 2000.0f;
    static constexpr int32_t kProcessingThreads = 2;
    static constexpr int32_t kRingBufferSize = 4096;
    static constexpr float kEnergyThreshold = 0.001f;
    
    std::vector<float> mProcessingBuffer;
    float mLastValidPitch{0.0f};
    int mStablePitchCount{0};
    static constexpr int kStablePitchThreshold = 3;
}; 