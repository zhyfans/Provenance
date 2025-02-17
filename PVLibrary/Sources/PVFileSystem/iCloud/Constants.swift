//
//  Constants.swift
//  PVLibrary
//
//  Created by Joseph Mattiello on 8/6/24.
//

import Foundation

package enum Constants {}

package extension Constants {
    enum iCloud {
        static let defaultProvenanceContainerIdentifier = "iCloud.org.provenance-emu.provenance"
        // Dynamic version based off of bundle Identifier
		static let containerIdentifier =  (Bundle.main.infoDictionary?["NSUbiquitousContainers"] as? [String: AnyObject])?.keys.first ?? defaultProvenanceContainerIdentifier
    }
}
