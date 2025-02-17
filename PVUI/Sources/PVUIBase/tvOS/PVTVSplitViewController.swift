//  Converted to Swift 4 by Swiftify v4.1.6613 - https://objectivec2swift.com/
//
//  PVTVSplitViewController.swift
//  Provenance
//
//  Created by James Addyman on 26/09/2015.
//  Copyright © 2015 James Addyman. All rights reserved.
//

import UIKit

#if os(tvOS)
public final class PVTVSplitViewController: UISplitViewController {
    public override func viewDidLoad() {
        super.viewDidLoad()
        preferredPrimaryColumnWidthFraction = 0.5
    }
}
#endif
