//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/07/06.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_DEBUGINFO_H
#define SOLARIUM_DEBUGINFO_H

#include <iostream>

#ifdef ENABLE_DEBUG_MACRO


#define __format_debug__(type, ...) {char buf[1024]; sprintf(buf, __VA_ARGS__);std::cerr<<"[" type "] " << __FILE__ << " (" << __LINE__ << ") : "<<buf<<std::endl;}

//#define __format_str__(x, ...) {char buf[1024]; sprintf(buf, __VA_ARGS__);std::cerr<<x<<buf<<std::endl;}
#define qWarning(...) __format_debug__("[WARNING] ", __VA_ARGS__)
#define qCritical(...) __format_debug__("[CRITICAL] ", __VA_ARGS__)
#define qDebug(...) __format_debug__("[DEBUG]", __VA_ARGS__)


#define debugLog(msg) {std::cerr<<"[DEBUG] " << __FILE__ << " (" << __LINE__ << ") : "<< msg <<std::endl;}
#define errorLog(msg) {std::cerr<<"[ERROR] " << __FILE__ << " (" << __LINE__ << ") : "<< msg <<std::endl;}


#else

#define qWarning(msg)
#define qCritical(msg)
#define debugLog(msg)

#endif


#endif //SOLARIUM_DEBUGINFO_H

