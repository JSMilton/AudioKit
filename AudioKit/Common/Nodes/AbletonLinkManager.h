//
//  AbletonLinkManager.h
//  AudioKit For iOS
//
//  Created by Willis on 13/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

#import <UIKit/UIKit.h>

static NSString * const AbletonLinkGlobalTempoDidChangeNotification = @"";

@interface AbletonLinkManager : NSObject

- (UIViewController *)settingsViewController;
- (void*)getLinkRef;
- (void)createLinkWithTempo:(double)tempo;
- (double)getGlobalTempo;

+ (instancetype)shared;

@end
