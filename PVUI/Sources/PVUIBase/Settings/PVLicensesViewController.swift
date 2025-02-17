//  Converted to Swift 4 by Swiftify v4.1.6613 - https://objectivec2swift.com/
//
//  PVLicensesViewController.swift
//  Provenance
//
//  Created by Marcel Voß on 20.09.16.
//  Copyright © 2016 James Addyman. All rights reserved.
//

import PVSupport
#if canImport(UIKit)
import UIKit
#endif
import Darwin
#if canImport(WebKit)
import WebKit
#endif

public final class PVLicensesViewController: UIViewController {
    #if os(tvOS) || targetEnvironment(macCatalyst)
        @IBOutlet var textView: UITextView!
    #endif

    public override func viewDidLoad() {
        super.viewDidLoad()
        // Do any additional setup after loading the view.
        title = NSLocalizedString("Acknowledgements", comment: "")

        #if canImport(WebKit)
            view.backgroundColor = UIColor.black
            let filesystemPath: String? = Bundle.main.path(forResource: "licenses", ofType: "html")
            let htmlContent = try? String(contentsOfFile: filesystemPath ?? "", encoding: .utf8)
            let configuration: WKWebViewConfiguration = WKWebViewConfiguration()
            configuration.suppressesIncrementalRendering = true
            let webView = WKWebView(frame: view.bounds, configuration: configuration)
            webView.isOpaque = false
            webView.backgroundColor = UIColor.black
            webView.loadHTMLString(htmlContent ?? "", baseURL: nil)
            webView.scalesLargeContentImage = false
            webView.scrollView.bounces = false
            webView.autoresizingMask = [.flexibleHeight, .flexibleWidth]
            view.addSubview(webView)
        #else
            guard let url = Bundle.main.url(forResource: "licenses", withExtension: "html") else {
                ELOG("Failed to read licenses.html")
                return
            }

            let options = [NSAttributedString.DocumentReadingOptionKey.documentType: NSAttributedString.DocumentType.html]
            // Change font colors for tvOS
            let html = try! String(contentsOf: url).replacingOccurrences(of: "white", with: "black").replacingOccurrences(of: "#dddddd", with: "#555").replacingOccurrences(of: "#ccc", with: "#444")

            let attributedString = try! NSMutableAttributedString(data: html.data(using: .utf8)!, options: options, documentAttributes: nil)

            textView.attributedText = attributedString
            textView.isUserInteractionEnabled = true
            textView.isSelectable = true
            textView.isScrollEnabled = true
            textView.panGestureRecognizer.allowedTouchTypes = [.indirect]
            textView.showsVerticalScrollIndicator = true
            textView.bounces = true

            textView.becomeFirstResponder()
        #endif
    }

    public override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }

    #if os(tvOS)
    public override var preferredFocusedView: UIView? {
            return textView
        }
    #endif
}
