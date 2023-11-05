//
//  ESTrace.hpp
//
//  Created by Steve Pucci 15 May 2011, based on ECTrace.h by Bill Arnett from chronometer app
//  Copyright Emerald Sequoia LLC 2011. All rights reserved.
//

#ifndef _ESTRACE_HPP_
#define _ESTRACE_HPP_

#include "ESUtil.hpp"

#include <string>

#ifdef ESTRACE
extern std::string traceTabsAndThread();
extern void traceEnter(const char *fmt);
#define traceEnter1(fmt,p1) { traceEnter(ESUtil::stringWithFormat(fmt,p1).c_str()); }
#define traceEnter2(fmt,p1,p2) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2).c_str()); }
#define traceEnter3(fmt,p1,p2,p3) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3).c_str()); }
#define traceEnter4(fmt,p1,p2,p3,p4) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4).c_str()); }
#define traceEnter5(fmt,p1,p2,p3,p4,p5) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5).c_str()); }
#define traceEnter6(fmt,p1,p2,p3,p4,p5,p6) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6).c_str()); }
#define traceEnter7(fmt,p1,p2,p3,p4,p5,p6,p7) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7).c_str()); }
#define traceEnter8(fmt,p1,p2,p3,p4,p5,p6,p7,p8) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7,p8).c_str()); }
#define traceEnter9(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9) { traceEnter(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9).c_str()); }
extern void traceExit(const char *fmt);
#define traceExit1(fmt,p1) { traceExit(ESUtil::stringWithFormat(fmt,p1).c_str()); }
#define traceExit2(fmt,p1,p2) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2).c_str()); }
#define traceExit3(fmt,p1,p2,p3) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3).c_str()); }
#define traceExit4(fmt,p1,p2,p3,p4) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4).c_str()); }
#define traceExit5(fmt,p1,p2,p3,p4,p5) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5).c_str()); }
#define traceExit6(fmt,p1,p2,p3,p4,p5,p6) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6).c_str()); }
#define traceExit7(fmt,p1,p2,p3,p4,p5,p6,p7) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7).c_str()); }
#define traceExit8(fmt,p1,p2,p3,p4,p5,p6,p7,p8) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7,p8).c_str()); }
#define traceExit9(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9) { traceExit(ESUtil::stringWithFormat(fmt,p1,p2,p3,p4,p5,p6,p7,p8,p9).c_str()); }
#undef ESTRACE_NTAP
#ifdef ESTRACE_NTAP
#define ESTRACE_PRINTER(str) ESUtil::noteTimeAtPhase(str)
#else
#define ESTRACE_PRINTER(str) ESErrorReporter::logInfo("TRACE", "%s\n", str.c_str())
#endif
#define tracePrintf(f)                    { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f);                   ESTRACE_PRINTER(str); }
#define tracePrintf1(f,p1)                 { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1);                 ESTRACE_PRINTER(str); }
#define tracePrintf2(f,p1,p2)               { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2);               ESTRACE_PRINTER(str); }
#define tracePrintf3(f,p1,p2,p3)             { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3);             ESTRACE_PRINTER(str); }
#define tracePrintf4(f,p1,p2,p3,p4)           { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4);           ESTRACE_PRINTER(str); }
#define tracePrintf5(f,p1,p2,p3,p4,p5)         { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5);         ESTRACE_PRINTER(str); }
#define tracePrintf6(f,p1,p2,p3,p4,p5,p6)       { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6);       ESTRACE_PRINTER(str); }
#define tracePrintf7(f,p1,p2,p3,p4,p5,p6,p7)     { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7);     ESTRACE_PRINTER(str); }
#define tracePrintf8(f,p1,p2,p3,p4,p5,p6,p7,p8)   { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7,p8);   ESTRACE_PRINTER(str); }
#define tracePrintf9(f,p1,p2,p3,p4,p5,p6,p7,p8,p9) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7,p8,p9); ESTRACE_PRINTER(str); }
#define traceAngle(a,f) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle1(a,f,p1) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle2(a,f,p1,p2) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle3(a,f,p1,p2,p3) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle4(a,f,p1,p2,p3,p4) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle5(a,f,p1,p2,p3,p4,p5) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle6(a,f,p1,p2,p3,p4,p5,p6) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle7(a,f,p1,p2,p3,p4,p5,p6,p7) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle8(a,f,p1,p2,p3,p4,p5,p6,p7,p8) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7,p8).c_str());  ESTRACE_PRINTER(str); }
#define traceAngle9(a,f,p1,p2,p3,p4,p5,p6,p7,p8,p9) { std::string str = traceTabsAndThread(); str += ESUtil::stringWithFormat("%s  %s", ESUtil::angleString(a).c_str(), ESUtil::stringWithFormat(f,p1,p2,p3,p4,p5,p6,p7,p8,p9).c_str());  ESTRACE_PRINTER(str); }
#else
#define traceTab() {;}
#define traceEnter(f) {;}
#define traceEnter1(f,p1) {;}
#define traceEnter2(f,p1,p2) {;}
#define traceEnter3(f,p1,p2,p3) {;}
#define traceEnter4(f,p1,p2,p3,p4) {;}
#define traceEnter5(f,p1,p2,p3,p4,p5) {;}
#define traceEnter6(f,p1,p2,p3,p4,p5,p6) {;}
#define traceEnter7(f,p1,p2,p3,p4,p5,p6,p7) {;}
#define traceEnter8(f,p1,p2,p3,p4,p5,p6,p7,p8) {;}
#define traceEnter9(f,p1,p2,p3,p4,p5,p6,p7,p8,p9) {;}
#define traceExit(f) {;}
#define traceExit1(f,p1) {;}
#define traceExit2(f,p1,p2) {;}
#define traceExit3(f,p1,p2,p3) {;}
#define traceExit4(f,p1,p2,p3,p4) {;}
#define traceExit5(f,p1,p2,p3,p4,p5) {;}
#define traceExit6(f,p1,p2,p3,p4,p5,p6) {;}
#define traceExit7(f,p1,p2,p3,p4,p5,p6,p7) {;}
#define traceExit8(f,p1,p2,p3,p4,p5,p6,p7,p8) {;}
#define traceExit9(f,p1,p2,p3,p4,p5,p6,p7,p8,p9) {;}
#define tracePrintf(f) {;}
#define tracePrintf1(f,p1) {;}
#define tracePrintf2(f,p1,p2) {;}
#define tracePrintf3(f,p1,p2,p3) {;}
#define tracePrintf4(f,p1,p2,p3,p4) {;}
#define tracePrintf5(f,p1,p2,p3,p4,p5) {;}
#define tracePrintf6(f,p1,p2,p3,p4,p5,p6) {;}
#define tracePrintf7(f,p1,p2,p3,p4,p5,p6,p7) {;}
#define tracePrintf8(f,p1,p2,p3,p4,p5,p6,p7,p8) {;}
#define tracePrintf9(f,p1,p2,p3,p4,p5,p6,p7,p8,p9) {;}
#define traceAngle(a,f) {;}
#define traceAngle1(a,f,p1) {;}
#define traceAngle2(a,f,p1,p2) {;}
#define traceAngle3(a,f,p1,p2,p3) {;}
#define traceAngle4(a,f,p1,p2,p3,p4) {;}
#define traceAngle5(a,f,p1,p2,p3,p4,p5) {;}
#define traceAngle6(a,f,p1,p2,p3,p4,p5,p6) {;}
#define traceAngle7(a,f,p1,p2,p3,p4,p5,p6,p7) {;}
#define traceAngle8(a,f,p1,p2,p3,p4,p5,p6,p7,p8) {;}
#define traceAngle9(a,f,p1,p2,p3,p4,p5,p6,p7,p8,p9) {;}
#endif

#endif  // _ESTRACE_HPP_
