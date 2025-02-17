import Foundation
import PVLogging
import Lighter
import ROMMetadataProvider
import PVLookupTypes
import PVSystems

public final class ShiraGame: ROMMetadataProvider, @unchecked Sendable {
    private var db: ShiragameSchema
    private let initializer: DatabaseInitializer
    private lazy var initializationTask: Task<Void, Error> = self.createInitializationTask()
    private let timeout: TimeInterval = 30  // 30 second timeout

    private actor DatabaseInitializer {
        var db: ShiragameSchema

        init(initialDB: ShiragameSchema) {
            self.db = initialDB
        }

        func updateDB(with newDB: ShiragameSchema) {
            self.db = newDB
        }

        func getDB() -> ShiragameSchema {
            return db
        }
    }

    private func createInitializationTask() -> Task<Void, Error> {
        return Task { @MainActor in
            try await ShiraGameManager.shared.prepareDatabaseIfNeeded()
            let realDB = ShiragameSchema(url: ShiraGameManager.shared.databasePath)
            await self.initializer.updateDB(with: realDB)
            self.db = realDB
        }
    }

    public init() async throws {
        DLOG("ShiraGame: Starting initialization...")

        // Initialize with empty database first
        let emptyDB = ShiragameSchema(url: URL(fileURLWithPath: ""))
        self.db = emptyDB
        self.initializer = DatabaseInitializer(initialDB: emptyDB)

        // Wait for database preparation to complete
        DLOG("ShiraGame: Waiting for database preparation...")
        try await ShiraGameManager.shared.prepareDatabaseIfNeeded()

        // Now initialize with real database
        DLOG("ShiraGame: Initializing with prepared database...")
        self.db = ShiragameSchema(url: ShiraGameManager.shared.databasePath)

        DLOG("ShiraGame: Initialization complete")
    }

    // Helper to wait for initialization
    private func awaitInitialization() async throws {
        try await withTimeout(timeout) { [initializationTask] in  // Capture specific property
            try await initializationTask.value
        }
    }

    private func withTimeout<T: Sendable>(_ seconds: TimeInterval, operation: @escaping @Sendable () async throws -> T) async throws -> T {
        try await withThrowingTaskGroup(of: T.self) { group in
            group.addTask {
                try await operation()
            }

            group.addTask {
                try await Task.sleep(nanoseconds: UInt64(seconds * 1_000_000_000))
                throw ShiraGameError.initializationTimeout
            }

            let result = try await group.next()!
            group.cancelAll()
            return result
        }
    }

    public func searchROM(byMD5 md5: String) async throws -> ROMMetadata? {
        try await awaitInitialization()

        // Normalize MD5 to lowercase to match database
        let normalizedMD5 = md5.lowercased()

        // First find the ROM
        let roms = try db.roms.filter(filter: { $0.md5 == normalizedMD5 })
        DLOG("ShiraGame: Found \(roms.count) ROMs for MD5: \(normalizedMD5)")
        guard let rom = roms.first else { return nil }

        // Then find the corresponding game
        let games = try db.games.filter(filter: { $0.id == rom.gameId })
        DLOG("ShiraGame: Found \(games.count) games for ROM ID: \(rom.gameId)")
        guard let game = games.first else { return nil }

        DLOG("ShiraGame: Platform ID: \(game.platformId)")
        let metadata = convertToROMMetadata(game: game, rom: rom)
        DLOG("ShiraGame: System ID: \(metadata.systemID)")

        return metadata
    }

    public func searchDatabase(usingFilename filename: String, systemID: SystemIdentifier?) async throws -> [ROMMetadata]? {
        try await awaitInitialization()

        // First find ROMs matching filename (case-insensitive)
        let lowercasedFilename = filename.lowercased()
        let roms = try db.roms.filter(filter: { rom in
            let fileName = rom.fileName
            return fileName.lowercased().contains(lowercasedFilename)
        })

        if roms.isEmpty {
            DLOG("ShiraGame: No ROMs found matching filename: \(filename)")
            return nil
        }

        // Find corresponding games
        let gameIds = Set(roms.map { $0.gameId })
        let games: [ShiragameSchema.Game]

        if let platformId = systemID?.shiraGameID {
            // If we have a system ID, filter by both game ID and platform
            games = try db.games.filter(filter: { game in
                guard let id = game.id else { return false }
                return gameIds.contains(id) && game.platformId == platformId
            })
        } else {
            // Otherwise just filter by game ID
            games = try db.games.filter(filter: { game in
                guard let id = game.id else { return false }
                return gameIds.contains(id)
            })
        }

        let results: [ROMMetadata] = games.compactMap { game in
            guard let rom = roms.first(where: { $0.gameId == game.id }) else {
                return nil
            }
            return convertToROMMetadata(game: game, rom: rom)
        }

        DLOG("ShiraGame: Found \(results.count) results for filename: \(filename)")
        return results.isEmpty ? nil : results
    }

    public func systemIdentifier(forRomMD5 md5: String, or filename: String?) async throws -> SystemIdentifier? {
        try await awaitInitialization()

        let normalizedMD5 = md5.lowercased()

        // Try MD5 first
        if let rom = try db.roms.filter(filter: { $0.md5 == normalizedMD5 }).first,
           let game = try db.games.filter(filter: { $0.id == rom.gameId }).first {
            return SystemIdentifier.fromShiraGameID(game.platformId)
        }

        // Try filename if MD5 fails
        if let filename = filename,
           let rom = try db.roms.filter(filter: { $0.fileName.contains(filename) }).first,
           let game = try db.games.filter(filter: { $0.id == rom.gameId }).first {
            return SystemIdentifier.fromShiraGameID(game.platformId)
        }

        return nil
    }

    // MARK: - Private Helpers

    private func convertToROMMetadata(game: ShiragameSchema.Game, rom: ShiragameSchema.Rom) -> ROMMetadata {
        let systemIdentifier = SystemIdentifier.fromShiraGameID(game.platformId) ?? .Unknown

        return ROMMetadata(
            gameTitle: game.entryName,
            boxImageURL: nil,  // ShiraGame doesn't provide artwork
            region: game.region,
            gameDescription: nil,
            boxBackURL: nil,
            developer: nil,
            publisher: nil,
            serial: nil,
            releaseDate: nil,
            genres: nil,
            referenceURL: nil,
            releaseID: nil,
            language: nil,
            regionID: nil,
            systemID: systemIdentifier,  // Now passing SystemIdentifier directly
            systemShortName: game.platformId,
            romFileName: rom.fileName,
            romHashCRC: rom.crc,
            romHashMD5: rom.md5,
            romID: Int(game.id ?? 0),
            isBIOS: game.isSystem,
            source: "ShiraGame"
        )
    }
}
