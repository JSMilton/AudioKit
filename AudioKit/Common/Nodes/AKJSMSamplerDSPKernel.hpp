//
//  AKJSMSamplerDSPKernel.h
//  AudioKit For iOS
//
//  Created by Jambo on 15/05/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#pragma once

#import "DSPKernel.hpp"

class AKJSMSamplerDSPKernel: public AKDSPKernel, public AKOutputBuffered {
public:
    
    AKJSMSamplerDSPKernel() {}
    ~AKJSMSamplerDSPKernel() {}
    
    void init(int _channels, double _sampleRate) override {
        AKDSPKernel::init(channels, sampleRate);
    }
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration) override {}
    
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override {
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            int frameOffset = int(frameIndex + bufferOffset);
            for (int channel = 0; channel < channels; ++channel) {
                float *out = (float *)outBufferListPtr->mBuffers[channel].mData + frameOffset;
                if (started) {
//                    if (data[currentSample] != 0) {
//                        printf("%f\n", data[currentSample]);
//                    }
                    *out = data[currentSample];
                    //*out = sin((currentSample / (44100.0 / 440.0)) * (M_PI*2));
//                    if (channelCount == 1) {§
//                        *out = data[0][currentSample + channel * isInterleaved];
//                    } else {
//                        *out = data[channel][currentSample + channel * isInterleaved];
//                    }
                } else {
                    *out = 0;
                }
            }
            
            currentSample++;
            if (currentSample >= sampleCount) {
                stop();
            }
        }
        
        //printf("samp: %f\n", data[0][currentSample]);
    }
    
    void start() {
        started = true;
        currentSample = 0;
    }
    
    void stop() {
        started = false;
    }
    
    void reset() {
        resetted = true;
    }
    
    void destroy() {
        free(data);
    }
    
    void setSampleData(AVAudioPCMBuffer *pcmBuffer) {
        
        size_t size = pcmBuffer.frameCapacity * sizeof(float);
        data = (float*)malloc(size);
        
        for (int i = 0; i < pcmBuffer.frameCapacity; i++) {
            float s = pcmBuffer.floatChannelData[0][i];
            data[i] = s;
        }
        
        //memcpy(data, pcmBuffer.floatChannelData, size);
        sampleCount = pcmBuffer.frameLength;
        stride = (int)pcmBuffer.stride;
        isInterleaved = pcmBuffer.format.isInterleaved;
        channelCount = pcmBuffer.format.channelCount;
    }
    
public:
    bool started = false;
    bool resetted = false;
    
private:
    float *data;
    int stride;
    int sampleCount;
    int currentSample = 0;
    int channelCount;
    bool isInterleaved;
};

