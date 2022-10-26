function computePicaAnalysis(lightningCondition, channel, croppedFolderPath, padding, rectangleWidth, mask, referenceNumberOfPixels, distanceCLtoTLMm, lengthTLMm, tmpExcelFilePath, initialParams, appVersion, excelFileCoeffs, proxNoiseTL, proxNoiseNoise)
    
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    execPath = strcat(currentfunctionpath,'\PicaRunner\PicaRunnerAnalyzeUseTest.exe');
    templateExcelFile='AnalysisResults_ModelFileSandwich_v4.xlsx';
    
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    templateFilePath=strcat(currentfunctionpath, '\', templateExcelFile);
    
    if ~isfile(excelFileCoeffs)
        copyfile(strcat(currentfunctionpath, '\Coefficients courbes calibrationV4.xlsx'), excelFileCoeffs);
    end
    
    paddingStr = num2str(padding);
    rectangleWidthStr = num2str(rectangleWidth);
    maskStr = num2str(mask);
    referenceNumberOfPixelsStr = num2str(referenceNumberOfPixels);
    distanceCLtoTLMmStr = num2str(distanceCLtoTLMm);
    lengthTLMmStr = num2str(lengthTLMm);
    proxNoiseTLStr = num2str(proxNoiseTL);
    proxNoiseNoiseStr = num2str(proxNoiseNoise);
    excelPath = convertStringsToChars(tmpExcelFilePath);
    
    meas = convertStringsToChars(initialParams(2));
    prototype = convertStringsToChars(initialParams(3));
    cassette = convertStringsToChars(initialParams(4));
    focal = convertStringsToChars(initialParams(6));
    nanoparticules = convertStringsToChars(initialParams(7));

    commandLine = ['"',execPath, '" "', croppedFolderPath,'" "', templateFilePath, '" "', excelPath, '" "', appVersion, '" "', date, '" "', meas,'" "', prototype,'" "', cassette,'" "', focal,'" "', nanoparticules,'" "' , lightningCondition, '" "', channel, '" ', referenceNumberOfPixelsStr, ' ', paddingStr, ' ', rectangleWidthStr, ' ', maskStr, ' ', distanceCLtoTLMmStr, ' ', lengthTLMmStr, ' ', proxNoiseTLStr,' ', proxNoiseNoiseStr,' "',excelFileCoeffs,'"'];
    system(commandLine);

end