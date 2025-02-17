//
//  PVVisualBoyAdvance.swift
//  PVVisualBoyAdvance
//
//  Created by Joseph Mattiello on 5/30/24.
//  Copyright © 2024 Provenance EMU. All rights reserved.
//

import Foundation
#if canImport(GameController)
import GameController
#endif
import PVCoreBridge
import PVLogging
import PVAudio
import PVEmulatorCore
import PVVisualBoyAdvanceOptions
import PVVisualBoyAdvanceBridge
import libvisualboyadvance

@objc
@objcMembers
open class PVVisualBoyAdvanceCore: PVEmulatorCore {
//
//    // MARK: Controller
//#if canImport(GameController)
//    public var valueChangedHandler: GCExtendedGamepadValueChangedHandler? = nil
//#endif
//
//    // MARK: Cheats
//    public var cheats: NSMutableArray = .init()
//
//    public var supportsCheatCode: Bool { true }
//
//    // MARK: Buffers
//
//    public var _videoBuffer: UnsafeMutablePointer<UInt8>? = nil // uint8_t *videoBuffer;
//    public var _soundBufer: UnsafeMutablePointer<UInt32>? = nil // int32_t *soundBuffer;
//
//    // MARK: Video
//
//    public var _videoWidth: Int = 0
//    public var _videoHeight: Int = 0
//
//    public var _frameInterval: TimeInterval = 0
//
//    // MARK: Audio
//
//    public var _sampleRate: Double = 44100.0
//
//    // MARK: VisualBoyAdvance ObjC
//
//    public var _romFile: URL? = nil
//    public var _saveFile: URL? = nil
//
//    public var _romID: String? = nil
//
//    public var _enableRTC: Bool = false
//    public var _enableMirroring: Bool = false
//    public var _useBIOS: Bool = false
//    public var _haveFrame: Bool = false
//    public var _migratingSave: Bool = false
//
//    public var _flashSize: Int = 0
//    public var _cpuSaveType: Int = 0

    // MARK: Lifecycle
    
    let _bridge: PVVisualBoyAdvanceBridge = .init()

    public required init() {
        super.init()
        self.bridge = (_bridge as! any ObjCBridgedCoreBridge)
    }
}

extension PVVisualBoyAdvanceCore: CoreOptional {
    public static var options: [CoreOption] {
        return VisualBoyAdvanceOptions.options
    }
}

extension PVVisualBoyAdvanceCore: PVGBASystemResponderClient {
    public func didPush(_ button: PVCoreBridge.PVGBAButton, forPlayer player: Int) {
        (_bridge as! PVGBASystemResponderClient).didPush(button, forPlayer: player)
    }
    
    public func didRelease(_ button: PVCoreBridge.PVGBAButton, forPlayer player: Int) {
        (_bridge as! PVGBASystemResponderClient).didRelease(button, forPlayer: player)
    }
}
