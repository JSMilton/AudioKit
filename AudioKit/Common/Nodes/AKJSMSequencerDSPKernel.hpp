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
        
        if (firstTimestamp == 0) {
            firstTimestamp = timestamp->mHostTime;
        }
        
        if (tempoChanged) {
            tempoChanged = false;
            uint64_t ticks = SEBeatsToHostTicks(beats, tempo);
            firstTimestamp = timestamp->mHostTime - ticks;
            setLinkTempo(tempo, timestamp->mHostTime);
        }

        double beat = SEHostTicksToBeats(timestamp->mHostTime - firstTimestamp, tempo);
        double lBeat = linkAdjustedBeat(beat, timestamp->mHostTime);
        
        if (lBeat != beat) {
            beat = lBeat;
            uint64_t ticks = SEBeatsToHostTicks(beat, tempo);
            //firstTimestamp = timestamp->mHostTime - ticks;
        }
        
        if (restarted) {
            if (fabs(fmod(beat, length) - length) > 0.1) {
                return;
            }
            restarted = false;
        }
        
        //double adjustedBeat = SEHostTicksToBeats(lastRenderTimestamp - firstTimestamp, tempo);
        double relativeLength = length / rate;
        double relativeBeat = fmod(beat, relativeLength);
        double frameSeconds = frameCount / 44100.0;
        double frameBeats = SESecondsToBeats(frameSeconds, tempo);
        double endBeat = relativeBeat + frameBeats;
        
        if (lastRenderTimestamp > 0) {
            //playNotes(fmod(adjustedBeat, relativeLength), relativeBeat, timestamp->mHostTime);
        }
        
        playNotes(relativeBeat, endBeat, timestamp->mHostTime);
        
        beats = beat + frameBeats;
        
        lastRenderTimestamp = timestamp->mHostTime;
    }
    
    void playNotes(double startBeat, double endBeat, uint64_t timestamp) {
        
        double relativeLength = length / rate;
        double endOffset = floor(endBeat / relativeLength) * relativeLength;
        double offset = floor(startBeat / relativeLength) * relativeLength;
        
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 64; j++) {
                double pos = tracks[i].notes[j].position / rate;
                
                if (pos == -1 || pos >= relativeLength) { continue; }
                
                if (pos >= startBeat && pos < endBeat) {
                    playMIDINote(i+36, tracks[i].notes[j].velocity, timestamp);
                } else if (endOffset > offset) {
                    double r = fmod(endBeat, relativeLength);
                    if (pos >= 0 && pos < r) {
                        playMIDINote(i+36, tracks[i].notes[j].velocity, timestamp);
                    }
                }
            }
        }
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
    
    double linkAdjustedBeat(double beat, uint64_t hostTime) {
        if (linkRef == NULL)return beat;
        if (ABLLinkIsEnabled(linkRef)) {
            ABLLinkTimelineRef timeLine = ABLLinkCaptureAudioTimeline(linkRef);
            double linkBeat = fmod(ABLLinkBeatAtTime(timeLine, hostTime, length), length);
            if (fabs(linkBeat - fmod(beat, length)) > 0.01) {
                
                printThrottleCount++;
                //if (printThrottleCount == 10) {
                    printf("linkbeat: %f, mybeat: %f\n", fmod( linkBeat, length ), fmod( beat, length ));
                    printThrottleCount = 0;
                //}
                
                return linkBeat;
            }
        }
        
        return beat;
    }
    
    void setLinkTempo(double tempo, uint64_t hostTime) {
        if (linkRef == NULL)return;
        if (ABLLinkIsEnabled(linkRef)) {
            ABLLinkTimelineRef timeLine = ABLLinkCaptureAudioTimeline(linkRef);
            ABLLinkSetTempo(timeLine, tempo, hostTime);
            ABLLinkCommitAudioTimeline(linkRef, timeLine);
        }
    }
    
    void resetFirstTimestamp() {
        
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
    
    int printThrottleCount = 0;
};
