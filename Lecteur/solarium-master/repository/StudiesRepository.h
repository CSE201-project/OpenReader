//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/02/03.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#ifndef SOLARIUM_STUDIESREPOSITORY_H
#define SOLARIUM_STUDIESREPOSITORY_H

#define DEFAULT_STUDIES_PATH "/mnt/update/studies"

#include <string>
#include "measures/StudySnapshotResult.h"
#include "IRepository.h"

class StudiesRepository : public IRepository {
public:
    explicit StudiesRepository(const std::string & studiesPath = DEFAULT_STUDIES_PATH);

    void save(const std::string &studyName,
              const std::string &studyDate,
              uint8_t paramIndex,
              const StudySnapshotResult &result);

    void saveParamsFinder(const std::string &sequenceName, const std::string &filename, const std::vector<uint8_t> snapshot);
    void saveParamsFinder(const std::string &sequenceName, const std::string &filename, std::vector<std::tuple<int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, std::vector<double>, std::vector<double>>> values);

    void saveParamsFinder(const std::string &sequenceName,
                          const std::string &filename,
                          const std::vector<std::string> & headers,
                          std::vector<std::tuple<std::vector<int>, int32_t, int32_t, std::vector<double>, std::vector<double>>> values);
    void saveParamsFinder(const std::string &sequenceName, const std::string &filename, std::vector<std::tuple<uint32_t, int32_t, int32_t, std::vector<double>>> values);

    void remove(const std::string & studyName, const std::string &studyDate, const std::string & fileName);

    void saveSequenceSnap(const std::string sequenceName, uint32_t index, const std::vector<uint8_t> snapshot);

    uint32_t getFreeSpace() const;

    static bool isStudyNameDerived(const std::string & studyName, const std::string & name);

};


#endif //SOLARIUM_STUDIESREPOSITORY_H
