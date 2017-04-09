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
@property (nonatomic) UInt64 lastTimestamp;
@property (nonatomic) double seconds;
@property (nonatomic) MIDIClientRef midiClient;
@property (nonatomic) MIDIEndpointRef ref;

- (void)blahMIDI;
@end
