//
//  AKBoosterDSPKernel.hpp
//  AudioKit
//
//  Created by Aurelius Prochazka, revision history on Github.
//  Copyright © 2017 Aurelius Prochazka. All rights reserved.
//

#pragma once

#import "DSPKernel.hpp"
#import "ParameterRamper.hpp"
#import <Accelerate/Accelerate.h>
#import <AudioKit/AudioKit-Swift.h>

enum {
    gainAddress = 0
};

class AKBoosterDSPKernel : public AKDSPKernel, public AKBuffered {
public:
    // MARK: Member Functions

    AKBoosterDSPKernel() {}

    void init(int _channels, double _sampleRate) override {
        AKDSPKernel::init(_channels, _sampleRate);
        gainRamper.init();
    }

    void start() {
        started = true;
    }

    void stop() {
        started = false;
    }

    void destroy() {
    }
    
    void reset() {
        resetted = true;
        gainRamper.reset();
    }

    void setGain(float value) {
        gain = clamp(value, -100000.0f, 100000.0f);
        gainRamper.setImmediate(gain);
    }


    void setParameter(AUParameterAddress address, AUValue value) {
        switch (address) {
            case gainAddress:
                gainRamper.setUIValue(clamp(value, -100000.0f, 100000.0f));
                break;

        }
    }

    AUValue getParameter(AUParameterAddress address) {
        switch (address) {
            case gainAddress:
                return gainRamper.getUIValue();

            default: return 0.0f;
        }
    }

    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration) override {
        switch (address) {
            case gainAddress:
                gainRamper.startRamp(clamp(value, -100000.0f, 100000.0f), duration);
                break;
        }
    }

    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override {
        
        float startGain = gainRamper.get();
        gainRamper.stepBy(frameCount);
        float endGain = gainRamper.get();
        
        vDSP_vgen(&startGain, &endGain, &ramp[0], 1, frameCount);
        
        float *leftIn = (float *)inBufferListPtr->mBuffers[0].mData + bufferOffset;
        float *rightIn;
        
        gain = gainRamper.get();
        
        if (channels > 1) {
            rightIn = (float *)inBufferListPtr->mBuffers[1].mData + bufferOffset;
        } else {
            rightIn = leftIn;
        }
        
        float *outLeft = (float *)outBufferListPtr->mBuffers[0].mData + bufferOffset;
        vDSP_vsmul(leftIn, 1, &gain, outLeft, 1, frameCount);
        
        if (channels > 1) {
            float *outRight = (float *)outBufferListPtr->mBuffers[1].mData + bufferOffset;
            vDSP_vsmul(rightIn, 1, &gain, outRight, 1, frameCount);
        }
    }

    // MARK: Member Variables

private:

    float gain = 1.0;
    float ramp[1024];

public:
    bool started = true;
    bool resetted = false;
    ParameterRamper gainRamper = 0;
};

