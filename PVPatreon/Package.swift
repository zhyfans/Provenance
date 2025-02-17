// swift-tools-version:6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription

let package = Package(
    name: "PVPatreon",
    platforms: [
        .iOS(.v13),
        .tvOS(.v13),
        .watchOS(.v7),
        .macOS(.v11),
        .macCatalyst(.v14)
    ],
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(
            name: "PVPatreon",
            targets: ["PVPatreon"]),
        .library(
            name: "PVPatreon-Dynamic",
            type: .dynamic,
            targets: ["PVPatreon"]),
        .library(
            name: "PVPatreon-Static",
            type: .static,
            targets: ["PVPatreon"])
    ],
    dependencies: [
        .package(
            name: "PVLogging",
            path: "../PVLogging"),
        .package(
            url: "https://github.com/kishikawakatsumi/KeychainAccess.git",
            from: "4.2.2")
    ],
    targets: [
        // MARK: -- Patreon API
        .target(
            name: "PVPatreon",
            dependencies: [
                "PVLogging",
                "KeychainAccess"
            ],
            resources: [
                .copy("Resources/Server/")
            ],
            linkerSettings: [
                .linkedFramework("AuthenticationServices"),
                .linkedFramework("CoreData")
            ]
        ),
        
        // MARK: -- Patreon UI
        .target(
            name: "PVPatreonUI",
            dependencies: [
                "PVLogging",
                "KeychainAccess"
            ],
            linkerSettings: [
                .linkedFramework("AuthenticationServices"),
                .linkedFramework("CoreData")
            ]
        ),

        // MARK: SwiftPM tests
        .testTarget(
            name: "PVPatreonTests",
            dependencies: ["PVPatreon"],
            path: "Tests")
    ],
    swiftLanguageVersions: [.v5]
)
