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
}

- (UInt64)lastTimestamp {
    return _kernel.lastTimestamp;
}

- (double)seconds {
    return _kernel.seconds;
}

- (void)setMidiClient:(MIDIClientRef)midiClient {
    _kernel.setMIDIClientRef(midiClient);
}

standardKernelPassthroughs()

- (void)createParameters
{
    standardSetup(JSMSequencer)
}

AUAudioUnitGeneratorOverrides(JSMSequencer)

@end
