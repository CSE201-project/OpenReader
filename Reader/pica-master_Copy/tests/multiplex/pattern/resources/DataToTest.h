#ifndef PICA_DATATOTEST_H
#define PICA_DATATOTEST_H

#include "multiplex/pattern/PatternManager.h"

//Expected landmarks when we parse "Grids Locations.txt" with ratioNumberPixelsMm of 43.63 (proto 204)
static std::vector<SpotLocation> expectedLandmarks204 ={
            { cv::Point(32, 147), 2},
            {cv::Point(23, 123), 2},
            {cv::Point(36, 102), 4},
            {cv::Point(23, 117), 5},
            {cv::Point(16, 132), 1},
            {cv::Point(19, 136), 3},
            {cv::Point(40, 128), 6}
        };

//Expected landmarks when we parse "Grids Locations.txt" with ratioNumberPixelsMm of 139.66 (proto 301)
static std::vector<SpotLocation> expectedLandmarks301 ={
        { cv::Point(104, 474), 2},
        {cv::Point(76, 397), 2},
        {cv::Point(118, 327), 4},
        {cv::Point(76, 376), 5},
        {cv::Point(55, 425), 1},
        {cv::Point(62, 439), 3},
        {cv::Point(132, 411), 6}
};

static std::map<ColumnIndex, LineIndex> expectedSpotsCAS = {
        {1, 0},
        {2, 2},
        {3, 0}
};

static std::map<ColumnIndex, std::vector<std::tuple<LineIndex, SpotType>>> expectedSpotsTest = {
        {1, { {1, "Anti_Proteine_N"}, {2, "Anti_hlgG"}, {4, "Molecule_I"}, {5, "Anti_hlgM"}, {7, "KLH_colorant"}, {8, "Anti_hlgA"} } },
        {2, { {1, "Anti_hlgG"}, {2, "Anti_Proteine_N"}, {3, "Molecule_I"}, {5, "KLH_colorant"}, {6, "Anti_hlgM"}, {7, "Anti_hlgA"} } },
        {3, { {1, "Anti_hlgG"}, {2, "Molecule_I"}, {3, "Anti_Proteine_N"}, {4, "KLH_colorant"}, {6, "Anti_hlgM"}, {8, "Anti_hlgA"} } }
};

static std::set<SpotType> expectedSpotsSpecies = {
        "Anti_Proteine_N",
        "Anti_hlgG",
        "Molecule_I",
        "Anti_hlgM",
        "KLH_colorant",
        "Anti_hlgA"
};

#endif //PICA_DATATOTEST_H
