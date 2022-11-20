//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_SIMPLE_LOGGING_H
#define SOLARIUM_SIMPLE_LOGGING_H

#include <iostream>
#include <cstdlib>

#define __format_str__(x, ...) {char buf[1024]; sprintf(buf, __VA_ARGS__);std::cerr<<x<<buf<<std::endl;}
#define qWarning(...) __format_str__("[WARNING] ", __VA_ARGS__)
#define qCritical(...) __format_str__("[CRITICAL] ", __VA_ARGS__)


#endif //SOLARIUM_SIMPLE_LOGGING_H
