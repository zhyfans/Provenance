//
//  PVSaveState+Artwork.swift
//  PVUI
//
//  Created by Joseph Mattiello on 11/25/24.
//

import PVLibrary
import UIKit

public extension PVSaveState {
    public func fetchUIImage() -> UIImage?  {
        guard let path: String = image?.url.standardizedFileURL.path else { return nil }
        return  UIImage(contentsOfFile: path)
    }
}
