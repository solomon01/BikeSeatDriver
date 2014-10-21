//
//  Direction.swift
//  BikeGuider
//
//  Created by Greg Stromire on 10/16/14.
//  Copyright (c) 2014 Greg Stromire. All rights reserved.
//


import Foundation
import UIKit
import CoreLocation

enum Arrow : String {
	case North = "North"
	case South = "South"
	case East = "East"
	case West = "West"
	case Right = "Right"
	case Left = "Left"
	case Continue = "Continue"
	case Go = "Go"
	static let allValues = [North, South, East, West, Right, Left, Continue]
}

class Direction {
	
	var startCoord : CLLocationCoordinate2D?
	var endCoord : CLLocationCoordinate2D?
	var rawInstruction : String?
	var maneuver : String?
	
	lazy var attributedInstruction : NSAttributedString = {
		let attrData : NSData? = "<span style=\"font-size: 17\">\(self.rawInstruction!)</span>".dataUsingEncoding(NSUTF8StringEncoding, allowLossyConversion: true)
		let attributed = NSMutableAttributedString(data: attrData!, options: [NSDocumentTypeDocumentAttribute : NSHTMLTextDocumentType, NSCharacterEncodingDocumentAttribute : NSUTF8StringEncoding], documentAttributes: nil, error: nil)
//		println("Initialized?")
		return attributed!
	}()
	
	var strippedInstruction : String? {
		return self.attributedInstruction.string.stringByReplacingOccurrencesOfString("\n", withString: ". ", options: NSStringCompareOptions.LiteralSearch, range: nil)
	}
	
	lazy var arrow : Arrow = {
		var maneuver : String?
		if let manString : String? = self.maneuver as String? {
			maneuver = manString
		} else {
			maneuver = self.strippedInstruction?
		}
		for way in Arrow.allValues {
			println("Searching for way: \(way.rawValue) in man: \(maneuver?)")
			if maneuver?.rangeOfString(way.rawValue, options: NSStringCompareOptions.CaseInsensitiveSearch, range: nil, locale: nil) != nil {
				return way
			}
		}
		return Arrow.Go
	}()
	
	init() {
//		println("Init")
	}
}
