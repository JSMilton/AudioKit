//
//  JSMSequencer.swift
//  AudioKit For iOS
//
//  Created by James Milton on 09/04/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

open class JSMSequencer: AKNode, AKToggleable, AKComponent {
    public typealias AKAudioUnitType = JSMSequencerAudioUnit
    public static let ComponentDescription = AudioComponentDescription(generator: "sequ")
    
    private var internalAU: AKAudioUnitType?
    
    public var isStarted: Bool {
        return internalAU?.isPlaying() ?? false
    }
    
    public func start() {
        internalAU?.start()
    }
    
    public func stop() {
        internalAU?.stop()
    }
    
    public func getTime() -> UInt64 {
        return internalAU?.lastTimestamp ?? 0
    }
    
    public override init() {
        
        _Self.register()
        
        super.init()
        AVAudioUnit._instantiate(with: _Self.ComponentDescription) { [weak self] avAudioUnit in
            
            self?.avAudioNode = avAudioUnit
            self?.internalAU = avAudioUnit.auAudioUnit as? AKAudioUnitType
        }
    }
    
}
