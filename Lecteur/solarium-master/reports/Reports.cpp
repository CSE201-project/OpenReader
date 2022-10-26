//
// Created by Mario Valdivia <mario@lumedix.com> on 2021/07/01.
// Copyright (c) 2021 LumediX. All rights reserved.
//

#include "Reports.h"
#include <hpdf.h>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <memory>
#include <SolariumApp.h>
#include <device/SolariumDevice.h>
#include <repository/AnalysisRepository.h>
#include <measures/AnalysisResult.h>

#include "configure.h"

#ifdef USE_DUMMY_PERIPHERALS
#define RESOURCE_PATH "../Assets"
#else
#define RESOURCE_PATH "/opt/Solarium/media"
#endif


struct OverviewData {
    std::string DeviceName;
    std::string SoftwareVersion;
    std::string Date;
    std::string PatientId;
    std::string MatrixType;
    std::string LotNumber;
    std::string Result;
    std::string Concentration;
};

std::string to_string(float num, int nbDecimals)
{
    std::ostringstream oss;
    oss.precision(nbDecimals);
    oss << std::fixed << num;
    return oss.str();
}

void customErrorHandler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data)
{
    std::ostringstream oss;
    oss << "PDF Error " << error_no << " detail " << detail_no;

    poco_error(LOGGER, oss.str());
    throw new std::runtime_error(oss.str());
}

std::vector<uint8_t> pdfStreamToVector(HPDF_Doc pdf)
{
    std::vector<uint8_t> resultingFile(HPDF_GetStreamSize(pdf));

    HPDF_SaveToStream(pdf);
    HPDF_ResetStream (pdf); //* rewind the stream.

    // * get the data from the stream and output it to stdout.
    for (;;) {
        HPDF_BYTE buf[4096];
        HPDF_UINT32 siz = 4096;
        HPDF_STATUS ret = HPDF_ReadFromStream (pdf, buf, &siz);

        if (siz == 0)
            break;

        resultingFile.insert(resultingFile.end(), buf, buf + siz);
    }
    return resultingFile;
}

void drawImage (HPDF_Doc     pdf,
                const std::string &filename,
                float        x,
                float        y)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);
    HPDF_Image image;

    image = HPDF_LoadPngImageFromFile(pdf, filename.c_str());

    // Draw image to the canvas.
    HPDF_Page_DrawImage(page, image, x, y, HPDF_Image_GetWidth(image),
                        HPDF_Image_GetHeight(image));
}

void drawImage (HPDF_Doc     pdf,
                const std::vector<uint8_t> &picture,
                float        x,
                float        y,
                std::optional<uint32_t> width = {})
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);
    HPDF_Image image;

    image = HPDF_LoadPngImageFromMem(pdf, picture.data(), picture.size());

    auto imageSize = HPDF_Image_GetSize(image);
    if (width.has_value()) {
        auto ratio = width.value() / imageSize.x;
        imageSize.x *= ratio;
        imageSize.y *= ratio;
    }
    // Draw image to the canvas.
    HPDF_Page_DrawImage(page, image, x, y, imageSize.x, imageSize.y);
}

void drawImageFromTL (HPDF_Doc     pdf,
                const std::vector<uint8_t> &picture,
                float        x,
                float        y,
                std::optional<uint32_t> width = {})
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);
    HPDF_Image image;

    image = HPDF_LoadPngImageFromMem(pdf, picture.data(), picture.size());

    auto imageSize = HPDF_Image_GetSize(image);
    if (width.has_value()) {
        auto ratio = width.value() / imageSize.x;
        imageSize.x *= ratio;
        imageSize.y *= ratio;
    }
    // Draw image to the canvas.
    HPDF_Page_DrawImage(page, image, x, y - imageSize.y, imageSize.x, imageSize.y);
}

void drawText(HPDF_Doc     pdf,
              const std::string &text,
              float        x,
              float        y)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);

    // Print the text.
    HPDF_Page_BeginText (page);
    HPDF_Page_SetTextLeading (page, 16);
    HPDF_Page_MoveTextPos (page, x, y);
    HPDF_Page_ShowTextNextLine (page, text.c_str());
    HPDF_Page_EndText (page);
}

void drawHeader(HPDF_Doc pdf)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);

    auto font = HPDF_GetFont (pdf, "Helvetica", NULL);

    HPDF_Page_BeginText (page);
    HPDF_Page_SetFontAndSize (page, font, 80);
    HPDF_Page_MoveTextPos (page, 800, HPDF_Page_GetHeight (page) - 250);
    HPDF_Page_ShowText (page, "Analysis Report");
    HPDF_Page_EndText (page);

    drawImage(pdf, RESOURCE_PATH "/LogoLumedix.png", 100, HPDF_Page_GetHeight(page) - 280);
}

void drawTextGrid(HPDF_Page page, HPDF_Font font, const std::vector<std::pair<std::string, std::string>> & texts, float x, float y, float x2 = 600)
{
    for (const auto & kv : texts)
    {
        HPDF_Page_BeginText (page);
        HPDF_Page_SetTextLeading (page, 40);
        HPDF_Page_SetFontAndSize (page, font, 40);

        HPDF_Page_MoveTextPos (page, x, y);
        HPDF_Page_ShowText (page, kv.first.c_str());

        HPDF_Page_MoveTextPos (page, x2, 0);
        HPDF_Page_ShowText (page, kv.second.c_str());

        HPDF_Page_EndText (page);
        y -= 60;
    }
}

void drawResultText(HPDF_Doc pdf, HPDF_Font font, const std::string & result, const std::string & concentration, float x, float y, float x2 = 600)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);
    auto font2 = HPDF_GetFont (pdf, "Helvetica-Bold", NULL);

    {
        HPDF_Page_BeginText (page);
        HPDF_Page_SetTextLeading (page, 40);
        HPDF_Page_SetFontAndSize (page, font, 40);

        HPDF_Page_MoveTextPos (page, x, y);
        HPDF_Page_ShowText (page, TXT_RESULT);

        HPDF_Page_MoveTextPos (page, x2, 0);
        HPDF_Page_SetFontAndSize (page, font, 50);

        float R = 0.0, G = 0.0, B = 0.0;
        if (result == TXT_POSITIVE) {
            R = 1.0;
        } else {
            R = 48.0/255.0;
            G = 187.0/255.0;
            B = 31.0/255.0;
        }
        HPDF_Page_SetRGBFill(page, R, G, B);

        HPDF_Page_ShowText (page, result.c_str());

        HPDF_Page_EndText (page);
    }
    {
        HPDF_Page_SetRGBFill(page, 0, 0, 0);
        /*HPDF_Page_BeginText (page);
        HPDF_Page_SetFontAndSize (page, font, 40);

        HPDF_Page_MoveTextPos (page, x, y - 60);
        HPDF_Page_ShowText(page, TXT_CONCENTRATION1);
        HPDF_Page_ShowTextNextLine(page, TXT_CONCENTRATION2);

        HPDF_Page_MoveTextPos (page, x2, 0);
        auto tmp = concentration;
        if (result == TXT_POSITIVE) tmp += " pg/mL";
        HPDF_Page_ShowText (page, tmp.c_str());

        HPDF_Page_EndText (page);*/
    }
}

void drawOverview(HPDF_Doc pdf, const OverviewData & overviewData)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);

    auto font = HPDF_GetFont (pdf, "Helvetica", NULL);

    /* Print the lines of the page. */
    HPDF_Page_SetLineWidth (page, 1);
    HPDF_Page_Rectangle (page, 50, 50, 2000, 2870);
    HPDF_Page_Stroke (page);

    float leftBorder = LEFT_BORDER, topBorder = 2500;

    std::vector<std::pair<std::string, std::string>> patientInfo = {
            {TXT_PATIENT_ID,  overviewData.PatientId},
            {TXT_STRIP_TYPE,  overviewData.MatrixType},
            {TXT_STRIP_BATCH, overviewData.LotNumber},
            {TXT_DATE,        overviewData.Date}
    };

    drawTextGrid(page, font, patientInfo, leftBorder, topBorder);

    std::vector<std::pair<std::string, std::string>> cartoucheTexts = {
            {TXT_DEVICE_SERIAL, overviewData.DeviceName},
            {TXT_SW_VERSION,    overviewData.SoftwareVersion}
    };

    drawTextGrid(page, font, cartoucheTexts, leftBorder, topBorder - (5*60));

    drawResultText(pdf, font, overviewData.Result, overviewData.Concentration, leftBorder, topBorder - (8*60));

}


void drawAlgosGrid(HPDF_Doc pdf, const AnalysisResult * result)
{
    HPDF_Page page = HPDF_GetCurrentPage(pdf);

    auto font = HPDF_GetFont (pdf, "Helvetica", NULL);

    float leftBorder = LEFT_BORDER, rightBorder = 1870, topBorder = 800, borderMargin = 10;

    /**/
    HPDF_Page_BeginText (page);
    HPDF_Page_SetTextLeading (page, 40);
    HPDF_Page_SetFontAndSize (page, font, 40);
    HPDF_Page_MoveTextPos (page, leftBorder, topBorder);
    HPDF_Page_ShowText (page, TXT_QUALITY_CONTROL);
    HPDF_Page_EndText (page);

    topBorder -= 30;

    /* Print the lines of the page. */
    HPDF_Page_SetLineWidth (page, 2.);
    HPDF_Page_MoveTo(page, leftBorder - borderMargin, topBorder);
    HPDF_Page_LineTo(page, rightBorder + borderMargin, topBorder);
    HPDF_Page_Stroke (page);

    float x = leftBorder, y = topBorder + borderMargin;
    for (int i = 0; i < result->AllResults.size(); i++)
    {
        //y -= 60;

        const auto & algo = result->AllResults.at(i);
        HPDF_Page_BeginText (page);
        HPDF_Page_SetTextLeading (page, 40);
        HPDF_Page_SetFontAndSize (page, font, 40);


        std::ostringstream oss;
        oss << "a" << i;

        HPDF_Rect textRect = {
                .left = x,
                .bottom = y - 60,
                .right = x + 150,
                .top = y - 2*borderMargin
        };


        HPDF_Page_TextRect(page, textRect.left, textRect.top, textRect.right, textRect.bottom, oss.str().c_str(), HPDF_TALIGN_RIGHT, nullptr);

        textRect.left += 200;
        textRect.right = textRect.left + 350;
        HPDF_Page_TextRect(page, textRect.left, textRect.top, textRect.right, textRect.bottom, to_string(algo.CutOff, 4).c_str(), HPDF_TALIGN_RIGHT, nullptr);

        textRect.left += 350;
        textRect.right = textRect.left + 350;
        HPDF_Page_TextRect(page, textRect.left, textRect.top, textRect.right, textRect.bottom, to_string(algo.Value, 4).c_str(), HPDF_TALIGN_RIGHT, nullptr);

        textRect.left += 350;
        textRect.right = textRect.left + 300;
        HPDF_Page_TextRect(page, textRect.left, textRect.top, textRect.right, textRect.bottom, to_string(algo.Quantity.p1, 0).c_str(), HPDF_TALIGN_RIGHT, nullptr);

        textRect.left += 300;
        textRect.right = textRect.left + 300;
        HPDF_Page_TextRect(page, textRect.left, textRect.top, textRect.right, textRect.bottom, to_string(algo.Quantity.p2, 0).c_str(), HPDF_TALIGN_RIGHT, nullptr);

        y -= 60;
        /*
        HPDF_Page_MoveTextPos (page, x, y);
        HPDF_Page_ShowText (page, oss.str().c_str());

        HPDF_Page_MoveTextPos (page, 150, 0);
        HPDF_Page_ShowText (page, to_string(algo.CutOff, 4).c_str());

        HPDF_Page_MoveTextPos (page, 300, 0);
        HPDF_Page_ShowText (page, to_string(algo.Value, 4).c_str());

        HPDF_Page_MoveTextPos (page, 300, 0);
        HPDF_Page_ShowText (page, to_string(algo.Quantity.p1, 0).c_str());

        HPDF_Page_MoveTextPos (page, 300, 0);
        HPDF_Page_ShowText (page, to_string(algo.Quantity.p2, 0).c_str());

         */


        HPDF_Page_EndText (page);

        HPDF_Page_SetLineWidth (page, 1.0);
        HPDF_Page_MoveTo(page, leftBorder - borderMargin, y - 15);
        HPDF_Page_LineTo(page, rightBorder + borderMargin, y - 15);
        HPDF_Page_Stroke (page);
    }

    /* Print the lines of the page. */
    HPDF_Page_SetLineWidth (page, 2.);
    HPDF_Page_MoveTo(page, leftBorder - borderMargin, y - 15);
    HPDF_Page_LineTo(page, rightBorder + borderMargin, y - 15);
    HPDF_Page_Stroke (page);

}

std::vector<uint8_t> Reports::createReport(const AnalysisResult * result)
{
    HPDF_Doc pdf;
    std::vector<uint8_t> resultingPdf;

    pdf = HPDF_New(customErrorHandler, NULL);
    if (!pdf) throw new std::runtime_error("Could not create PDF Object");

    try {

        HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
        HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_USE_OUTLINE);

        // create default-font
        auto font = HPDF_GetFont (pdf, "Helvetica", NULL);

        auto page = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

        HPDF_Page_SetWidth (page, 2100);
        HPDF_Page_SetHeight (page, 2970);

        auto dst = HPDF_Page_CreateDestination (page);
        HPDF_Destination_SetXYZ (dst, 0, HPDF_Page_GetHeight (page), 1);
        HPDF_SetOpenAction(pdf, dst);

        drawHeader(pdf);

        OverviewData overviewData = {
                .DeviceName = SolariumDevice::instance().getHostname(),
                .SoftwareVersion = SolariumApp::version(),
                .Date = Poco::DateTimeFormatter::format(result->Date, "%d/%m/%Y %H:%M:%S"),
                .PatientId = result->PatientId,
                .MatrixType = result->StripType,
                .LotNumber = result->StripBatchId,
                .Result = "N/A"
        };

        if (result->AllResults.size() > 0)
        {
            if (result->AllResults.at(0).Value >= result->AllResults.at(0).CutOff)
            {
                overviewData.Result = TXT_POSITIVE;
                try {
                    auto concentration = std::round(result->AllResults.at(0).Quantity.p1 * 10) / 10;
                    overviewData.Concentration = to_string(concentration, 0);
                }
                catch (const std::exception &) {
                    overviewData.Concentration = "N/A";
                }
            }
            else {
                overviewData.Result = TXT_NEGATIVE;
                overviewData.Concentration = "N/A";
            }
        }

        drawOverview(pdf, overviewData);

        drawImageFromTL(pdf, result->StripImage, LEFT_BORDER, 1800, 1640);

        drawAlgosGrid(pdf, result);

        HPDF_Page_SetFontAndSize (page, font, 12);

        resultingPdf = pdfStreamToVector(pdf);
    }
    catch (const std::exception & ex)
    {

    }

    HPDF_Free(pdf);
    return resultingPdf;
}

std::vector<std::string> Reports::createFullExport()
{
    auto allResults = AnalysisRepository::exportAllAnalysis();
    return allResults;
}
