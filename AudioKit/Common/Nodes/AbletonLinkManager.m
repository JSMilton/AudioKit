//
//  AbletonLinkManager.m
//  AudioKit For iOS
//
//  Created by Willis on 13/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import "AbletonLinkManager.h"
#include "ABLLink.h"
#include "ABLLinkSettingsViewController.h"

@implementation AbletonLinkManager
{
    ABLLinkRef _linkRef;
}

+ (instancetype)shared
{
    static AbletonLinkManager *shared;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [[AbletonLinkManager alloc] init];
    });
    return shared;
}

- (void)createLinkWithTempo:(double)tempo
{
    _linkRef = ABLLinkNew(tempo);
    ABLLinkSetSessionTempoCallback(_linkRef, tempoCallback, (__bridge void *)(self));
}

- (void *)getLinkRef
{
    if (_linkRef == NULL) {
        NSLog(@"Warning! NULL link reference. This method isn't returning anything.");
    }
    return _linkRef;
}

- (double)getGlobalTempo
{
    ABLLinkTimelineRef timeRef = ABLLinkCaptureAppTimeline(_linkRef);
    return ABLLinkGetTempo(timeRef);
}

- (ABLLinkSettingsViewController *)settingsViewController
{
    return [ABLLinkSettingsViewController instance:_linkRef];
}

static void tempoCallback(double tempo, void *context) {
    [[NSNotificationCenter defaultCenter] postNotificationName:AbletonLinkGlobalTempoDidChangeNotification object:nil];
}

@end
