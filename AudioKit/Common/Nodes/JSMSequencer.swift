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
            internalAU?.length = length
        }
    }
    
    open dynamic var rate = 1.0 {
        didSet {
            internalAU?.rate = rate
        }
    }
    
    open dynamic var tempo = 120.0 {
        didSet {
            internalAU?.tempo = tempo
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
    
    public func currentBeats() -> Double {
        return internalAU?.beats ?? 0
    }
    
    public func currentRelativeBeats() -> Double {
        return currentBeats().truncatingRemainder(dividingBy: length)
    }
    
    public init(midiClient: MIDIClientRef) {
        
        _Self.register()
        
        super.init()
        AVAudioUnit._instantiate(with: _Self.ComponentDescription) { [weak self] avAudioUnit in
            
            self?.avAudioNode = avAudioUnit
            self?.internalAU = avAudioUnit.auAudioUnit as? AKAudioUnitType
        }
        
        internalAU?.doStartStuff()
        internalAU?.midiClient = midiClient
    }
    
    public func createTrack(withEndpoint endpoint: MIDIEndpointRef) {
        internalAU?.createTrack(withEndpoint: endpoint)
    }
    
    public func addNote(noteNumber: Int, with velocity: Int, at position: Double, to trackIndex: Int) {
        internalAU?.addNote(Int32(noteNumber), withVelocity: Int32(velocity), atPosition: position, toTrack: Int32(trackIndex))
    }
    
    public func removeNote(at position: Double, from trackIndex: Int) {
        internalAU?.removeNote(atPosition: position, fromTrack: Int32(trackIndex))
    }
    
    public func moveNote(at position: Double, by amount: Double, on trackIndex: Int) {
        internalAU?.moveNote(atPosition: position, byAmount: amount, onTrack: Int32(trackIndex))
    }
    
    public func updateVelocity(at position: Double, to velocity: Int, on trackIndex: Int) {
        internalAU?.updateVelocity(atPosition: position, toVelocity: Int32(velocity), onTrack: Int32(trackIndex))
    }
    
    public func clear() {
        internalAU?.clearSequence()
    }
}
