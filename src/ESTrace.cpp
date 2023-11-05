//
//  ESTrace.cpp
//
//  Created by Steve Pucci 15 May 2011 based on ECTrace.m by Bill Arnett in chronometer app
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#define ESTRACE

#include "ESTrace.hpp"
#include "ESThread.hpp"
#include "ESThreadLocalStorage.hpp"

#include <stdio.h>

static ESThreadLocalStorageScalar<long> _traceIndent;

static int
traceIndentForThisThread() {
    return (int)_traceIndent;
}

static void
incrementTraceIndentInThisThread() {
    _traceIndent = (int)_traceIndent + 1;
}

static void
decrementTraceIndentInThisThread() {
    _traceIndent = (int)_traceIndent - 1;
}

std::string
traceTabsAndThread() {
    std::string ret = "Thread '";
    ret += ESThread::currentThread()->name();
    ret += "' ";
    int i = traceIndentForThisThread();
    while (i-- > 0) {
        ret += "  ";
    }
    return ret;
}

void
traceEnter(const char *msg) {
    std::string str = traceTabsAndThread();
    str += msg;
    str += " enter";
    ESTRACE_PRINTER(str);
    incrementTraceIndentInThisThread();
}

void
traceExit(const char *msg) {
    decrementTraceIndentInThisThread();
    std::string str = traceTabsAndThread();
    str += msg;
    str += " exit";
    ESTRACE_PRINTER(str);
}

