//  Converted to Swift 4 by Swiftify v4.1.6640 - https://objectivec2swift.com/
//
//  PViCadeControllerViewController.swift
//  Provenance
//
//  Created by James Addyman on 17/04/2015.
//  Copyright (c) 2015 James Addyman. All rights reserved.
//

import PVSupport
#if canImport(UIKit)
import UIKit
#endif
import PVSettings
import PVUIBase

public final class PViCadeControllerViewController: UITableViewController {
    public override func viewDidLoad() {
        super.viewDidLoad()
        #if os(tvOS)
            tableView.backgroundColor = UIColor.black
            tableView.backgroundView = nil
        #endif
    }

    public override func numberOfSections(in _: UITableView) -> Int {
        return 1
    }

    public override func tableView(_: UITableView, numberOfRowsInSection _: Int) -> Int {
        return iCadeControllerSetting.allCases.count
    }

    public override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = tableView.dequeueReusableCell(withIdentifier: "iCadeCell") ?? UITableViewCell(style: .default, reuseIdentifier: "iCadeCell")

        if indexPath.row == Defaults[.myiCadeControllerSetting].rawValue {
            cell.accessoryType = .checkmark
        } else {
            cell.accessoryType = .none
        }
        cell.textLabel?.text = iCadeControllerSetting(rawValue: indexPath.row)?.description ?? "nil"

        #if os(iOS)
            cell.backgroundColor = .secondarySystemGroupedBackground
        #endif

        return cell
    }

    public override func tableView(_: UITableView, titleForHeaderInSection _: Int) -> String? {
        return "Supported iCade Controllers"
    }

    public override func tableView(_: UITableView, titleForFooterInSection _: Int) -> String? {
        return "Controllers must be paired with device."
    }

    public override func tableView(_: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.cellForRow(at: indexPath)?.accessoryType = .checkmark
        if let aRow = self.tableView.indexPathForSelectedRow {
            tableView.deselectRow(at: aRow, animated: true)
        }
        Defaults[.myiCadeControllerSetting] = iCadeControllerSetting(rawValue: indexPath.row)!
        PVControllerManager.shared.resetICadeController()
        navigationController?.popViewController(animated: true)
    }
}
