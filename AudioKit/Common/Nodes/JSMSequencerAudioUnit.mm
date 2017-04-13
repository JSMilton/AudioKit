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
#import "AbletonLinkManager.h"

@implementation JSMSequencerAudioUnit
{
    AKJSMSequencerDSPKernel _kernel;
    BufferedInputBus _inputBus;
    TPCircularBuffer *circBuffer;
}

- (double)beats {
    return _kernel.beats;
}

- (void)setTempo:(double)tempo {
    _tempo = tempo;
    _kernel.setTempo(tempo);
}

- (void)setLength:(double)length {
    _length = length;
    _kernel.length = length;
}

- (void)setRate:(double)rate {
    _rate = rate;
    _kernel.setRate(rate);
}

- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex
{
    NoteUpdate update;
    update.updatedNote.noteNumber = noteNumber;
    update.updatedNote.velocity = velocity;
    update.updatedNote.position = position;
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

- (void)updateVelocityAtPosition:(double)position toVelocity:(int)velocity onTrack:(int)trackIndex
{
    NoteUpdate update;
    update.updatedNote.position = position;
    update.updatedNote.velocity = velocity;
    update.trackIndex = trackIndex;
    update.type = UPDATE;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)clearSequence
{
    NoteUpdate update;
    update.type = CLEAR;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)setPosition:(double)position
{
    NoteUpdate update;
    update.type = POSITION;
    update.currentPosition = position;
    TPCircularBufferProduceBytes(circBuffer, &update, sizeof(NoteUpdate));
}

- (void)doStartStuff
{
    circBuffer = &_kernel.circBuffer;
    _kernel.setABLinkRef((ABLLinkRef)[[AbletonLinkManager shared] getLinkRef]);
    _kernel.setupMIDI(self.midiClient, self.inputPort);
}

standardKernelPassthroughs()

- (void)createParameters
{
    standardSetup(JSMSequencer)
}

AUAudioUnitGeneratorOverrides(JSMSequencer)

@end
