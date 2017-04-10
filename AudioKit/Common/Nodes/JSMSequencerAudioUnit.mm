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

- (void)createTrackWithEndpoint:(MIDIEndpointRef)endpoint
{
    _kernel.createTrack(trackCount, endpoint);
    trackCount++;
}

- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex
{
    _kernel.addNote(noteNumber, velocity, position, trackIndex);
}

- (void)doStartStuff
{
    trackCount = 0;
}

standardKernelPassthroughs()

- (void)createParameters
{
    standardSetup(JSMSequencer)
}

AUAudioUnitGeneratorOverrides(JSMSequencer)

@end
