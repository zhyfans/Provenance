import Foundation
import PVPrimitives

public protocol RealmRepresentable {
    associatedtype RealmType: DomainConvertibleType

    var uid: String { get }

    @MainActor
    func asRealm() async -> RealmType
}
