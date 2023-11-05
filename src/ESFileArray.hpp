//
//  ESFileArray.hpp
//
//  Created by Steven Pucci 21 Jul 2011
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESUTILFILES_HPP_
#define _ESUTILFILES_HPP_

#include "ESFile.hpp"  // For path type enum

/** A file array is a simple C array which is backed by an external file.   There are three use models:
 *  * Read the entire array in at once with a single kernel call
 *  * Read in a single element with lseek and read
 *  * Write an entire array to disk with a single kernel call
 */
template <class ElementType>
class ESFileArray {
  public:
    /** Construct the object, filling in the array with the external file's contents if the external file exists. */
                            ESFileArray(const char     *path,
                                        ESFilePathType pathType,
                                        bool           readInAtStartup = true);
                            ~ESFileArray();
    /** Return a readonly pointer to the internal array */
    operator                const ElementType *() const { return _array; }

    /** Return a readonly pointer to the internal array */
    const ElementType       *array() const { return _array; }

    /** The number of bytes read at construction (will be 0 if file didn't exist) */
    size_t                  bytesRead() const { return _bytesRead; }

    /** The number of elements read at construction (will be 0 if file didn't exist) */
    int                     numElements() const { return (long)(_bytesRead / sizeof(ElementType)); }

    /** Allocate an array of the given element size which can then be obtained by calling writableArray() */
    void                    setupForWriteWithNumElements(int numElements);

    /** Return a writable array previously obtained from either setupForWriteWithNumElements or by reading the external file during construction. */
    ElementType             *writableArray() { return _array; }

    /** Write the array previously filled in to the given external path.
     *  @return  true iff the write was successful. */
    bool                    writeToPath(const char     *path,
                                        ESFilePathType pathType);

    /** Read a single element from a file which hasn't been opened yet, and then close the file */
    static void             readElementFromFileAtIndex(const char     *path,
                                                       ESFilePathType pathType,
                                                       int            indx,
                                                       ElementType    *element);
  protected:
    ElementType             *_array;
    size_t                  _bytesRead;
};

/** This class is used to read in and store a large number of strings from a single file.
 */
class ESFileStringArray : protected ESFileArray<char> {
  public:
                            ESFileStringArray(const char     *path,
                                              ESFilePathType pathType,
                                              int            arraySizeGuess);
                            ~ESFileStringArray();
    const char              *stringAtIndex(int indx);
    const char              **strings() const { return _strings; }
    int                     numStrings() const { return (int)_numStrings; }

  private:
    const char              **_strings;
    long                    _numStrings;
    long                    _stringsCapacity;
};

#include "ESFileArrayInl.hpp"

#endif  // _ESUTILFILES_HPP_
