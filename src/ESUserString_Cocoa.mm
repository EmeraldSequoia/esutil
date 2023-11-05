//
//  ESUserString_Cocoa.cpp
//
//  Created by Steve Pucci on 11/22/2010
//  Copyright 2010 Emerald Sequoia LLC. All rights reserved.
//

// The portion of the implementation of ESUserString which is specific to iOS

#import <Foundation/Foundation.h>

#include "ESUserString.hpp"

#if !ES_COCOA
syntax error;   // Don't try to build this file on other platforms
#endif

std::string 
ESUserString::findLocalizedValue(const std::string &englishForm) {
    return [[[NSBundle mainBundle] localizedStringForKey:[NSString stringWithUTF8String:englishForm.c_str()] value:@"" table:nil] UTF8String];
}
