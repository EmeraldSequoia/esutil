//
//  ESUtil_iOS.mm
//
//  Created by Steve Pucci 27 Jan 2012
//  Copyright Emerald Sequoia LLC 2012. All rights reserved.
//

#include "ESUtil.hpp"

#import "UIKit/UIKit.h"

/*static*/ bool
ESUtil::isTablet() {
    static bool initialized = false;
    static bool isipad = false;
    if (!initialized) {
	if ([UIDevice instancesRespondToSelector:@selector(userInterfaceIdiom)]) {
	    isipad = ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad);
	    //printf("sez it's %s\n", isIpad ? "ipad" : "not ipad");
	} else {
	    isipad = false;
	}
    }
    return isipad;
}

/*static*/ ESBackgroundTaskID 
ESUtil::requestMoreTimeForBackgroundTask(ESBGTaskExpirationHandler expirationHandler) {
    ESBackgroundTaskID taskID = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:^(void){ (*expirationHandler)(); }];
    if (taskID == UIBackgroundTaskInvalid) {
        return 0;
    } else {
        return (ESBackgroundTaskID)taskID;
    }
}

/*static*/ void 
ESUtil::declareBackgroundTaskFinished(ESBackgroundTaskID taskID) {
    // This may not be on the main thread, but the docs say it's ok
    [[UIApplication sharedApplication] endBackgroundTask:taskID];
}

