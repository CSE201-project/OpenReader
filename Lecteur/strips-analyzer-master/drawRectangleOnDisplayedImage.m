function  drawRectangleOnDisplayedImage(croppedImage, ...
    maxControlLine, ...
    calibrationParams, ...
    displayedImageAxis, ...
    testLinesNumber, ...
    clearRectangle)
% drawRectangleOnDisplayedImage :
%   
%   Version of the function displayRectangle adapted to the third version
%   of stripsAnalysis. This function displays rectangle around control and
%   test lines + noise. No limitation of test line.
%
%
% INPUT PARAMETERS : 
%   
%   croppedImage
%   
%   maxControlLine : indice to find the max of the control line.
%   
%   calibrationParams : vector with the C2C value for each test line.
%   
%   calibrationParams.AnalysisRectangle.Width : heigh of the rectangle.
%
%   displayedImageAxis
%
%   testLinesNumber : width of the rectangle.

% OUTPUT PARAMETERS :
%
%   POSITIONVECTOR : vector with the position (x,y) of (in order) the
%   control line, noise, test line (the closest to the control line to the 
%   furthest).
%
%
% WRITTEN BY CVE
% V4 created by TMY on 17/10/2020  

    persistent localAllRectangles;
    if isempty(localAllRectangles)
        localAllRectangles.imageName = croppedImage.name;
    end
        
    if (nargin < 6)
        clearRectangle = '';
    end
    
    if ~isempty(clearRectangle) || ~strcmp(localAllRectangles.imageName, croppedImage.name)
        fields = fieldnames(localAllRectangles);
        localAllRectangles = rmfield(localAllRectangles, fields);
        localAllRectangles.imageName = croppedImage.name;
    end

    widthPx = size(croppedImage.rCroppedImage,2);
    paddingPx = floor(calibrationParams.referenceNumberOfPixels * calibrationParams.AnalysisRectangle.Padding);

    nbPxToDelete = widthPx - paddingPx;
    if nbPxToDelete < 1
        nbPxToDelete = 1;
        paddingPx = widthPx - 1;
    end
    
    displayedImageSize=size(croppedImage.rCroppedImage);
    long=displayedImageSize(2)-paddingPx;

    % Display Control Line Rectangle

    xc=floor(nbPxToDelete/2);
    yc=maxControlLine;
    
    [localAllRectangles,controlLineRectangle] = getLocalRectangle(localAllRectangles, 'controlLineRectangle', displayedImageAxis, 'g', '-');
    controlLineRectangle.Position = [xc,yc,paddingPx,2];
    
    % Display Test Line

    distanceCLtoTLPx = floor(calibrationParams.DistanceCLtoTLMm * calibrationParams.referenceNumberOfPixels);
    lengthTLPx = floor(calibrationParams.LengthTLMm * calibrationParams.referenceNumberOfPixels);
    
    xt=xc;
    yt=yc+distanceCLtoTLPx-(floor(lengthTLPx/2));
        
    smallRectangleName = 'smallRect';
    [localAllRectangles,smallRectangle] = getLocalRectangle(localAllRectangles, smallRectangleName, displayedImageAxis, 'r', '-');
    smallRectangle.Position = [xt,yt,paddingPx,lengthTLPx];
       
end

function [allRectangles,localRectangle] = getLocalRectangle(allRectangles, rectangleId, displayedImageAxis, edgeColor, lineStyle)
    
    if ~isfield(allRectangles, rectangleId)
        %allRectangles.(rectangleId) = rectangle(displayedImageAxis,'Position',position,'EdgeColor',edgeColor,'LineStyle',lineStyle);
        allRectangles.(rectangleId) = rectangle(displayedImageAxis,'EdgeColor',edgeColor,'LineStyle',lineStyle);
    end
    
    localRectangle = allRectangles.(rectangleId);
end
