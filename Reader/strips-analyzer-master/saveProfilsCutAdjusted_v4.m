function saveProfilsCutAdjusted_v4(newpath, studyName, infosImages, figureName, fileNameSuffix, waitbarFigure, wbPercentStart, wbPercentRange)

    getAbscisses = @(infosImage)(1-infosImage.maxControlLine+infosImage.minStrip):(length(infosImage.cutProfilImage)-infosImage.maxControlLine+infosImage.minStrip);

    mergeProfilesIntoGraph(infosImages, studyName, ...
        newpath, figureName, fileNameSuffix, ...
        @(infoImage)(infoImage.profilFitExp), getAbscisses, ...
        waitbarFigure, wbPercentStart, wbPercentRange);
 
end