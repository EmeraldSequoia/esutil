//
//  ESFile_simpleResource.cpp
//
//  Created by Steve Pucci 01 Sep 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#include "ESFile.hpp"
#include "ESFilePvt.hpp"

/*static*/ int 
ESFile::getFDPointingAtResource(const char   *resourcePath,
                                bool         missingOK,
                                size_t       *resourceSizeReturn,
                                ESFileCloser **fileCloser) {
    int fd = getFDPointingAtFileInDirectory(resourcePath, resourceDirectory(), missingOK, resourceSizeReturn);
    *fileCloser = new ESStaticFileCloser(fd);
    return fd;
}
