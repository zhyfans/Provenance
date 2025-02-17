//
//  PVCoresTableViewController
//  Provenance
//
//  Created by Joe Mattiello on 16.03.18.
//  Copyright © 2018 Joe Mattiello. All rights reserved.
//

import PVLibrary
import PVSettings

import RealmSwift
#if canImport(UIKit)
import UIKit
#endif
#if canImport(MBProgressHUD)
import MBProgressHUD
#endif

public final class PVCoresTableViewController: QuickTableViewController {
    public override func viewDidLoad() {
        super.viewDidLoad()

        /// Get cores from the RomDatabase
        let cores = RomDatabase.sharedInstance.all(PVCore.self, sortedByKeyPath: #keyPath(PVCore.projectName))

#if os(tvOS)
        splitViewController?.title = NSLocalizedString("Cores", comment: "")
#endif
        /// If we should hide unsupported systems
        let showsUnsupportedSystems = Defaults[.unsupportedCores]

        tableContents = [
            Section(title: NSLocalizedString("Cores", comment: ""), rows: cores.map { core in
                /// Get the list of systems it supports
                let systemsText = core.supportedSystems.filter { $0.supported || showsUnsupportedSystems }.map({ $0.shortName }).joined(separator: ", ")

                /// Show the project version and the systems it supports
                let detailLabelText = "\(core.projectVersion) : \(systemsText)"

                return NavigationRow(text: core.projectName, detailText: .subtitle(detailLabelText), icon: nil, customization: { cell, _ in
#if os(iOS)
                    /// If the core has a URL, show a disclosure indicator
                    if URL(string: core.projectURL) != nil {
                        cell.accessoryType = .disclosureIndicator
                    } else {
                        cell.accessoryType = .none
                    }
#else
                    /// On tvOS, don't show an accessory type
                    cell.accessoryType = .none
                    cell.selectionStyle = .none
#endif
                }, action: { _ in
#if os(iOS)
                    /// If the core has a URL, show a web view
                    guard let url = URL(string: core.projectURL) else {
                        return
                    }

                    let webVC = WebkitViewController(url: url)
                    webVC.title = core.projectName

                    self.navigationController?.pushViewController(webVC, animated: true)
#endif
                })
            })
        ]
    }
}

#if os(iOS)
import WebKit
class WebkitViewController: UIViewController {
    private let url: URL
    private var webView: WKWebView!
#if canImport(MBProgressHUD)
    private var hud: MBProgressHUD!
#endif
    private var token: NSKeyValueObservation?

    init(url: URL) {
        self.url = url
        super.init(nibName: nil, bundle: nil)
    }

    required init?(coder _: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    override func viewWillDisappear(_ animated: Bool) {
        super.viewWillDisappear(animated)
        token?.invalidate()
        token = nil
    }

    override func viewDidLoad() {
        let config = WKWebViewConfiguration()
        let webView = WKWebView(frame: view.bounds, configuration: config)
        webView.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        webView.navigationDelegate = self
        self.webView = webView

        view.addSubview(webView)

#if canImport(MBProgressHUD)
        let hud = MBProgressHUD(view: view)
        hud.isUserInteractionEnabled = false
        hud.mode = .determinateHorizontalBar
        hud.progress = 0
        hud.label.text = NSLocalizedString("Loading", comment: "Loading") + "…"

        self.hud = hud
        webView.addSubview(hud)

        token = webView.observe(\.estimatedProgress) { webView, _ in
            let estimatedProgress = webView.estimatedProgress
            self.hud.progress = Float(estimatedProgress)
        }
#endif
    }

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        webView.load(URLRequest(url: url))
    }
}

extension WebkitViewController: WKNavigationDelegate {
    func webView(_: WKWebView, didStartProvisionalNavigation _: WKNavigation!) {
#if canImport(MBProgressHUD)
        hud.show(animated: true)
#endif
    }

    func webView(_: WKWebView, didFinish _: WKNavigation!) {
#if canImport(MBProgressHUD)
        hud.hide(animated: true, afterDelay: 0.0)
#endif
    }
}
#endif
