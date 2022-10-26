//
// Created by KarimSadki on 17/12/2020.
//

#ifndef PICA_MULTIPLEXSTRUCTS_H
#define PICA_MULTIPLEXSTRUCTS_H

typedef int ColumnIndex;
typedef int LineIndex;
typedef std::string SpotType;

/* For the CAS spots, the line index precise the number of lines between the current CAS spot and the landmark.
 * For the test spots, the line index precise the number of lines between the current test spot and its CAS spot.*/
struct Pattern{
    std::map<ColumnIndex, LineIndex> SpotsCAS;
    std::map<ColumnIndex, std::vector<std::tuple<LineIndex, SpotType>>> SpotsTest;
    std::set<SpotType> SpotsSpecies;
};

struct SpotLocation{
    cv::Point point;
    ColumnIndex colIndexInPattern;
    LineIndex lineIndexInPattern;
};

typedef std::tuple<SpotLocation, SpotType> SpotTest;

struct GridOfSpots{
    std::vector<ColumnIndex> ColumnsOrderInImage;
    std::map<ColumnIndex, SpotLocation> SpotsCASCoordinates;
    std::map<ColumnIndex, std::vector<SpotTest>> SpotsTestCoordinates;
};

#endif //PICA_MULTIPLEXSTRUCTS_H
