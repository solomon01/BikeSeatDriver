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
	
	lazy var attributedInstruction : NSAttributedString = {
		let attributed = NSMutableAttributedString(data: self.rawInstruction?.dataUsingEncoding(NSUTF8StringEncoding, allowLossyConversion: true), options: [NSDocumentTypeDocumentAttribute : NSHTMLTextDocumentType, NSCharacterEncodingDocumentAttribute : NSUTF8StringEncoding], documentAttributes: nil, error: nil)
//		println("Initialized?")
		return attributed
	}()
	
	var strippedInstruction : String? {
		return self.attributedInstruction.string
	}
	
	lazy var arrow : Arrow = {
		for way in Arrow.allValues {
			if self.strippedInstruction?.rangeOfString(way.toRaw(), options: NSStringCompareOptions.CaseInsensitiveSearch, range: nil, locale: nil) != nil {
				return way
			}
		}
		return Arrow.Go
	}()
	
	init() {
//		println("Init")
	}
}
