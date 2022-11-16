function saveProfilsCut_v4(newpath, studyName, infosImages, fileNameSuffix, filenameExcel, waitbarFigure, wbPercentStart, wbPercentRange)

    mergeProfilesIntoGraph(infosImages, studyName, ...
        newpath, 'Profils bruts coupés', fileNameSuffix, ...
        @(infoImage)(infoImage.cutProfilImage), @(infoImage)(1:length(infoImage.cutProfilImage)), ...
        waitbarFigure, wbPercentStart, wbPercentRange);
    
end