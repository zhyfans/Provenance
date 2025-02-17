//
//  AppearanceStyleable.swift
//  SwiftyAppearance
//
//  Created by Victor Pavlychko on 5/2/17.
//  Copyright © 2017 address.wtf. All rights reserved.
//

#if canImport(UIKit)
import UIKit
#else
import AppKit
public typealias UIViewController = NSViewController
#endif

// @IBDesignable
public extension UIView {
    @IBInspectable var appearanceStyleName: String {
        get { return appearanceStyle.name }
        set { appearanceStyle = AppearanceStyle(newValue) }
    }

    var appearanceStyle: AppearanceStyle {
        get { return AppearanceStyle(styleName(String(cString: object_getClassName(self)))) }
        set { setAppearanceStyle(newValue, animated: false) }
    }

    func setAppearanceStyle(_ style: AppearanceStyle, animated: Bool) {
        object_setClass(self, styleClass(type(of: self), styleName: style.name))
        appearanceRoot?.refreshAppearance(animated: animated)
    }

    private var appearanceRoot: UIWindow? {
        return window ?? (self as? UIWindow)
    }
}

// @IBDesignable
public extension UIViewController {
    @IBInspectable var appearanceStyleName: String {
        get { return appearanceStyle.name }
        set { appearanceStyle = AppearanceStyle(newValue) }
    }

    var appearanceStyle: AppearanceStyle {
        get { return AppearanceStyle(styleName(String(cString: object_getClassName(self)))) }
        set { setAppearanceStyle(newValue, animated: false) }
    }

    func setAppearanceStyle(_ style: AppearanceStyle, animated: Bool) {
        object_setClass(self, styleClass(type(of: self), styleName: style.name))
        appearanceRoot?.refreshAppearance(animated: animated)
    }

    private var appearanceRoot: UIWindow? {
        if #available(macOS 14.0, *) {
            return viewIfLoaded?.window
        } else {
            return view.window
        }
    }
}
