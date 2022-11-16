function drawGridOnDisplayedImage(croppedImage, calibrationParams, displayedImageAxis, clearGrid)
    persistent localAllRectangles;
    if isempty(localAllRectangles)
        localAllRectangles.imageName = croppedImage.name;
    end
    
    if ~isempty(clearGrid) || ~strcmp(localAllRectangles.imageName, croppedImage.name)
        fields = fieldnames(localAllRectangles);
        localAllRectangles = rmfield(localAllRectangles, fields);
        localAllRectangles.imageName = croppedImage.name;
    end
    
    FirstSpotXPx = floor(calibrationParams.referenceNumberOfPixels * calibrationParams.FirstSpotX);
    FirstSpotYPx = floor(calibrationParams.referenceNumberOfPixels * calibrationParams.FirstSpotY);
    
    displayedImageSize=size(croppedImage.rCroppedImage);
    if(FirstSpotXPx > displayedImageSize(2))
        FirstSpotXPx = displayedImageSize(2) - 1;
    end
    if(FirstSpotYPx > displayedImageSize(1))
        FirstSpotYPx = displayedImageSize(1) - 1;
    end
    
    [localAllRectangles,FirstSpotX] = getLocalRectangle(localAllRectangles, 'FirstSpotX', displayedImageAxis, 'r', '-');
    FirstSpotX.Position = [FirstSpotXPx, 1, 0, displayedImageSize(1)];
    
    [localAllRectangles,FirstSpotY] = getLocalRectangle(localAllRectangles, 'FirstSpotY', displayedImageAxis, 'r', '-');
    FirstSpotY.Position = [1, FirstSpotYPx, displayedImageSize(2), 0];
    
    rectangleShiftXPx = round(calibrationParams.referenceNumberOfPixels * calibrationParams.spacingX);
    rectangleShiftYPx = round(calibrationParams.referenceNumberOfPixels * calibrationParams.spacingY);
    currentColCoordinate = FirstSpotXPx;
    currentLineCoordinate = FirstSpotYPx;
    
    acc = 2;
    while((currentColCoordinate) < displayedImageSize(2) - (rectangleShiftXPx/2))
        currentOriginX = currentColCoordinate - rectangleShiftXPx/2;
        for i=0:8
            currentOriginY = FirstSpotYPx + rectangleShiftYPx * i - (rectangleShiftYPx/2);
            [localAllRectangles,CurrentColumn] = getLocalRectangle(localAllRectangles, strcat('Case',num2str(acc),num2str(i)), displayedImageAxis, 'g', '-');
            CurrentColumn.Position = [currentOriginX, currentOriginY, rectangleShiftXPx, rectangleShiftYPx];
            
            verticalCrossX = currentColCoordinate;
            horizontalCrossX = currentColCoordinate - rectangleShiftXPx/8;
            verticalCrossY = currentOriginY + ((rectangleShiftYPx*3)/8);
            horizontalCrossY = currentOriginY + rectangleShiftYPx/2;
            
            [localAllRectangles,verticalCross] = getLocalRectangle(localAllRectangles, strcat('verticalCross',num2str(acc),num2str(i)), displayedImageAxis, 'r', '-');
            verticalCross.Position = [verticalCrossX, verticalCrossY, 0, rectangleShiftYPx/4];
            [localAllRectangles,horizontalCross] = getLocalRectangle(localAllRectangles, strcat('horizontalCross',num2str(acc),num2str(i)), displayedImageAxis, 'r', '-');
            horizontalCross.Position = [horizontalCrossX, horizontalCrossY, rectangleShiftXPx/4, 0];

        end
        currentColCoordinate = currentColCoordinate + rectangleShiftXPx;
        acc = acc + 1;
    end
end

function [allRectangles,localRectangle] = getLocalRectangle(allRectangles, rectangleId, displayedImageAxis, edgeColor, lineStyle)
    
    if ~isfield(allRectangles, rectangleId)
        allRectangles.(rectangleId) = rectangle(displayedImageAxis,'EdgeColor',edgeColor,'LineStyle',lineStyle);
    end
    
    localRectangle = allRectangles.(rectangleId);
end