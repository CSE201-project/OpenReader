function computeMultiplexAnalysis(lightningCondition, channel, croppedFolderPath, referenceNumberOfPixels, tmpExcelFilePath, initialParams, appVersion, patternFilePath, tmpFileGridsLocationsPath, spotDiameterToAnalyze)
    
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerAnalyzeMultiplex.exe');

    
    templateExcelFile='AnalysisResults_ModelFileMultiplex.xlsx';
    templateFilePath=strcat(currentfunctionpath, '\', templateExcelFile);
    
    spotDiameterToAnalyzeStr = num2str(spotDiameterToAnalyze);
    referenceNumberOfPixelsStr = num2str(referenceNumberOfPixels);
    excelPath = convertStringsToChars(tmpExcelFilePath);
    
    meas = convertStringsToChars(initialParams(2));
    prototype = convertStringsToChars(initialParams(3));
    cassette = convertStringsToChars(initialParams(4));
    focal = convertStringsToChars(initialParams(6));
    nanoparticules = convertStringsToChars(initialParams(7));

    commandLine = ['"',execPath, '" "', croppedFolderPath,'" "', templateFilePath, '" "', excelPath, '" "', appVersion, '" "', date, '" "', meas,'" "', prototype,'" "', cassette,'" "', focal,'" "', nanoparticules,'" "' , lightningCondition, '" "', channel, '" ', referenceNumberOfPixelsStr, ' "',patternFilePath,'" "', tmpFileGridsLocationsPath,'" ',spotDiameterToAnalyzeStr];
    system(commandLine);

end