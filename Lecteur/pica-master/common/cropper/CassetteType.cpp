//
// Created by KarimSadki on 10/03/2021.
//

#include "CassetteType.h"

std::map<CassetteModel, CassetteType::CassetteDataToCrop> CassetteType::CassetteDataToCropMap =
        {{K701WT,
          {"MK001 White top", L, L, 3.7, false}},
         {K701BT,
          {"MK001 Black top", L, L, 3.7, true}}
        };

std::map<StripMatrix, bool> CassetteType::MatrixDataToCropMap =
        {{Serum, false}, {Blood, false}, {Tampon, false}, {Nasal, false}};

CassetteModel decodeCassetteModel(const std::string& cassette)
{
    if((cassette == "K701WT"))
        return K701WT;
    else if((cassette == "K701BT"))
        return K701BT;
    else
        return UnknownCassetteModel;
}

StripMatrix decodeStripMatrix(const std::string& stripMatrix)
{
    if(stripMatrix == "SE")
        return Serum;
    else if(stripMatrix == "BL")
        return Blood;
    else if(stripMatrix == "TP")
        return Tampon;
    else if (stripMatrix == "NL")
        return Nasal;
    else
        return UnknownMatrix;
}

CassetteData CassetteType::decodeCassetteName(const std::string& name)
{
    std::vector<std::string> dataFromName;

    std::string::size_type beg = 0;
    for (auto end = 0; (end = name.find("_", end)) != std::string::npos; ++end)
    {
        dataFromName.push_back(name.substr(beg, end - beg));
        beg = end + 1;
    }
    dataFromName.push_back(name.substr(beg));

    if(dataFromName.size() != 2)
        return {UnknownCassetteModel, UnknownMatrix};

    CassetteModel cassetteModel = decodeCassetteModel(dataFromName[0]);
    StripMatrix stripMatrix = decodeStripMatrix(dataFromName[1]);

    return {cassetteModel, stripMatrix};
}