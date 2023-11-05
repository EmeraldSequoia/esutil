//
//  ESOfflineLogger.hpp
//
//  Created by Steve Pucci 10 Nov 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//

#ifndef _ESOFFLINELOGGER_HPP_
#define _ESOFFLINELOGGER_HPP_

#include <string>

/** class description */
class ESOfflineLogger {
  public:
    static void             initialize();
    static void             log(const std::string &txt);

    static void             writeScreenCaptureRequestFile(const std::string &filename,
                                                          const std::string &textToWrite);
    static void             screenCaptureResponseFileExists(const std::string &filename);
};

#endif  // _ESOFFLINELOGGER_HPP_
