// swift-tools-version:5.10
// The swift-tools-version declares the minimum version of Swift required to build this package.
import PackageDescription
import Foundation

#if swift(>=5.9)
var pvemulatorCoreSwiftFlags: [SwiftSetting] = [
    .define("GL_SILENCE_DEPRECATION"),
    .define("GLES_SILENCE_DEPRECATION"),
    .define("CI_SILENCE_GL_DEPRECATION")
    //    .interoperabilityMode(.Cxx)
]
#else
var pvemulatorCoreSwiftFlags: [SwiftSetting] = [
    .define("GL_SILENCE_DEPRECATION"),
    .define("GLES_SILENCE_DEPRECATION"),
    .define("CI_SILENCE_GL_DEPRECATION")
]
#endif

let package = Package(
    name: "PVEmulatorCore",
    platforms: [
        .iOS(.v13),
        .tvOS(.v13),
        .watchOS(.v7),
        .macOS(.v11),
        .macCatalyst(.v14)
    ],
    products: [
        .library(
            name: "PVEmulatorCore",
            targets: ["PVEmulatorCore"]),
         .library(
             name: "PVEmulatorCore-Dynamic",
             type: .dynamic,
             targets: ["PVEmulatorCore"]),
         .library(
             name: "PVEmulatorCore-Static",
             type: .static,
             targets: ["PVEmulatorCore"]),
    ],

    dependencies: [
        // Dependencies declare other packages that this package depends on.
        .package(name: "PVCoreBridge", path: "../PVCoreBridge/"),
        .package(name: "PVLogging", path: "../PVLogging/"),
        .package(name: "PVSupport", path: "../PVSupport/"),
        .package(name: "PVObjCUtils", path: "../PVObjCUtils/"),
        .package(name: "PVAudio", path: "../PVAudio/")
    ],

    // MARK: - Targets
    targets: [
        // MARK: - PVEmulatorCore
        .target(
            name: "PVEmulatorCore",
            dependencies: [
                "PVCoreBridge",
                .product(name: "PVObjCUtils", package: "PVObjCUtils"),
                .product(name: "PVSupport", package: "PVSupport"),
                .product(name: "PVLogging", package: "PVLogging"),
                .product(name: "PVAudio", package: "PVAudio")
            ],
            cSettings: [
                .headerSearchPath("include"),
                .define("GL_SILENCE_DEPRECATION",
                        .when(platforms: [.macOS, .macCatalyst])),
                .define("GLES_SILENCE_DEPRECATION",
                        .when(platforms: [.iOS, .tvOS, .watchOS, .macCatalyst])),
                .define("CI_SILENCE_GL_DEPRECATION")
            ],
            swiftSettings: pvemulatorCoreSwiftFlags,
            linkerSettings: [
				.linkedFramework("Metal"),
				.linkedFramework("MetalKit"),
                .linkedFramework("GameController", .when(platforms: [.iOS, .tvOS, .macCatalyst])),
				.linkedFramework("UIKit", .when(platforms: [.iOS, .tvOS, .macCatalyst])),
                .linkedFramework("OpenGLES", .when(platforms: [.iOS, .tvOS, .watchOS])),
                .linkedFramework("OpenGL", .when(platforms: [.macOS])),
				.linkedFramework("AppKit", .when(platforms: [.macOS])),
                .linkedFramework("CoreGraphics"),
                .linkedFramework("WatchKit", .when(platforms: [.watchOS]))
            ]
        ),

        // MARK: SwiftPM tests
        .testTarget(
            name: "PVEmulatorCoreTests",
            dependencies: ["PVEmulatorCore"]
        )
    ],
    swiftLanguageVersions: [.v5],
    cLanguageStandard: .gnu11,
    cxxLanguageStandard: .gnucxx20
)
