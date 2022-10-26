//
// Created by Mario Valdivia on 2020/06/24.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_EXCEPTION_H
#define SOLARIUM_EXCEPTION_H

#include <sstream>
#include <exception>

#define THROWEX(msg) { \
    std::ostringstream oss; \
    oss << msg; \
    throw std::runtime_error(oss.str()); \
}

#define __str__(x) #x
#define __xstr__(x) __str__(x)

#endif //SOLARIUM_EXCEPTION_H
