//
//  ESUserString.hpp
//
//  Created by Steve Pucci on 11/19/2010
//  Copyright 2010 Emerald Sequoia LLC. All rights reserved.
//

#ifndef _ESUSERSTRING_H
#define _ESUSERSTRING_H

#include "ESPlatform.h"

#include "string"

ES_OPAQUE_OBJC(NSString);

#define ESLocalizedString(str, helpfulComment) ESUserString(str)  // helpfulComment is only for translators (thru extraction scripts) -- we don't use it

/*! A portable, localized string which is translated appropriately, no matter which platform we're running on. */
class ESUserString {
  public:
			    ESUserString(const char *englishForm);
                            ESUserString(const std::string &englishForm);
			    ~ESUserString();

    std::string             localizedValue() const;
    
  protected:
			    ESUserString(const char *universalForm,
                                         bool       preLocalized);

    static std::string      findLocalizedValue(const std::string &englishForm);

    mutable std::string     _cachedLocalizedValue;
    mutable bool            _cacheValid;
};

/*! A string which doesn't need further localization, either because it is a pure number or because it has already
 * been localized (e.g., with a date formatter).
 */
class ESPreLocalizedUserString : public ESUserString {
  public:
                            ESPreLocalizedUserString(const char *universalForm);

    static ESPreLocalizedUserString emptyString;
};

#endif // _ESUSERSTRING_H
