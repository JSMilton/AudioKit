//
//  JSMSequencerAudioUnit.h
//  AudioKit For iOS
//
//  Created by James Milton on 09/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "AKAudioUnit.h"
#import <CoreMIDI/CoreMIDI.h>

@interface JSMSequencerAudioUnit : AKAudioUnit
@property (nonatomic) MIDIClientRef midiClient;
@property (readonly) double beats;
@property (nonatomic) double tempo;
@property (nonatomic) double length;
@property (nonatomic) double rate;

- (void)createTrackWithEndpoint:(MIDIEndpointRef)endpoint;
- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex;
- (void)removeNoteAtPosition:(double)position fromTrack:(int)trackIndex;
- (void)updateVelocityAtPosition:(double)position toVelocity:(int)velocity onTrack:(int)trackIndex;
- (void)moveNoteAtPosition:(double)position byAmount:(double)amount onTrack:(int)trackIndex;
- (void)doStartStuff;
@end
