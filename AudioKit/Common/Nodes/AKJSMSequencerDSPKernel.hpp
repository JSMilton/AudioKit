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
#include "TPCircularBuffer.h"
#include "SECommon.h"
#include "ABLLink.h"
#include "AKJSMSamplerDSPKernel.hpp"

struct Note {
    uint8_t noteNumber;
    uint8_t velocity;
    double position = -1;
};

struct Track {
    MIDIEndpointRef endpointRef;
    Note notes[64];
};

typedef enum UpdateType {
    ADD,
    MOVE,
    REMOVE,
    UPDATE,
    CLEAR,
    POSITION
} UpdateType;

struct NoteUpdate {
    Note updatedNote;
    int trackIndex;
    double currentPosition = -1;
    UpdateType type;
};

class AKJSMSequencerDSPKernel: public AKDSPKernel, public AKOutputBuffered {
public:
    
    AKJSMSequencerDSPKernel() {}
    ~AKJSMSequencerDSPKernel() {}
    
    void init(int _channels, double _sampleRate) override {
        AKDSPKernel::init(channels, sampleRate);
        TPCircularBufferInit(&circBuffer, 4096);
    }
    
    void setupMIDI(MIDIClientRef client, MIDIPortRef inputPort) {
        MIDISourceCreate(client, CFSTR(""), &outputSrc);
        MIDIObjectSetIntegerProperty(outputSrc, kMIDIPropertyPrivate, 1);
        MIDIPortConnectSource(inputPort, outputSrc, &inputPort);
    }
    
    void setABLinkRef(ABLLinkRef ref) {
        linkRef = ref;
    }
    
    void start() {
        if (linkRef != NULL) {
            if (ABLLinkIsConnected(linkRef)) {
                restarted = true;
            }
        }
        started = true;
    }
    
    void stop() {
        restarted = false;
        started = false;
        firstTimestamp = 0;
        beats = 0;
        timeToStart = 0;
        lastRenderTimestamp = 0;
    }
    
    void destroy() {
        MIDIEndpointDispose(outputSrc);
    }
    
    void reset() {
        resetted = true;
        firstTimestamp = 0;
        beats = 0;
    }
    
    void setTempo(double t) {
        tempo = t;
        tempoChanged = true;
    }
    
    void setRate(double r) {
        rate = r;
    }
    
    void setMeasureLatency(bool measure) {
        measuringLatency = measure;
    }
    
    void setOutputLatency(double l) {
        outputLatency = l;
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
    
    void processWithEvents(AudioTimeStamp const* timestamp,
                           AUAudioFrameCount frameCount,
                           AURenderEvent const* events)
    {
        processBuffer();
        
        if (!started) { return; }
        
        uint64_t latencyTicks = SESecondsToHostTicks(outputLatency);
        uint64_t hostTime = timestamp->mHostTime + latencyTicks;
        double frameSeconds = frameCount / 44100.0;
        
        ABLLinkTimelineRef timeLine = ABLLinkCaptureAudioTimeline(linkRef);
        
        if (firstTimestamp == 0) {
            firstTimestamp = timestamp->mHostTime;
            ABLLinkRequestBeatAtTime(timeLine, 0, hostTime, length);
        }
        
        if (tempoChanged) {
            tempoChanged = false;
            resetFirstTimestamp(beats, hostTime);
            setLinkTempo(tempo, hostTime, timeLine);
        }
        
        if (restarted) {
            if (timeToStart == 0) {
                timeToStart = 1;
                beats = 0;
                ABLLinkRequestBeatAtTime(timeLine, 0, hostTime, length);
            } else {
                uint64_t startt = ABLLinkTimeAtBeat(timeLine, 0, length);
                if (startt >= lastRenderTimestamp && startt <= timestamp->mHostTime) {
                    firstTimestamp = timestamp->mHostTime;
                    restarted = false;
                    timeToStart = 0;
                }
            }
        } else {
            double beat = SEHostTicksToBeats(hostTime - firstTimestamp, tempo);
            if (beats == 0) {
                beat = 0;
            } else {
                double lBeat = linkAdjustedBeat(beat, hostTime, timeLine);
                if (lBeat != beat) {
                    double missedBeats = beat;
                    beat = lBeat;
                    resetFirstTimestamp(beat, hostTime);
                    if (missedBeats < beat) {
                        playNotes(fmod(missedBeats, length), fmod(beat, length));
                    }
                }
            }
            
            double relativeLength = length / rate;
            double relativeBeat = fmod(beat, relativeLength);
            double frameBeats = SESecondsToBeats(frameSeconds, tempo);
            double endBeat = fmod(relativeBeat + frameBeats, length);
            playNotes(relativeBeat, endBeat);
            beats = beat + frameBeats;
        }
        
        lastRenderTimestamp = timestamp->mHostTime;

        if (measuringLatency) {
            playMIDINote(0, 0, timestamp->mHostTime);
        }
        
        ABLLinkCommitAudioTimeline(linkRef, timeLine);
    }
    
    void playNotes(double startBeat, double endBeat) {
        double relativeLength = length / rate;
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 64; j++) {
                double pos = tracks[i].notes[j].position / rate;
                
                if (pos == -1 || pos >= relativeLength) { continue; }
                
                if (pos >= startBeat && pos < endBeat) {
                    playMIDINote(i+36, tracks[i].notes[j].velocity, 0);
                } else if (endBeat < startBeat) {
                    if (pos >= startBeat || pos < endBeat) {
                        playMIDINote(i+36, tracks[i].notes[j].velocity, 0);
                    }
                }
            }
        }
    }
    
    void addSampler(AKJSMSamplerDSPKernel *samp) {
        sampler = samp;
    }
    
private:
    void addNote(Note note, int trackIndex) {
        int noteIndex = 0;
        for (int i = 0; i < 64; i++) {
            if (tracks[trackIndex].notes[i].position == -1)  {
                noteIndex = i;
                break;
            }
        }
        
        tracks[trackIndex].notes[noteIndex] = note;
    }
    
    void removeNote(Note note, int trackIndex) {
        for (int i = 0; i < 64; i++) {
            if (tracks[trackIndex].notes[i].position == note.position)  {
                tracks[trackIndex].notes[i].position = -1;
                break;
            }
        }
    }
    
    void moveNote(double currentPosition, Note note, int trackIndex) {
        for (int i = 0; i < 64; i++) {
            if (tracks[trackIndex].notes[i].position == currentPosition)  {
                tracks[trackIndex].notes[i].position = note.position;
                break;
            }
        }
    }
    
    void updateNote(Note note, int trackIndex) {
        for (int i = 0; i < 64; i++) {
            if (tracks[trackIndex].notes[i].position == note.position)  {
                tracks[trackIndex].notes[i] = note;
                break;
            }
        }
    }
    
    void clearSequence() {
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 64; j++) {
                tracks[i].notes[j].position = -1;
            }
        }
    }
    
    void setPosition(double pos) {
        beats = pos;
        tempoChanged = true;
    }
    
    void playMIDINote(uint8_t note, uint8_t velocity, uint64_t timestamp) {
        MIDIPacketList packetList;
        packetList.numPackets = 1;
        MIDIPacket* firstPacket = &packetList.packet[0];
        firstPacket->timeStamp = timestamp;
        firstPacket->length = 3;
        firstPacket->data[0] = 0x90;
        firstPacket->data[1] = note;
        firstPacket->data[2] = velocity;
        
        MIDIReceived(outputSrc, &packetList);
    }
    
    void processBuffer() {
        int32_t availableBytes;
        NoteUpdate *updates = (NoteUpdate*)TPCircularBufferTail(&circBuffer, &availableBytes);
        int total = availableBytes / sizeof(NoteUpdate);
        for (int i = 0; i < total; i++) {
            switch (updates[i].type) {
                case ADD:
                    addNote(updates[i].updatedNote, updates[i].trackIndex);
                    break;
                case MOVE:
                    moveNote(updates[i].currentPosition, updates[i].updatedNote, updates[i].trackIndex);
                    break;
                case REMOVE:
                    removeNote(updates[i].updatedNote, updates[i].trackIndex);
                    break;
                case UPDATE:
                    updateNote(updates[i].updatedNote, updates[i].trackIndex);
                    break;
                case CLEAR:
                    clearSequence();
                    break;
                case POSITION:
                    setPosition(updates[i].currentPosition);
                    break;
            }
        }
        
        TPCircularBufferConsume(&circBuffer, availableBytes);
    }
    
    double linkAdjustedBeat(double beat, uint64_t hostTime, ABLLinkTimelineRef timeline) {
        if (linkRef == NULL)return beat;
        if (ABLLinkIsEnabled(linkRef)) {
            double linkBeat = ABLLinkBeatAtTime(timeline, hostTime, length);
            if (fabs(linkBeat - beat) > 0.05) {
                return linkBeat;
            }
        }
        
        return beat;
    }
    
    void setLinkTempo(double tempo, uint64_t hostTime, ABLLinkTimelineRef timeline) {
        if (linkRef == NULL)return;
        if (ABLLinkIsEnabled(linkRef)) {
            ABLLinkSetTempo(timeline, tempo, hostTime);
        }
    }
    
    void resetFirstTimestamp(double beat, uint64_t hostTime) {
        uint64_t ticks = SEBeatsToHostTicks(beat, tempo);
        firstTimestamp = hostTime - ticks;
    }
    
public:
    bool started = false;
    bool resetted = false;
    double beats = 0.0;
    double length = 4.0;
    
    TPCircularBuffer circBuffer;
    
private:
    MIDIEndpointRef outputSrc;
    Track tracks[16];
    uint64_t firstTimestamp = 0;
    uint64_t lastRenderTimestamp = 0;
    bool tempoChanged = false;
    double tempo = 120.0;
    double rate = 1.0;
    ABLLinkRef linkRef;
    bool restarted = false;
    bool measuringLatency = false;
    double outputLatency = 0;
    
    int printThrottleCount = 0;
    
    uint64_t timeToStart = 0;
    
    AKJSMSamplerDSPKernel *sampler;
};
