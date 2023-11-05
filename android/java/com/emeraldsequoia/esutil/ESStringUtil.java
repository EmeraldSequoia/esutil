//
//  ESStringUtil.java
//
//  Created by Steve Pucci 15 Oct 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//

package com.emeraldsequoia.esutil;

public class ESStringUtil {
    public static String removeFinalCharacterFromString(String s) {
        if (s.length() == 0) {
            return s;
        }
        if (Character.isLowSurrogate(s.charAt(s.length() - 1))) {
            return s.substring(0, s.length() - 2);
        } else {
            return s.substring(0, s.length() - 1);
        }
    }
}
