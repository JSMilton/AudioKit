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
#include <float.h>

struct Note {
    uint8_t noteNumber;
    uint8_t velocity;
    double position = -1;
};

struct Track {
    MIDIEndpointRef endpointRef;
    Note notes[64];
    double lastPlayedPosition = DBL_MAX;
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
    
    void createTrack(int trackIndex, MIDIEndpointRef endpoint) {
        tracks[trackIndex].endpointRef = endpoint;
    }
    
    void addNote(uint8_t noteNumber, uint8_t velocity, double position, int trackIndex) {
        tracks[trackIndex].notes[0].noteNumber = noteNumber;
        tracks[trackIndex].notes[0].velocity = velocity;
        tracks[trackIndex].notes[0].position = position;
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
    
    void playMIDINote(uint8_t note, uint8_t velocity, MIDIEndpointRef ref) {
        MIDIPacketList packetList;
        packetList.numPackets = 1;
        MIDIPacket* firstPacket = &packetList.packet[0];
        firstPacket->timeStamp = 0;
        firstPacket->length = 3;
        firstPacket->data[0] = 0x90;
        firstPacket->data[1] = 0;
        firstPacket->data[2] = 127;
        
        MIDIPacketList packetList2;
        packetList2.numPackets = 1;
        MIDIPacket* firstPacket2 = &packetList2.packet[0];
        firstPacket2->timeStamp = 1000000;
        firstPacket2->length = 3;
        firstPacket2->data[0] = 0x80;
        firstPacket2->data[1] = note;
        firstPacket2->data[2] = velocity;
        
        MIDISend(outputPort, ref, &packetList);
        MIDISend(outputPort, ref, &packetList2);
    }
    
    void processWithEvents(AudioTimeStamp const* timestamp,
                           AUAudioFrameCount frameCount,
                           AURenderEvent const* events)
    {
        if (!started) { return; }
        
        if (firstTimestamp == 0) {
            firstTimestamp = timestamp->mHostTime;
        }
        
        uint64_t elapsedHostTime = convertTimeInNanoseconds(timestamp->mHostTime - firstTimestamp);
        double seconds = (double)elapsedHostTime * (1.0 / 1e+9);
        beats = (seconds * tempo) / 60.0;

        double relativeBeat = fmod(beats, 4.0);
        
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 64; j++) {
                double pos = tracks[i].notes[j].position;
                if (pos <= relativeBeat && tracks[i].lastPlayedPosition != pos && pos >= 0) {
                    playMIDINote(tracks[i].notes[j].noteNumber, tracks[i].notes[j].velocity, tracks[i].endpointRef);
                    tracks[i].lastPlayedPosition = pos;
                }
            }
        }
    }
    
public:
    bool started = false;
    bool resetted = false;
    double beats = 0;
    double tempo = 120.0;
    
private:
    MIDIPortRef outputPort;
    Track tracks[16];
    uint64_t firstTimestamp = 0;
};
