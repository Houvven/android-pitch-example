#include "pitch_detector.h"
#include <android/log.h>

#if defined(__ARM_NEON) && defined(__aarch64__)
#include <arm_neon.h>
#endif

#define LOG_TAG "PitchDetector"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

PitchDetector::PitchDetector(PitchCallback callback)
        : mCallback(std::move(callback)) {
    try {
        mPYIN = std::make_unique<PYIN>(kSampleRate, kBufferSize);
        LOGD("PYIN initialized successfully.");
    } catch (const std::exception& e) {
        LOGE("Failed to initialize PYIN: %s", e.what());
    }
}

PitchDetector::~PitchDetector() {
    stop();
}

// 开始音频流
bool PitchDetector::start() {
    if (!mPYIN) {
        LOGE("PYIN not initialized");
        return false;
    }

    oboe::AudioStreamBuilder builder;
    oboe::Result result = builder.setDirection(oboe::Direction::Input)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setFormat(oboe::AudioFormat::Float)
            ->setChannelCount(kChannelCount)
            ->setSampleRate(kSampleRate)
            ->setCallback(this)
            ->setFramesPerCallback(kBufferSize / 2)
            ->setInputPreset(oboe::InputPreset::VoiceRecognition)
            ->openStream(mStream);

    if (result != oboe::Result::OK) {
        LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
        return false;
    }

    mStream->setBufferSizeInFrames(kBufferSize);
    result = mStream->requestStart();
    if (result != oboe::Result::OK) {
        LOGE("Failed to start stream. Error: %s", oboe::convertToText(result));
        return false;
    }

    LOGD("Audio stream started successfully.");
    return true;
}

// 停止音频流
void PitchDetector::stop() {
    if (mStream) {
        mStream->stop();
        mStream->close();
        mStream.reset();
        LOGD("Audio stream stopped and closed.");
    }
}

oboe::DataCallbackResult PitchDetector::onAudioReady(
        oboe::AudioStream* audioStream,
        void* audioData,
        int32_t numFrames) {

    if (!mPYIN) {
        return oboe::DataCallbackResult::Stop;
    }

    auto* floatData = static_cast<float*>(audioData);

    float energy = 0.0f;
#if defined(__ARM_NEON) && defined(__aarch64__)
    float32x4_t sum = vdupq_n_f32(0);
        int i;
        for (i = 0; i + 4 <= numFrames; i += 4) {
            float32x4_t data = vld1q_f32(&floatData[i]);
            sum = vmlaq_f32(sum, data, data);
        }
        float32x2_t sum2 = vadd_f32(vget_low_f32(sum), vget_high_f32(sum));
        float32x2_t sum1 = vpadd_f32(sum2, sum2);
        energy = vget_lane_f32(sum1, 0);

        // 处理剩余样本
        for (; i < numFrames; i++) {
            energy += floatData[i] * floatData[i];
        }
#else
    for (int i = 0; i < numFrames; i++) {
        energy += floatData[i] * floatData[i];
    }
#endif
    energy /= numFrames;

    // 如果能量低于阈值，忽略此帧
    if (energy < kEnergyThreshold) {
        mStablePitchCount = 0;
        return oboe::DataCallbackResult::Continue;
    }

    try {
        float pitch = mPYIN->getPitch(floatData, numFrames);

        if (pitch > kMinValidPitch && pitch < kMaxValidPitch) {
            if (std::abs(pitch - mLastValidPitch) < 1.0f) {
                mStablePitchCount++;
                if (mStablePitchCount >= kStablePitchThreshold) {
                    mCallback(pitch);
                }
            } else {
                mStablePitchCount = 0;
            }
            mLastValidPitch = pitch;
        }
    } catch (const std::exception& e) {
        LOGE("Error in pitch detection: %s", e.what());
    }

    return oboe::DataCallbackResult::Continue;
}

void PitchDetector::onErrorBeforeClose(oboe::AudioStream* audioStream, oboe::Result error) {
    LOGE("Error before close: %s", oboe::convertToText(error));
}

void PitchDetector::onErrorAfterClose(oboe::AudioStream* audioStream, oboe::Result error) {
    LOGE("Error after close: %s", oboe::convertToText(error));
}

bool PitchDetector::isRunning() const {
    if (mStream) {
        auto state = mStream->getState();
        return state == oboe::StreamState::Started || state == oboe::StreamState::Starting;
    }
    return false;
}
