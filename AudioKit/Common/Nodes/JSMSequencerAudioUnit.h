//
//  JSMSequencerAudioUnit.h
//  AudioKit For iOS
//
//  Created by James Milton on 09/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "AKAudioUnit.h"
#import <CoreMIDI/CoreMIDI.h>

@class JSMSamplerAudioUnit;

@interface JSMSequencerAudioUnit : AKAudioUnit
@property (nonatomic) MIDIClientRef midiClient;
@property (nonatomic) MIDIPortRef inputPort;
@property (readonly) double beats;
@property (nonatomic) double tempo;
@property (nonatomic) double length;
@property (nonatomic) double rate;
@property (nonatomic) double outputLatency;
@property (nonatomic) BOOL measureLatency;

- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex;
- (void)removeNoteAtPosition:(double)position fromTrack:(int)trackIndex;
- (void)updateVelocityAtPosition:(double)position toVelocity:(int)velocity onTrack:(int)trackIndex;
- (void)moveNoteAtPosition:(double)position byAmount:(double)amount onTrack:(int)trackIndex;
- (void)clearSequence;
- (void)setPosition:(double)position;
- (void)doStartStuff;
- (void)addSampler:(JSMSamplerAudioUnit *)sampler;
@end
