//
//  SaveState.swift
//  PVLibrary
//
//  Created by Joseph Mattiello on 10/25/18.
//  Copyright © 2018 Provenance Emu. All rights reserved.
//

import Foundation

public protocol SaveStateInfoProvider {
    var id: String { get }
    var game: Game { get }
    var core: Core { get }
    var file: FileInfo { get }
    var date: Date { get }
    var lastOpened: Date? { get }
    var image: LocalFile? { get }
    var isAutosave: Bool { get }
    var isPinned: Bool { get }
    var isFavorite: Bool { get }
    var userDescription: String? { get }
}

public struct SaveState: SaveStateInfoProvider, Codable, Sendable, Identifiable {
    public let id: String
    public let game: Game
    public let core: Core
    public let file: FileInfo
    public let date: Date
    public let lastOpened: Date?
    public let image: LocalFile?
    public let isAutosave: Bool
    public let isPinned: Bool
    public let isFavorite: Bool
    public let userDescription: String?

    public init(id: String, game: Game, core: Core, file: FileInfo, date: Date, lastOpened: Date?, image: LocalFile?, isAutosave: Bool, isPinned: Bool = false, isFavorite: Bool = false, userDescription: String? = nil) {
        self.id = id
        self.game = game
        self.core = core
        self.file = file
        self.date = date
        self.lastOpened = lastOpened
        self.image = image
        self.isAutosave = isAutosave
        self.isPinned = isPinned
        self.isFavorite = isFavorite
        self.userDescription = userDescription
    }
    
    public init(from decoder: any Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        self.id = try container.decode(String.self, forKey: .id)
        self.game = try container.decode(Game.self, forKey: .game)
        self.core = try container.decode(Core.self, forKey: .core)
        self.file = try container.decode(FileInfo.self, forKey: .file)
        self.date = try container.decode(Date.self, forKey: .date)
        self.lastOpened = try container.decodeIfPresent(Date.self, forKey: .lastOpened)
        self.image = try container.decodeIfPresent(LocalFile.self, forKey: .image)
        self.isAutosave = try container.decode(Bool.self, forKey: .isAutosave)
        self.isPinned = try container.decode(Bool.self, forKey: .isPinned)
        self.isFavorite = try container.decode(Bool.self, forKey: .isFavorite)
        self.userDescription = try container.decodeIfPresent(String.self, forKey: .userDescription)

    }
}

extension SaveState: Equatable {
    public static func == (lhs: SaveState, rhs: SaveState) -> Bool {
        return lhs.id == rhs.id
    }
}

#if canImport(CoreTransferable)
import CoreTransferable
import UniformTypeIdentifiers
@available(iOS 16.0, macOS 13, tvOS 16.0, *)
extension SaveState: Transferable {
    public static var transferRepresentation: some TransferRepresentation {
        CodableRepresentation(contentType: .savestate)
    }
}
#endif
