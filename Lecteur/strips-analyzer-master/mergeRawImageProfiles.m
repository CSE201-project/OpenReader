function mergeRawImageProfiles(rawImagesPath,essayNumber,infosImages, fileNameSuffix, lightningCondition)


    getProfile = @(infoImage)(mean(infoImage.rCroppedImage,2));
    if strcmp(lightningCondition,'Visible')
        getProfile = @(infoImage)([255] - mean(infoImage.rCroppedImage,2));
    end

    mergeProfilesIntoGraph(infosImages, essayNumber, ...
        rawImagesPath, 'Profils bruts', fileNameSuffix, ...
        getProfile, @(infoImage)(1:length(infoImage.rCroppedImage)), ...
        '');
    
end
