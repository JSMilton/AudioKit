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
    ABLLinkSetSessionTempoCallback(_linkRef, tempoCallback, NULL);
    ABLLinkSetIsEnabledCallback(_linkRef, isConnectedCallback, NULL);
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

- (BOOL)isLinkEnabled
{
    return ABLLinkIsEnabled(_linkRef);
}

- (ABLLinkSettingsViewController *)settingsViewController
{
    return [ABLLinkSettingsViewController instance:_linkRef];
}

static void tempoCallback(double tempo, void *context) {
    [[NSNotificationCenter defaultCenter] postNotificationName:AbletonLinkGlobalTempoDidChangeNotification object:nil];
}

static void isConnectedCallback(bool isConnected, void *context) {
    if (isConnected) {
        [[NSNotificationCenter defaultCenter] postNotificationName:AbletonLinkEnabledNotification object:nil];
    } else {
        [[NSNotificationCenter defaultCenter] postNotificationName:AbletonLinkDisabledNotification object:nil];
    }
}

@end
