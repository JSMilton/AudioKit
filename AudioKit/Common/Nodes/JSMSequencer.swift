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
    
    open dynamic var length = 4.0 {
        didSet {
            
        }
    }
    
    open dynamic var rate = 1.0 {
        didSet {
            
        }
    }
    
    open dynamic var tempo = 120.0 {
        didSet {
            
        }
    }
    
    open dynamic var isPlaying: Bool {
        get {
            return false
        }
    }
    
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
    
    public func getSeconds() -> Double {
        return internalAU?.seconds ?? 0
    }
    
    public func currentBeats() -> Double {
        return 0
    }
    
    public init(midiClient: MIDIClientRef) {
        
        _Self.register()
        
        super.init()
        AVAudioUnit._instantiate(with: _Self.ComponentDescription) { [weak self] avAudioUnit in
            
            self?.avAudioNode = avAudioUnit
            self?.internalAU = avAudioUnit.auAudioUnit as? AKAudioUnitType
        }
        
        internalAU?.midiClient = midiClient
    }
    
    func createTrack(withEndpoint endpoint: MIDIEndpointRef) {
        
    }
    
    func addNote(noteNumber: Int, with velocity: Int, at position: Double, to trackIndex: Int) {
        
    }
    
    func removeNote(at position: Double, from trackIndex: Int) {
        
    }
    
    func moveNote(at position: Double, by amount: Double, on trackIndex: Int) {
        
    }
}
