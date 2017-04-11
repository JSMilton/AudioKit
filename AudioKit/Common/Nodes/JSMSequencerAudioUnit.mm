//
//  JSMSequencerAudioUnit.m
//  AudioKit For iOS
//
//  Created by James Milton on 09/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "JSMSequencerAudioUnit.h"
#import "AKJSMSequencerDSPKernel.hpp"
#import "BufferedAudioBus.hpp"
#import <AudioKit/AudioKit-Swift.h>

@implementation JSMSequencerAudioUnit
{
    AKJSMSequencerDSPKernel _kernel;
    BufferedInputBus _inputBus;
    int trackCount;
    TPCircularBuffer *circBuffer;
}

- (double)beats {
    return _kernel.beats;
}

- (void)setMidiClient:(MIDIClientRef)midiClient {
    _kernel.setMIDIClientRef(midiClient);
}

- (void)setTempo:(double)tempo {
    _tempo = tempo;
    _kernel.tempo = tempo;
}

- (void)setLength:(double)length {
    _length = length;
    _kernel.length = length;
}

- (void)setRate:(double)rate {
    _rate = rate;
    _kernel.rate = rate;
}

- (void)createTrackWithEndpoint:(MIDIEndpointRef)endpoint
{
    _kernel.createTrack(trackCount, endpoint);
    trackCount++;
}

- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex
{
    NoteUpdate update;
    update.updatedNote.noteNumber = noteNumber;
    update.updatedNote.velocity = velocity;
    update.updatedNote.position = position;
    update.currentPosition = position;
    update.trackIndex = trackIndex;
    update.type = ADD;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)removeNoteAtPosition:(double)position fromTrack:(int)trackIndex
{
    NoteUpdate update;
    update.updatedNote.position = position;
    update.trackIndex = trackIndex;
    update.type = REMOVE;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)moveNoteAtPosition:(double)position byAmount:(double)amount onTrack:(int)trackIndex
{
    NoteUpdate update;
    update.updatedNote.position = position + amount;
    update.currentPosition = position;
    update.trackIndex = trackIndex;
    update.type = MOVE;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)doStartStuff
{
    trackCount = 0;
    circBuffer = &_kernel.circBuffer;
}

standardKernelPassthroughs()

- (void)createParameters
{
    standardSetup(JSMSequencer)
}

AUAudioUnitGeneratorOverrides(JSMSequencer)

@end
