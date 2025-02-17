//
//  ShaderManager.swift
//  Provenance
//
//  Created by Joseph Mattiello on 7/13/22.
//  Copyright © 2022 Provenance Emu. All rights reserved.
//

import Foundation
import PVSettings
import PVPrimitives

@objc
public enum ShaderType: UInt, Codable {
    case blitter
    case filter
    case vertex
}

@objc public final class Shader: NSObject, Codable {
    @objc public let type: ShaderType
    @objc public let name: String
    @objc public let function: String

    @objc
    public init(type: ShaderType, name: String, function: String) {
        self.type = type
        self.name = name
        self.function = function
    }
}

public protocol ShaderProvider {
    var shaders: [Shader] { get }
}

@objc
public final class MetalShaderManager: NSObject, ShaderProvider {

    @objc(sharedInstance)
    public static let shared: MetalShaderManager = MetalShaderManager()

    @objc
    public var shaders: [Shader] = {
        let shaders: [Shader] = [
            .init(type: .vertex, name: "Fullscreen", function: "fullscreen_vs"),
            .init(type: .blitter, name: "Blitter", function: "blit_ps"),
            .init(type: .filter, name: "Complex CRT", function: "crt_filter_ps"),
            .init(type: .filter, name: "Simple CRT", function: "simpleCRT"),
            .init(type: .filter, name: "LCD", function: "lcdFilter"),
            .init(type: .filter, name: "Line Tron", function: "lineTron"),
            .init(type: .filter, name: "Mega Tron", function: "megaTron"),
            .init(type: .filter, name: "ulTron", function: "ultron"),
            .init(type: .filter, name: "Game Boy", function: "gameBoyFilter"),
            .init(type: .filter, name: "VHS", function: "vhsFilter")
        ]
        ILOG("Registered shaders: \(shaders.map { $0.name })")
        return shaders
    }()

    @objc public
    lazy var vertexShaders: [Shader] = {
        return shaders(forType: .vertex)
    }()

    @objc public
    lazy var filterShaders: [Shader] = {
        return shaders(forType: .filter)
    }()

    @objc public
    lazy var blitterShaders: [Shader] = {
        return shaders(forType: .blitter)
    }()

    @objc
    public func filterShader(forName name: String) -> Shader? {
        return filterShaders.first(where: { $0.name == name })
    }

    public func filterShader(forOption option: MetalFilterModeOption, screenType: ScreenTypeObjC) -> Shader? {
        switch option {
        case .none:
            return nil
        case .always(let filter):
            return filterShaders.first(where: { $0.name == filter.description })
        case .auto(let crt, let lcd):
            switch screenType {
            case .colorLCD, .monochromaticLCD:
                return filterShaders.first(where: { $0.name == lcd.description })
            case .dotMatrix:
                return filterShaders.first(where: { $0.name == "Game Boy" })
            case .crt:
                return filterShaders.first(where: { $0.name == crt.description })
            case .modern, .unknown:
                return nil
            }
        }
    }

    private
    func shaders(forType type: ShaderType) -> [Shader] {
        return shaders.filter { $0.type == type }
    }
}
