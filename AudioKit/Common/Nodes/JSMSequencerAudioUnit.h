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
@property (nonatomic) double beats;
@property (nonatomic) double tempo;

- (void)createTrackWithEndpoint:(MIDIEndpointRef)endpoint;
- (void)addNote:(int)noteNumber withVelocity:(int)velocity atPosition:(double)position toTrack:(int)trackIndex;
- (void)removeNoteAtPosition:(double)position fromTrack:(int)trackIndex;
- (void)doStartStuff;
@end
