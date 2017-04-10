//
//  AKJSMSequencerDSPKernel.h
//  AudioKit For iOS
//
//  Created by James Milton on 09/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#pragma once

#import "DSPKernel.hpp"
#include <mach/mach_time.h>
#include <vector>

struct Note {
    uint8_t noteNumber;
    uint8_t velocity;
    double position;
};

struct Track {
    MIDIEndpointRef endpointRef;
    Note notes[64];
};

class AKJSMSequencerDSPKernel: public AKDSPKernel, public AKOutputBuffered {
public:
    
    AKJSMSequencerDSPKernel() {}
    ~AKJSMSequencerDSPKernel() {}
    
    void init(int _channels, double _sampleRate) override {
        AKDSPKernel::init(channels, sampleRate);
    }
    
    void setMIDIClientRef(MIDIClientRef client) {
        MIDIOutputPortCreate(client, CFSTR("Deep-808 Internal"), &outputPort);
    }
    
    void start() {
        started = true;
    }
    
    void stop() {
        started = false;
    }
    
    void destroy() {
        MIDIPortDispose(outputPort);
    }
    
    void reset() {
        resetted = true;
    }
    
    void startRamp(AUParameterAddress address, AUValue value, AUAudioFrameCount duration) override {}
    
    void process(AUAudioFrameCount frameCount, AUAudioFrameCount bufferOffset) override {
        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            int frameOffset = int(frameIndex + bufferOffset);
            for (int channel = 0; channel < channels; ++channel) {
                float *out = (float *)outBufferListPtr->mBuffers[channel].mData + frameOffset;
                *out = 0.0;
            }
        }
    }
    
    uint64_t convertTimeInNanoseconds(uint64_t time)
    {
        static mach_timebase_info_data_t s_timebase_info;
        
        if (s_timebase_info.denom == 0)
        {
            (void) mach_timebase_info(&s_timebase_info);
        }
        
        return (uint64_t)((time * s_timebase_info.numer) / s_timebase_info.denom);
    }
    
    void processWithEvents(AudioTimeStamp const* timestamp,
                           AUAudioFrameCount frameCount,
                           AURenderEvent const* events)
    {
        if (firstTimestamp == 0) {
            firstTimestamp = timestamp->mHostTime;
        }
        
        uint64_t elapsedHostTime = convertTimeInNanoseconds(timestamp->mHostTime - firstTimestamp);
        double seconds = (double)elapsedHostTime * (1.0 / 1e+9);
        beats = (seconds * 120.0) / 60.0;
        
//        if (seconds >= 0.25) {
//            seconds = 0;
//            firstTimestamp = timestamp->mHostTime;
//            
//            for (int i = 0; i < 16; i++) {
//                MIDIPacketList packetList;
//                
//                packetList.numPackets = 1;
//                
//                MIDIPacket* firstPacket = &packetList.packet[0];
//                
//                firstPacket->timeStamp = 0; // send immediately
//                
//                firstPacket->length = 3;
//                
//                firstPacket->data[0] = i;
//                
//                firstPacket->data[1] = 0x90;
//                firstPacket->data[2] = 127;
//                
//                MIDISend(outputPort, ref, &packetList);
//            }
//        }
    }
    
public:
    bool started = false;
    bool resetted = false;
    double beats = 0;
    
private:
    MIDIPortRef outputPort;
    Track tracks[16];
    uint64_t firstTimestamp = 0;
};