#pragma once

#include <atomic>
#include <functional>
#include <vector>
#include "pyin.h"
#include "oboe/Oboe.h"

class PitchDetector : public oboe::AudioStreamCallback {
public:
    using PitchCallback = std::function<void(float)>;

    explicit PitchDetector(PitchCallback callback);

    ~PitchDetector() override;

    bool start();

    void stop();

    [[nodiscard]] bool isRunning() const;

private:
    oboe::DataCallbackResult onAudioReady(
            oboe::AudioStream *audioStream,
            void *audioData,
            int32_t numFrames) override;

    void onErrorBeforeClose(oboe::AudioStream *audioStream, oboe::Result error) override;

    void onErrorAfterClose(oboe::AudioStream *audioStream, oboe::Result error) override;

    std::shared_ptr<oboe::AudioStream> mStream;
    std::unique_ptr<PYIN> mPYIN;
    PitchCallback mCallback;

    static constexpr int32_t kSampleRate = 44100;
    static constexpr int32_t kChannelCount = 1;
    static constexpr int32_t kBufferSize = 2048;
    static constexpr float kMinValidPitch = 50.0f;
    static constexpr float kMaxValidPitch = 2000.0f;
    static constexpr float kEnergyThreshold = 0.001f;
    static constexpr int kStablePitchThreshold = 3;

    float mLastValidPitch{0.0f};
    int mStablePitchCount{0};
};
