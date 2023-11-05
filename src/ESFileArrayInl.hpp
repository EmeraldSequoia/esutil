//
//  ESUtilFilesInl.hpp
//
//  Created by Steven Pucci 21 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESUTILFILESINL_HPP_
#define _ESUTILFILESINL_HPP_

#include "ESErrorReporter.hpp"
#include "ESUtil.hpp"

#include <sys/errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

// These definitions are inline to avoid portability issues with templates defined in c++ files.

template <class ElementType>
inline
ESFileArray<ElementType>::ESFileArray(const char     *path,
                                      ESFilePathType pathType,
                                      bool           readInAtStartup)
:   _array(NULL),
    _bytesRead(0)
{
    if (!readInAtStartup) {
        return;
    }
    _array = (ElementType *)ESFile::getFileContentsInMallocdArray(path, pathType, false/* !missingOK*/, &_bytesRead);
    if (_array) {
        ESErrorReporter::logInfo("ESFileArray", "Successful read of %s\n", path);
    } else {
        ESErrorReporter::logError("ESFileArray", "Unsuccessful read of %s\n", path);
    }
}

template <class ElementType>
inline
ESFileArray<ElementType>::~ESFileArray() {
    if (_array) {
        free(_array);
    }
}

template <class ElementType>
inline void
ESFileArray<ElementType>::setupForWriteWithNumElements(int numElements) {
    if (_array) {
        free(_array);
    }
    ESAssert(numElements > 0);
    _bytesRead = numElements * sizeof(ElementType);
    _array = (ElementType *)malloc(_bytesRead);
}

template <class ElementType>
inline bool
ESFileArray<ElementType>::writeToPath(const char     *path,
                                      ESFilePathType pathType) {
    return ESFile::writeArrayToFile(_array, _bytesRead, path, pathType);
}

template <class ElementType>
/*static*/ inline void 
ESFileArray<ElementType>::readElementFromFileAtIndex(const char     *path,
                                                     ESFilePathType pathType,
                                                     int            indx,
                                                     ElementType    *element) {
    size_t elementSizeInBytes = sizeof(ElementType);
    size_t fileSize;
    ESFileCloser *fileCloser;
    int fd = ESFile::getFDPointingAtFile(path, pathType, false/* !missingOK*/, &fileSize, &fileCloser);
    if (fd < 0) {
        bzero(element, elementSizeInBytes);
        fileSize = 0;
        return;
    }
    size_t off = indx * elementSizeInBytes;
    ESAssert(off + elementSizeInBytes <= fileSize);
    off_t st2 = lseek(fd, off, SEEK_CUR);
    if (st2 < 0) {
        bzero(element, elementSizeInBytes);
        if (fileCloser) {
            fileCloser->closeAndDie();
        }
        ESErrorReporter::checkAndLogSystemError("ESFileArray", errno, ESUtil::stringWithFormat("Trouble seeking to position %d of %s file %s\n",
                                                                                               off, ESFile::pathTypeString(pathType), path).c_str());
        ESAssert(false);
        return;
    }
    ssize_t st3 = read(fd, element, elementSizeInBytes);
    if (st3 != elementSizeInBytes) {
        bzero(element, elementSizeInBytes);
        if (fileCloser) {
            fileCloser->closeAndDie();
        }
        ESErrorReporter::checkAndLogSystemError("ESFileArray", errno, ESUtil::stringWithFormat("Trouble seeking to position %d of %s file %s\n",
                                                                                               off, ESFile::pathTypeString(pathType), path).c_str());
        ESAssert(false);
        return;
    }
    if (fileCloser) {
        ESErrorReporter::logInfo("ESFileArray::readElementFromFileAtIndex", "closing fd %d", fd);
        fileCloser->closeAndDie();
    }
}

inline const char *
ESFileStringArray::stringAtIndex(int indx) {
    ESAssert(indx >= 0);
    ESAssert(indx < _numStrings);
    return _strings[indx];
}

#endif  // _ESUTILFILESINL_HPP_
