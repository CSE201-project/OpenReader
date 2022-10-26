function drawProfileAndROI(imageProfil, axisCanvas, ROILength, controlROIPos, testROIPos)

    y=1:255;

    x1=controlROIPos-ROILength/2;
    x2=x1+ROILength;
    x3=testROIPos-ROILength/2;
    x4=x3+ROILength;
    cla(axisCanvas)
    
    plot(axisCanvas,1:length(imageProfil),imageProfil)
    hold(axisCanvas,'on')
    plot(axisCanvas,x1,y,'g.')
    hold(axisCanvas,'on')
    plot(axisCanvas,x2,y,'g.')
    hold(axisCanvas,'on')
    plot(axisCanvas,x3,y,'r.')
    hold(axisCanvas,'on')
    plot(axisCanvas,x4,y,'r.')

end