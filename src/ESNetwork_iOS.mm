//
//  ESNetwork_iOS.mm
//
//  Created by Steven Pucci 26 Feb 2013
//  Copyright Emerald Sequoia LLC 2013. All rights reserved.
//

#include "ESNetwork.hpp"
#include <UIKit/UIApplication.h>

// =============================
// Indicator control
// =============================

/*static*/ void 
ESNetwork::startNetworkActivityIndicatorForRealInMainThread() {
// Deprecated in iOS 13    [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
}

/*static*/ void 
ESNetwork::stopNetworkActivityIndicatorForRealInMainThread() {
// Deprecated in iOS 13    [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
}
