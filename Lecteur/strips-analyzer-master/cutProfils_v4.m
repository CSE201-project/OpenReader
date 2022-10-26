function [infosImages]=cutProfils_v4(infosImages,i,zmin,zmax, padding, halfTLLength, lightningCondition, positionLastTestLine, referenceNumberOfPixels, maskSize)

% CUTPROFIL_v4 :
%   
%   Version of the function cutProfil adapted to the third version
%   of stripsAnalysis. This function cuts the extremities of the profil.
%
%
% INPUT PARAMETERS : 
%   
%   INFOSIMAGES : structure with main information on each images.
%
%   i : indice of the image to be analyzed.
%
%   ZMIN : minimum of the control line research area.
%
%   ZMAX : maximum of the control line research area.
%
%
% OUTPUT PARAMETERS :
%
%   INFOSIMAGES : structure with main et new information on each images.
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  


    % Get the profil
    imagecropped=infosImages(i).rCroppedImage;
    
    profil = computeProfilWithPadding(imagecropped, padding, lightningCondition, referenceNumberOfPixels);
    
    % Derive the profil
    derivatedImage=diff(profil);

    % Get the maximum of the derivated profil.
    [~,maxDerive]=(max(derivatedImage(zmin:zmax)));
    maxDerive=maxDerive+zmin;

    % Max cut
    maskSizePx = floor(maskSize * referenceNumberOfPixels);

    positionLastTestLineAndMargin = positionLastTestLine + floor(maskSizePx / 2) + 1;
    [cutProfilSize, ~] = size(profil);
    if(positionLastTestLineAndMargin >= cutProfilSize)
        positionLastTestLineAndMargin = cutProfilSize - 1;
    end
    
    disp(size(profil));
    disp(positionLastTestLine);
    disp("------");
    [~,maxStrip]=max(profil(positionLastTestLineAndMargin:end));
    maxStrip=positionLastTestLineAndMargin+maxStrip-1;
    
    % Min cut
    negativeDerive=find(derivatedImage<=0);
    try
        minStrip=negativeDerive(length(find(negativeDerive<=maxDerive)));
    catch
        minStrip=zmin;
    end
    
    if((infosImages(i).maxControlLine - halfTLLength) < minStrip)
        minStrip = zmin;
    end

    % Save information
    infosImages(i).rCroppedImageProfil=profil;
    infosImages(i).cutProfilImage=profil(minStrip:maxStrip);
    infosImages(i).minStrip=minStrip;
    infosImages(i).maxStrip=maxStrip;


end
