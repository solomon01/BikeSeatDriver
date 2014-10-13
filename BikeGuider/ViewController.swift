//
//  ViewController.swift
//  Mapper
//
//  Created by Greg Stromire on 10/8/14.
//  Copyright (c) 2014 Greg Stromire. All rights reserved.
//

import UIKit
import CoreLocation
import AddressBookUI

import Alamofire
import SwiftyJSON

class ViewController: UIViewController, CLLocationManagerDelegate {
	
	// MARK: Class Members
	// User interface
	@IBOutlet weak var statusLabel: UILabel!
	@IBOutlet weak var activityView: UIActivityIndicatorView!
	@IBOutlet weak var addressField: UITextField!
	@IBOutlet weak var directionsBox: UITextView!
	@IBOutlet weak var navigationSelector: UISegmentedControl!
	
	// Class variables
	let geo : CLGeocoder = CLGeocoder()
	var locationManager : CLLocationManager = CLLocationManager()
	var distanceFilter : CLLocationDistance = 10.0
	var start : CLLocation?
	
	// MARK: View Lifetime
	override func viewDidLoad() {
		super.viewDidLoad()
		// Do any additional setup after loading the view, typically from a nib.
		
		// initialize location manager
		locationManager.delegate = self
		locationManager.desiredAccuracy = kCLLocationAccuracyBestForNavigation
		locationManager.distanceFilter = distanceFilter;
		locationManager.activityType = CLActivityType.Fitness
		locationManager.pausesLocationUpdatesAutomatically = true
		
		// request authorization
		if CLLocationManager.authorizationStatus() == CLAuthorizationStatus.NotDetermined {
			locationManager.requestAlwaysAuthorization()
		}
		
		/*
		(To update location / navigation in background)
		
		https://developer.apple.com/library/ios/documentation/CoreLocation/Reference/CLLocationManager_Class/index.html#//apple_ref/occ/clm/CLLocationManager/headingAvailable
		
		Core Location delivers heading data to your app while it is in the foreground. When your app moves to the background, event delivery stops unless your app has the location-updates background mode enabled and the standard location service is running. You can enable background modes from the Capabilities tab of your Xcode project but should do so only if you can use heading data practically while in the background.
		
		
		Using Location Services in the Background ...
		*/
	}
	
	
	// MARK: Memory Management
	
	override func didReceiveMemoryWarning() {
		super.didReceiveMemoryWarning()
		// Dispose of any resources that can be recreated.
	}
	
	// MARK: IB Actions
	
	/**
	Fired when user selects "GO" button. Collects entered address from label,
	geocodes that address into coordinates, constructs parameters for Google
	Maps api call for bicycle directs, and prints those directions to the user's
	text box.
	
	:param: sender UIButton "Go"
	*/
	@IBAction func generateDirections(sender: AnyObject) {
		
		// dismiss keyboard
		addressField.resignFirstResponder()
		
		// if user has entered an address into the text field
		if let addr : String = addressField.text {
			
			// update ui on start of search
			activityView.hidden = false
			activityView.startAnimating()
			directionsBox.text = "Searching..."
			
			// first, geocode the address string to get coordinates
			geo.geocodeAddressString(addr, completionHandler: { (placemarks:[AnyObject]!, error:NSError!) -> Void in
				if placemarks != nil {
					
//				println(placemarks.description)
					
					// the first location is assumed most accurate
					if let there : CLPlacemark = placemarks.first as? CLPlacemark {
//						println(there.location)
						
						// Construct parameters for Google API Call, bicycle directions
						// Uses start and destination coordinates
						// kBROWSER_KEY or kAPI_KEY defined in API_KEYS.swift (not in repo)
						let params : Dictionary = [
							"origin": String("\(self.start!.coordinate.latitude),\(self.start!.coordinate.longitude)"),
							"destination": String("\(there.location.coordinate.latitude), \(there.location.coordinate.longitude)"),
							"mode": "bicycling",
							"key": kBROWSER_KEY
						]
						
						// api call to Google for bicycle directions with Alamofire module,
						// response JSON parsed with SwiftyJSON module
						Alamofire.request(.GET, "https://maps.googleapis.com/maps/api/directions/json", parameters: params).responseJSON { (request, _, jsonData, _) -> Void in
							
							// update ui on final api completion
							self.activityView.stopAnimating()
							
							// parse if response is well-formed json
							if let json = JSON.fromRaw(jsonData!) {
								//object can be converted to JSON
								
								println("Request: \(request.description)")
								println(json.description)
								
								// 'status' response shows if Google successfully found bicycling directions
								if json["status"].stringValue == "OK" {
									
									// traverse into json for direction steps
									let steps = json["routes"][0]["legs"][0]["steps"]
									
									// Google api returns html-styled instuctions,
									// append each direction as html line
									var directions : String = String()
									for (index: String, subJson: JSON) in steps {
										//Do something you want
										directions += subJson["html_instructions"].stringValue + "<br />"
									}
									
									// create attributed string to display html
									let attributedDirections = NSMutableAttributedString(data: directions.dataUsingEncoding(NSUTF8StringEncoding, allowLossyConversion: true), options: [NSDocumentTypeDocumentAttribute : NSHTMLTextDocumentType, NSCharacterEncodingDocumentAttribute : NSUTF8StringEncoding], documentAttributes: nil, error: nil)
									
									self.directionsBox.attributedText = attributedDirections
								} else {
									// Status NOT FOUND
									self.directionsBox.text = "Not Found."
								}
							} else {
								//object can not be converted to JSON
								println("Can't convert JSON...")
							}
						}
					}
				} else {
					// Can't geolocate from address.
					self.directionsBox.text = "Not Found. Error: \(error.description)"
				}
			})
		} else {
			directionsBox.text = "No address entered."
		}
	}
 
	// MARK: CLLocationManagerDelegate Callbacks
	
	func locationManager(manager: CLLocationManager!,
		didChangeAuthorizationStatus status: CLAuthorizationStatus) {
			println("Location Updated")

			
			// Authorization!
			
			println("Authorization?")
			
			switch status {
			case .NotDetermined:
				println("Not Determined?")
			case .Authorized:
				println("Authorized")
				
//				locationManager.startUpdatingLocation()
				// or
				locationManager.startMonitoringSignificantLocationChanges()
				locationManager.startUpdatingHeading()
			case .AuthorizedWhenInUse:
				println("Authorized with in use")
			case .Denied:
				println("Denied")
			case .Restricted:
				println("Restricted?")
			default:
				print("No Status?")
			}
	}
	
	func locationManager(manager: CLLocationManager!, didUpdateLocations locations: [AnyObject]!)	{
		println("Location Updated")
		
		if let here : CLLocation = locations.last as? CLLocation {
			start = here
			geo.reverseGeocodeLocation(here, completionHandler: { (places:[AnyObject]!, error:NSError!) -> Void in
				if let placemark = places.first as? CLPlacemark {
					self.activityView.stopAnimating()
					self.activityView.hidden = true
					self.statusLabel.text = ABCreateStringWithAddressDictionary(placemark.addressDictionary, true)
				}
			})
		}
	}
}