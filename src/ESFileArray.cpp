//
//  ESFileArray.cpp
//
//  Created by Steven Pucci 21 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESFileArray.hpp"
#include "ESErrorReporter.hpp"

class ESFileStringArray;

ESFileStringArray::ESFileStringArray(const char     *path,
                                     ESFilePathType pathType,
                                     int            arraySizeGuess)
:   ESFileArray<char>(path, pathType)
{
    if (!_array) {
        _strings = NULL;
        _numStrings = 0;
        _stringsCapacity = 0;
        return;
    }
    _stringsCapacity = arraySizeGuess;
    ESAssert(arraySizeGuess > 0);
    _strings = (const char **)malloc(_stringsCapacity * sizeof(char *));
    char *ptr = _array;
    char *end = _array + _bytesRead;
    const char **dest = _strings;
    const char **destCapacityEnd = _strings + _stringsCapacity;
    while (ptr < end) {
	char *adv = ptr;
	while (*adv++)
	    ;  // empty
        if (dest >= destCapacityEnd) {
            _stringsCapacity *= 1.5;
            size_t destOffset = dest - _strings;
            _strings = (const char **)realloc(_strings, _stringsCapacity * sizeof(char *));
            dest = _strings + destOffset;
            destCapacityEnd = _strings + _stringsCapacity;
        }
        *dest++ = ptr;
	ESAssert(adv <= end);
	ptr = adv;
    }
    _numStrings = dest - _strings;
}

ESFileStringArray::~ESFileStringArray() {
    free((char **)_strings);
}

