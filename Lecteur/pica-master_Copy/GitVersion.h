//
// Created by KarimSadki on 30/12/2020.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef PICA_GITVERSION_H
#define PICA_GITVERSION_H

#include <string>

class GitVersion {
public:
    static const std::string Revision;
    static const std::string Tag;
    static const std::string Branch;
};

#endif //PICA_GITVERSION_H
