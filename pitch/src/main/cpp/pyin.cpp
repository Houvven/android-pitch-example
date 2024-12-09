#include "pyin.h"
#include <cmath>
#include <algorithm>
#include <stdexcept>

PYIN::PYIN(int sampleRate, int bufferSize) {
    if (sampleRate <= 0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    if (bufferSize < kMinBufferSize || bufferSize > kMaxBufferSize) {
        throw std::invalid_argument("Buffer size out of range");
    }

    mSampleRate = sampleRate;
    mBufferSize = bufferSize;
    mThreshold = 0.1f;
    mYinBuffer.resize(bufferSize / 2);
}

float PYIN::getPitch(const float* buffer, int size) {
    if (!buffer || size <= 0) {
        throw std::invalid_argument("Invalid buffer");
    }
    if (size > mBufferSize) {
        throw std::invalid_argument("Buffer size too large");
    }

    computeYinBuffer(buffer, size);

    int tau = 0;
    float minProb = 1.0f;
    float bestPeriod = 0.0f;

    // 搜索最佳周期
    const int minTau = mSampleRate / 2000;  // 最高频率限制
    const int maxTau = mSampleRate / 50;    // 最低频率限制

    for (int i = minTau; i < maxTau && i < mYinBuffer.size(); i++) {
        float prob = probabilisticYinStep(i);
        if (prob < minProb) {
            minProb = prob;
            tau = i;
        }
    }

    // 使用抛物线插值提高精度
    if (tau > 0 && tau < mYinBuffer.size() - 1) {
        float alpha = mYinBuffer[tau - 1];
        float beta = mYinBuffer[tau];
        float gamma = mYinBuffer[tau + 1];
        float peak = tau + 0.5f * (alpha - gamma) / (alpha - 2.0f * beta + gamma);
        bestPeriod = peak;
    } else {
        bestPeriod = tau;
    }

    // 只有当概率足够低时才返回音高
    return (minProb < mThreshold && bestPeriod > 0) ? mSampleRate / bestPeriod : 0.0f;
}

void PYIN::computeYinBuffer(const float* buffer, int size) {
    int halfSize = size / 2;

    // 预计算平方差并存储
    for (int tau = 0; tau < halfSize; tau++) {
        mYinBuffer[tau] = 0.0f;
        for (int i = 0; i < halfSize; i++) {
            if (i + tau < size) {
                float diff = buffer[i] - buffer[i + tau];
                mYinBuffer[tau] += diff * diff;
            }
        }
    }

    // 优化：改进运行和累加和的计算
    float runningSum = 0.0f;
    mYinBuffer[0] = 1.0f;  // 处理第一个值为最大值

    for (int tau = 1; tau < halfSize; tau++) {
        runningSum += mYinBuffer[tau];
        if (runningSum > 0) {
            mYinBuffer[tau] *= tau / runningSum;
        }
    }
}

float PYIN::probabilisticYinStep(int tau) {
    if (tau == 0) return 1.0f;

    float previousMinimum = mYinBuffer[tau - 1];
    float currentValue = mYinBuffer[tau];
    float nextValue = tau < mYinBuffer.size() - 1 ? mYinBuffer[tau + 1] : 1.0f;

    // 优化：如果不是局部最小值，提前退出
    if (currentValue >= previousMinimum || currentValue >= nextValue) {
        return 1.0f;
    }

    return currentValue;
}
