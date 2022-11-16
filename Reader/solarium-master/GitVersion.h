//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/11/16.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_GITVERSION_H
#define SOLARIUM_GITVERSION_H

#include <string>

class GitVersion {
public:
    static const std::string Revision;
    static const std::string Tag;
    static const std::string Branch;
};

#endif //SOLARIUM_GITVERSION_H
