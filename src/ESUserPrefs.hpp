//
//  ESUserPrefs.hpp
//
//  Created by Steve Pucci 25 Jan 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESUSERPREFS_HPP_
#define _ESUSERPREFS_HPP_

#include "string"

/*! Platform-independent storage of user preferences.
 *  Names are qualified by the specific application, either by the OS or by this class's
 *  per-platform implementation.
 */
class ESUserPrefs {
  public:

    // These methods are used to obtain the value of a pref.
    static bool             boolPref(const char *name);
    static bool             boolPref(const std::string &name) { return boolPref(name.c_str()); }
    static int              intPref(const char *name);
    static int              intPref(const std::string &name) { return intPref(name.c_str()); }
    static double           doublePref(const char *name);
    static double           doublePref(const std::string &name) { return doublePref(name.c_str()); }
    static std::string      stringPref(const char *name);
    static std::string      stringPref(const std::string &name) { return stringPref(name.c_str()); }
    
    // These methods are used to initialize the defaults before reading the prefs from the last session.
    //  note that through C++ overloading the names are all the same, unlike the "get" methods which
    //  cannot overload based on return type
    static void             initDefaultPref(const char *name,
                                            bool       value);
    static void             initDefaultPref(const char *name,
                                            int        value);
    static void             initDefaultPref(const char *name,
                                            double     value);
    static void             initDefaultPref(const char *name,
                                            const char *value);

    // These methods are used to change the value during a session
    static void             setPref(const char *name,
                                    bool       value);
    static void             setPref(const std::string &name,
                                    bool              value) { setPref(name.c_str(), value); }
    static void             setPref(const char *name,
                                    int        value);
    static void             setPref(const std::string &name,
                                    int               value) { setPref(name.c_str(), value); }
    static void             setPref(const char *name,
                                    double     value);
    static void             setPref(const std::string &name,
                                    double            value) { setPref(name.c_str(), value); }
    static void             setPref(const char *name,
                                    const char *value);
    static void             setPref(const std::string &name,
                                    const std::string &value) { setPref(name.c_str(), value.c_str()); }

    // This method ensures the values are written to "disk"
    static void             synchronize();
};

#endif  // _ESUSERPREFS_HPP_
