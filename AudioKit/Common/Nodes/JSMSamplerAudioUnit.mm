//
//  JSMSamplerAudioUnit.m
//  AudioKit For iOS
//
//  Created by Jambo on 15/05/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "JSMSamplerAudioUnit.h"
#import "BufferedAudioBus.hpp"
#import "AKJSMSamplerDSPKernel.hpp"
#import <AudioKit/AudioKit-Swift.h>

@implementation JSMSamplerAudioUnit
{
    AKJSMSamplerDSPKernel _kernel;
    BufferedInputBus _inputBus;
}

standardKernelPassthroughs()

- (void)createParameters
{
    standardSetup(JSMSampler)
}

- (void)setupKernelAudioData:(AVAudioPCMBuffer *)buffer
{
    _kernel.setSampleData(buffer);
}

- (void *)getKernelPointer
{
    return &_kernel;
}

AUAudioUnitGeneratorOverrides(JSMSampler)

@end
