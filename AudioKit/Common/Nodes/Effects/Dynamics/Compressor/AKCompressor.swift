//
//  AKCompressor.swift
//  AudioKit
//
//  Created by Jeff Cooper, revision history on Github.
//  Copyright © 2017 Aurelius Prochazka. All rights reserved.
//

/// AudioKit Compressor based on Apple's DynamicsProcessor Audio Unit
///
open class AKCompressor: AKNode, AKToggleable, AUEffect {
    public static let ComponentDescription = AudioComponentDescription(appleEffect: kAudioUnitSubType_DynamicsProcessor)

    internal var internalEffect = AVAudioUnitEffect()
    internal var internalAU: AudioUnit?

    /// Threshold (dB) ranges from -40 to 20 (Default: -20)
    open dynamic var threshold: Double = -20 {
        didSet {
            threshold = (-40...20).clamp(threshold)
            if let audioUnit = internalAU {
                AudioUnitSetParameter(audioUnit,
                                      kDynamicsProcessorParam_Threshold,
                                      kAudioUnitScope_Global,
                                      0,
                                      Float(threshold),
                                      0)
            }
        }
    }

    /// Head Room (dB) ranges from 0.1 to 40.0 (Default: 5)
    open dynamic var headRoom: Double = 5 {
        didSet {
            headRoom = (0.1...40).clamp(headRoom)
            if let audioUnit = internalAU {
                AudioUnitSetParameter(audioUnit,
                                      kDynamicsProcessorParam_HeadRoom,
                                      kAudioUnitScope_Global,
                                      0,
                                      Float(headRoom),
                                      0)
            }
        }
    }

    /// Attack Time (secs) ranges from 0.0001 to 0.2 (Default: 0.001)
    open dynamic var attackTime: Double = 0.001 {
        didSet {
            attackTime = (0.000_1...0.2).clamp(attackTime)
            if let audioUnit = internalAU {
                AudioUnitSetParameter(audioUnit,
                                      kDynamicsProcessorParam_AttackTime,
                                      kAudioUnitScope_Global,
                                      0,
                                      Float(attackTime),
                                      0)
            }
        }
    }

    /// Release Time (secs) ranges from 0.01 to 3 (Default: 0.05)
    open dynamic var releaseTime: Double = 0.05 {
        didSet {
            releaseTime = (0.01...3).clamp(releaseTime)
            if let audioUnit = internalAU {
                AudioUnitSetParameter(audioUnit,
                                      kDynamicsProcessorParam_ReleaseTime,
                                      kAudioUnitScope_Global,
                                      0,
                                      Float(releaseTime),
                                      0)
            }
        }
    }

    /// Compression Amount (dB) read only
    open dynamic var compressionAmount: Double {
        return 0// au[kDynamicsProcessorParam_CompressionAmount]
    }

    /// Input Amplitude (dB) read only
    open dynamic var inputAmplitude: Double {
        return 0// au[kDynamicsProcessorParam_InputAmplitude]
    }

    /// Output Amplitude (dB) read only
    open dynamic var outputAmplitude: Double {
        return 0// au[kDynamicsProcessorParam_OutputAmplitude]
    }

    /// Master Gain (dB) ranges from -40 to 40 (Default: 0)
    open dynamic var masterGain: Double = 0 {
        didSet {
            masterGain = (-40...40).clamp(masterGain)
            if let audioUnit = internalAU {
                AudioUnitSetParameter(audioUnit,
                                      kDynamicsProcessorParam_MasterGain,
                                      kAudioUnitScope_Global,
                                      0,
                                      Float(masterGain),
                                      0)
            }
        }
    }

    /// Dry/Wet Mix (Default 100)
    open dynamic var dryWetMix: Double = 100 {
        didSet {
            dryWetMix = (0...100).clamp(dryWetMix)
            inputGain?.volume = 1 - dryWetMix / 100
            effectGain?.volume = dryWetMix / 100
        }
    }

    fileprivate var lastKnownMix: Double = 100
    fileprivate var inputGain: AKMixer?
    fileprivate var effectGain: AKMixer?

    /// Tells whether the node is processing (ie. started, playing, or active)
    open dynamic var isStarted = true

    /// Initialize the dynamics processor node
    ///
    /// - Parameters:
    ///   - input: Input node to process
    ///   - threshold: Threshold (dB) ranges from -40 to 20 (Default: -20)
    ///   - headRoom: Head Room (dB) ranges from 0.1 to 40.0 (Default: 5)
    ///   - attackTime: Attack Time (secs) ranges from 0.0001 to 0.2 (Default: 0.001)
    ///   - releaseTime: Release Time (secs) ranges from 0.01 to 3 (Default: 0.05)
    ///   - masterGain: Master Gain (dB) ranges from -40 to 40 (Default: 0)
    ///
    public init(
        _ input: AKNode?,
        threshold: Double = -20,
        headRoom: Double = 5,
        attackTime: Double = 0.001,
        releaseTime: Double = 0.05,
        masterGain: Double = 0) {
        
        internalEffect = AVAudioUnitEffect(audioComponentDescription: AKCompressor.ComponentDescription)
        
        super.init()
        self.avAudioNode = internalEffect
        AudioKit.engine.attach(self.avAudioNode)
        input?.addConnectionPoint(self)
        internalAU = internalEffect.audioUnit
        
        self.threshold = threshold
        self.headRoom = headRoom
        self.attackTime = attackTime
        self.releaseTime = releaseTime
        self.masterGain = masterGain
    }

    /// Function to start, play, or activate the node, all do the same thing
    open func start() {
        if isStopped {
            dryWetMix = lastKnownMix
            isStarted = true
        }
    }

    /// Function to stop or bypass the node, both are equivalent
    open func stop() {
        if isPlaying {
            lastKnownMix = dryWetMix
            dryWetMix = 0
            isStarted = false
        }
    }
}
