// swift-tools-version:6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

let package = Package(
    name: "PVHashing",
    platforms: [
        .iOS(.v16),
        .tvOS(.v16),
        .watchOS(.v9),
        .macOS(.v11),
        .macCatalyst(.v17),
        .visionOS(.v1)
    ],
    products: [
        .library(
            name: "PVHashing",
            targets: ["PVHashing"]
        )
    ],
    dependencies: [
        .package(path: "../PVLogging"),
//        .package(url: "https://github.com/rnine/Checksum.git", from: "1.0.2")
        .package(url: "https://github.com/JoeMatt/Checksum.git", from: "1.1.1")
    ],
    targets: [
        .target(
            name: "PVHashing",
            dependencies: [ "Checksum", "PVLogging" ]
        ),

        // MARK: SwiftPM tests
        .testTarget(
            name: "PVHashingTests",
            dependencies: ["PVHashing"],
            resources: [ .copy("Resources/testFile.txt") ]
        )
    ],
    swiftLanguageModes: [.v5, .v6],
    cLanguageStandard: .gnu17,
    cxxLanguageStandard: .gnucxx20
)
