//
//  JSMSampler.swift
//  AudioKit For iOS
//
//  Created by Jambo on 15/05/2017.
//  Copyright © 2017 AudioKit. All rights reserved.
//

open class JSMSampler: AKNode, AKToggleable, AKComponent {
    
    public typealias AKAudioUnitType = JSMSamplerAudioUnit
    public static let ComponentDescription = AudioComponentDescription(generator: "samp")
    
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
    
    public init(fileName: String) {
        
        _Self.register()
        
        super.init()
        
        let file = try! AKAudioFile(readFileName: fileName)
        
        AVAudioUnit._instantiate(with: _Self.ComponentDescription) { [weak self] avAudioUnit in
            self?.avAudioNode = avAudioUnit
            self?.internalAU = avAudioUnit.auAudioUnit as? AKAudioUnitType
            self?.internalAU?.setupKernelAudioData(file.pcmBuffer)
        }
    }
    
    func getInternalAU() -> AKAudioUnitType? {
        return internalAU
    }
}
