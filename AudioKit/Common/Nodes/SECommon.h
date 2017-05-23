//
//  SECommon.h
//  The Spectacular Sync Engine
//
//  Created by Michael Tyson on 31/12/2014.
//  Copyright (c) 2015 A Tasty Pixel. All rights reserved.
//

#ifndef SECommon_h
#define SECommon_h

#ifdef __cplusplus
extern "C" {
#endif

#include <mach/mach_time.h>

/*!
 * Get current global timestamp, in host ticks
 */
uint64_t SECurrentTimeInHostTicks();

/*!
 * Get current global timestamp, in seconds
 */
double SECurrentTimeInSeconds();

/*!
 * Convert time in seconds to host ticks
 *
 * @param seconds The time in seconds
 * @return The time in host ticks
 */
uint64_t SESecondsToHostTicks(double seconds);

/*!
 * Convert time in host ticks to seconds
 *
 * @param ticks The time in host ticks
 * @return The time in seconds
 */
double SEHostTicksToSeconds(uint64_t ticks);

/*!
 * Convert seconds to beats (quarter notes)
 *
 * @param seconds The time in seconds
 * @param tempo The current tempo, in beats per minute
 * @return The time in beats for the given tempo
 */
double SESecondsToBeats(double seconds, double tempo);

/*!
 * Convert beats (quarter notes) to seconds
 *
 * @param beats The time in beats
 * @param tempo The current tempo, in beats per minute
 * @return The time in seconds
 */
double SEBeatsToSeconds(double beats, double tempo);

/*!
 * Convert host ticks to beats (quarter notes)
 *
 * @param ticks The time in host ticks
 * @param tempo The current tempo, in beats per minute
 * @return The time in beats for the given tempo
 */
double SEHostTicksToBeats(uint64_t ticks, double tempo);

/*!
 * Convert beats (quarter notes) to host ticks
 *
 * @param beats The time in beats
 * @param tempo The current tempo, in beats per minute
 * @return The time in host ticks
 */
uint64_t SEBeatsToHostTicks(double beats, double tempo);
    
#ifdef __cplusplus
}
#endif
    
#endif