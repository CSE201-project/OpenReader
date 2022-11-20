//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/03.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_STUDYSNAPSHOTRESULT_H
#define SOLARIUM_STUDYSNAPSHOTRESULT_H

#include <string>
#include <vector>

class StudySnapshotResult {
public:
    std::string StudyName;
    std::string FileName;
    std::string Params;
    std::string SensorsInfo;
    std::string CassetteName;
    std::vector<uint8_t> Snapshot;
    bool IsCroppingSnapshot;
};


#endif //SOLARIUM_STUDYSNAPSHOTRESULT_H
