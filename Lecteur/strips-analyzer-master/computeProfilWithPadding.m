function [profil] = computeProfilWithPadding(image,padding, lightningCondition, referenceNumberOfPixels)
    currentSize = size(image,2);
    paddingPx = floor(referenceNumberOfPixels * padding);
    nbPxToDelete = floor((currentSize - paddingPx)/2);
    if nbPxToDelete < 1
        nbPxToDelete = 1;
    end
    
    paddedImage = image(:,nbPxToDelete:currentSize-nbPxToDelete);
    profil = mean(paddedImage,2);
    if strcmp(lightningCondition,'Visible')
        profil = [255] - profil;
    end
end

