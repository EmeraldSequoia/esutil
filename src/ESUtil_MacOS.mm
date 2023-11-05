//
//  ESUtil_MacOS.mm
//
//  Created by Steven Pucci 26 Feb 2013
//  Copyright Emerald Sequoia LLC 2013. All rights reserved.
//

#include "ESUtil.hpp"

/*static*/ bool
ESUtil::isTablet() {
    return false;
}

/*static*/ ESBackgroundTaskID 
ESUtil::requestMoreTimeForBackgroundTask(ESBGTaskExpirationHandler expirationHandler) {
    // MacOS tasks run in the background without any special API (though if some day Apple exports Power Nap APIs, we might require something here...)
    return 0;
}

/*static*/ void 
ESUtil::declareBackgroundTaskFinished(ESBackgroundTaskID taskID) {
    // Again, nothing to do
}
