//
//  ESUserString.cpp
//
//  Created by Steve Pucci on 11/22/2010
//  Copyright 2010 Emerald Sequoia LLC. All rights reserved.
//
//  This is the generic (platform-independent or shared-platform) implementation of ESUserString

#include "ESUserString.hpp"
#include "ESErrorReporter.hpp"

ESUserString::ESUserString(const char *englishForm)
:   _cachedLocalizedValue(englishForm),
    _cacheValid(false)
{
}

ESUserString::ESUserString(const std::string &englishForm)
:   _cachedLocalizedValue(englishForm),
    _cacheValid(false)
{
}
 
ESUserString::~ESUserString() {

}

std::string 
ESUserString::localizedValue() const {
    if (!_cacheValid) {
        _cachedLocalizedValue = findLocalizedValue(_cachedLocalizedValue);
        _cacheValid = true;
    }
    return _cachedLocalizedValue;
}

ESUserString::ESUserString(const char *universalForm,
                           bool       preLocalized) {
    ESAssert(preLocalized);  // Only path should be through ESPreLocalizedUserString ctor
    _cacheValid = true;
    _cachedLocalizedValue = universalForm;
}

/*static*/ ESPreLocalizedUserString
ESPreLocalizedUserString::emptyString = "";

ESPreLocalizedUserString::ESPreLocalizedUserString(const char *universalForm)
:   ESUserString(universalForm, true/*preLocalized*/)
{
}

