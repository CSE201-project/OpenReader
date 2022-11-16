function listOfBadCropped = checkCroppedImagesSize(infosImages, infosCroppedImages, cassette, numberOfPixels)
    listOfBadCropped = [];
    
    if contains(cassette,'K701')
        theoricCassetteWidth = 3.7;
    end
    
    marginMinimum = floor((0.19 * theoricCassetteWidth) * numberOfPixels);
    marginMaximum = floor((0.12 * theoricCassetteWidth) * numberOfPixels);
    theoricCroppedWidth = floor(theoricCassetteWidth * numberOfPixels);
    
    minimumAcceptableWidth = theoricCroppedWidth - marginMinimum;
    maximumAcceptableWidth = theoricCroppedWidth + marginMaximum;
    
    % To order the images by concentration.
    for i=1:length(infosImages)
        currentImageWidth = size(infosImages(i).rCroppedImage,2);
        nameImage = infosCroppedImages(i).name;
        
        if currentImageWidth < minimumAcceptableWidth | currentImageWidth > maximumAcceptableWidth
            listOfBadCropped = [listOfBadCropped; string(nameImage)];
        end
    end
end