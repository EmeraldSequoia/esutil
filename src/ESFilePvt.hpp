//
//  ESFilePvt.hpp
//
//  Created by Steve Pucci 17 Oct 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//

#ifndef _ESFILEPVT_HPP_
#define _ESFILEPVT_HPP_

#include <unistd.h>

class ESStaticFileCloser : public ESFileCloser {
  public:
                            ESStaticFileCloser(int fd)
    :   _fd(fd)
    {}
    virtual void            closeAndDie() {
        ::close(_fd);
        delete this;
    }

  private:
    int _fd;
};

#endif  // _ESFILEPVT_HPP_
