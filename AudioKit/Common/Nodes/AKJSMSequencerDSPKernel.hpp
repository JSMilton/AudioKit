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
    CLEAR
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
    
    void setMIDIClientRef(MIDIClientRef client) {
        MIDIOutputPortCreate(client, CFSTR("Deep-808 Internal"), &outputPort);
    }
    
    void start() {
        started = true;
    }
    
    void stop() {
        started = false;
        firstTimestamp = 0;
    }
    
    void destroy() {
        MIDIPortDispose(outputPort);
    }
    
    void reset() {
        resetted = true;
    }
    
    void setTempo(double t) {
        tempo = t;
        tempoChanged = true;
    }
    
    void setRate(double r) {
        rate = r;
        tempoChanged = true;
    }
    
    void createTrack(int trackIndex, MIDIEndpointRef endpoint) {
        tracks[trackIndex].endpointRef = endpoint;
    }
    
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
    
    void playMIDINote(uint8_t note, uint8_t velocity, MIDIEndpointRef ref) {
        MIDIPacketList packetList;
        packetList.numPackets = 1;
        MIDIPacket* firstPacket = &packetList.packet[0];
        firstPacket->timeStamp = 0;
        firstPacket->length = 3;
        firstPacket->data[0] = 0x90;
        firstPacket->data[1] = note;
        firstPacket->data[2] = velocity;
        
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
                case CLEAR:
                    clearSequence();
                    break;
            }
        }
        
        TPCircularBufferConsume(&circBuffer, availableBytes);
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
        
        double relativeTempo = tempo * rate;
        
        if (tempoChanged) {
            tempoChanged = false;
            
            // get the beat value relative to the start of the loop,
            // use the stored beats value as it's unaffected by the new tempo
            double relativeBeat = fmod(beats, length);
            
            // convert this relative value into host ticks. this will be the new firstTimeStamp.
            // this will maintain the current beat position, whilst allowing the rest of the sequence to
            // run at the new tempo
            uint64_t ticks = SEBeatsToHostTicks(relativeBeat, relativeTempo);
            firstTimestamp = timestamp->mHostTime - ticks;
        }

        double beat = SEHostTicksToBeats(timestamp->mHostTime - firstTimestamp, relativeTempo);
        double frameSeconds = frameCount / 44100.0;
        double endBeat = beat + SESecondsToBeats(frameSeconds, relativeTempo);
        double endOffset = floor(endBeat / length) * length;
        double offset = floor(beat / length) * length;
        beats = endBeat;
        
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 64; j++) {
                double pos = tracks[i].notes[j].position;
                
                if (pos == -1) { continue; }
                
                pos += offset;
                
                if (pos >= beat && pos < endBeat) {
                    playMIDINote(tracks[i].notes[j].noteNumber, tracks[i].notes[j].velocity, tracks[i].endpointRef);
                } else if (endOffset > offset) {
                    pos += length;
                    if (pos >= beat && pos < endBeat) {
                        playMIDINote(tracks[i].notes[j].noteNumber, tracks[i].notes[j].velocity, tracks[i].endpointRef);
                    }
                }
            }
        }
    }
    
public:
    bool started = false;
    bool resetted = false;
    double beats = 0.0;
    double length = 4.0;
    
    TPCircularBuffer circBuffer;
    
private:
    MIDIPortRef outputPort;
    Track tracks[16];
    uint64_t firstTimestamp = 0;
    bool tempoChanged = false;
    double tempo = 120.0;
    double rate = 1.0;
};
