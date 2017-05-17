//
//  JSMSamplerAudioUnit.h
//  AudioKit For iOS
//
//  Created by Jambo on 15/05/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "AKAudioUnit.h"

@interface JSMSamplerAudioUnit : AKAudioUnit

- (void)setupKernelAudioData:(AVAudioPCMBuffer *)buffer;
- (void *)getKernelPointer;

@end
