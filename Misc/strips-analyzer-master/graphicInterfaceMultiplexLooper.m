function graphicInterfaceMultiplexLooper(rawImagesPath,studyName,infosImages,generalInfos,initialParams,numberOfPixels,patternFilePath)

myColor=[160/255 100/255 160/255];
if (ispc)
    fontSize1 = 12;
    fontSize10 = 10;
    fontSize12 = 12;
    fontSize2 = 14;
    fontSize21 = 16;
    fontSize22 = 18;
    fontSize3 = 17;
else
    fontSize1 = 14;
    fontSize10 = 10;
    fontSize12 = 12;
    fontSize2 = 16;
    fontSize21 = 18;
    fontSize22 = 18;
    fontSize3 = 20;
end

calibrationParams.AllFirstSpotX = [];
calibrationParams.AllFirstSpotY = [];
calibrationParams.AllFirstColIndex = [];
calibrationParams.FirstSpotX = 0.5;
calibrationParams.FirstSpotY = 3;
calibrationParams.FirstColIndex = 1;
calibrationParams.referenceNumberOfPixels = numberOfPixels;

[calibrationParams.nbGridLines, calibrationParams.spacingX, calibrationParams.spacingY, calibrationParams.theoricSpotDiameter] = getPatternHeaderInformations(patternFilePath);
minXCoordinate = calibrationParams.spacingX / 2;

handlingNextButton = false;

%% Figure, panels and axes.

fig = figure('Units', 'normalized', 'Position', [0.05 0.1 0.9 0.8], ...
    'Color',myColor,'Visible', 'On', ...
    'Name', ['Test Analyse - Images du répertoire : ' rawImagesPath]);

p1 = uipanel(fig, 'TitlePosition','lefttop','Units', 'normalized',...
    'FontSize',20,'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'HighlightColor',myColor,...
    'ShadowColor',myColor,...
    'Visible','On','Position',[0.025 0.05 0.15 0.9]);

p2 = uipanel(fig,'Title','Settings','FontSize',20,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[0.685 0.05 0.29 0.91]);

p3 = uipanel(fig,'Title','Pattern model','FontSize',20,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[0.2 0.05 0.45 0.91]);

displayedImageAxis = axes(p1);
displayedImageAxis.Position=[0 0 0 0];

[currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
patternModelImage = imread(strcat(currentfunctionpath, '\Multiplex pattern.PNG'));
axesP3 = axes('Parent',p3);
image(axesP3, patternModelImage);
% PANEL 1

fig_text = uicontrol(p1,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.9 1 0.1]);

% PANEL 2
% Text

uicontrol(p2,'Style', 'text','Units', 'normalized', ...
    'String','Settings',...
    'FontSize',fontSize3,...
    'ForegroundColor','white',...
    'FontWeight','bold',...
    'BackgroundColor',myColor,...
    'Position',[0.005 0.79 0.4 0.1]);

uicontrol(p2,'Style', 'Text','Units', 'normalized', ...
    'String','X coordinate of first CAS point',...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.72 1 0.1]);

uicontrol(p2,'Style', 'Text','Units', 'normalized',...
    'String','Y coordinate of first CAS point',...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.62 1 0.1]);

uicontrol(p2,'Style', 'Text','Units', 'normalized',...
    'String','First column index in pattern',...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.52 1 0.1]);

FirstSpotX_value = uicontrol(p2,'Style', 'Text', 'Units', 'normalized', ...
    'String',num2str(calibrationParams.FirstSpotX),...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0.68 0.744 0.2 0.031]);

FirstSpotY_value = uicontrol(p2,'Style', 'Text', 'Units', 'normalized', ...
    'String',num2str(calibrationParams.FirstSpotY),...
    'FontSize',fontSize2,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0.68 0.644 0.2 0.031]);

FirstColIndex_value = uicontrol(p2,'Style', 'Text', 'Units', 'normalized', ...
    'String',num2str(calibrationParams.FirstColIndex),...
    'FontSize',fontSize2,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0.68 0.544 0.2 0.031]);

information_text = uicontrol(p2,'Style', 'Text', 'Units', 'normalized',...
    'FontSize',fontSize2,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.012 1 0.04]);

% Buttons

uicontrol(p2,'style', 'Pushbutton','Units', 'normalized', ...
    'Position',[0.05 0.91 0.27 0.065], ...
    'FontSize',fontSize22,...
    'String', 'Calibration', ...
    'Callback', @calibration_callback);


next_button = uicontrol(p2,'style', 'Pushbutton','Units', 'normalized', ...
    'Position',[0.45 0.91 0.27 0.065], ...
    'FontSize',fontSize22,...
    'String', 'Next image', ...
    'Visible', 'off', ...
    'Callback', @next_callback);


% Sliders

FirstSpotX_slider = uicontrol(p2,'Style', 'Slider',...
    'Units', 'normalized', ...
    'Position',[0.16 0.74 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.FirstSpotX,...
    'Min',minXCoordinate,'Max',5.0,...
    'SliderStep',[0.05/(5.0 - minXCoordinate), 0.5/(5.0 - minXCoordinate)],...
    'Callback', @FirstSpotX_callback);

FirstSpotY_slider = uicontrol(p2,'Style', 'Slider','Units', 'normalized', ...
    'Position',[0.16 0.64 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.FirstSpotY,...
    'Min',0.1,'Max',20.0,...
    'SliderStep',[0.05/19.9, 0.5/19.9],...
    'Callback', @FirstSpotY_callback);

FirstColIndex_slider = uicontrol(p2,'Style', 'Slider','Units', 'normalized', ...
    'Position',[0.16 0.54 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.FirstColIndex,...
    'Min',1,'Max',6,...
    'SliderStep',[1/5, 1/5],...
    'Callback', @FirstColIndex_callback);

%% Callback functions

    function setCurrentImageToDisplay(image)
        % Display the name of the image
        set(fig_text,'String',image.name);
          
        displayedImage = calibrationParams.CurrentImage.rCroppedImage;
        displayedImageAxis.Position = [0.3 0 0.63 0.9];
        
        imagesc(displayedImageAxis,displayedImage);
        colormap gray
        
        % Draw Rectangles on Displayed Image
        
        drawGridOnDisplayedImage(calibrationParams.CurrentImage, calibrationParams, displayedImageAxis, 'clearPreviousGrid');
                
    end


    function calibration_callback(~,~,~)
        calibrationParams.CalibrationImageIndex = 1;
        calibrationParams.CurrentImage = infosImages(calibrationParams.CalibrationImageIndex);
        
        setCurrentImageToDisplay(calibrationParams.CurrentImage);
        
        next_button.Visible = 'on';
    end

    function disp_loop_description(startTime, endTime, paramList, paramName, paramStartValue, paramIncrement, nbLoop)
        disp(strcat("Loop Started : ", startTime));
        if (startTime ~= endTime)
            disp(strcat("Loop Ended : ", endTime));    
        end        
        disp("");
        
        disp([paramList;
            "Param Used" paramName; 
            "Param Start Value" num2str(paramStartValue);
            "Param Increment" num2str(paramIncrement)
            "Nb Loops" num2str(nbLoop)]);
    end


    function [resultFilePath, resultingValues] = runAnalysis(loopNumber, totalLoops, movingValue, currentValue, tmpFileGridsLocationsPath)
            
            % Create the Excel file
            generalInfos.fileNameSuffix = strcat(generalInfos.analysisDateTime,'_',movingValue,string(currentValue));
            infoCurrentLoop = strcat(string(loopNumber), " / ", string(totalLoops));

            wbf = waitbar(0, 'creating excelFile', 'Name', strcat(studyName, " - Loop ", infoCurrentLoop));

            excelFilename=strcat('Results_',generalInfos.name,'_',generalInfos.fileNameSuffix,'.xlsx');
            filenameExcel=strcat(tempdir,excelFilename);
                        
            % message for the user
            waitbar(0.09, wbf, 'Processing Images');
            %set(information_text,'String','Analyse en cours');
            
            
            for j=1:generalInfos.N
                timerVal = tic;
                
                % Message to the user
                message=strcat(int2str(j),'/',int2str(generalInfos.N));
                disp(['----> ' message ' : ' infosImages(j).name]);

                % Display name of the figure
                set(information_text,'String',strcat("Loop ", infoCurrentLoop , " - Analyse en cours : ",message));

                elapsed = toc(timerVal);
                disp(['====> Image processed in ' num2str(elapsed) ' s']);    
            end
            close(wbf);
            wbf = waitbar(0, 'Pica Analyze and Writing in Excel in progress', 'Name', strcat("Loop ", infoCurrentLoop));
            
            croppedFolderPath=strcat(rawImagesPath, 'Cropped');
            appVersion = generalInfos.currentAppVersion.version;
            computeMultiplexAnalysis(generalInfos.lightningCondition, generalInfos.channel, croppedFolderPath, calibrationParams.referenceNumberOfPixels, filenameExcel, initialParams, appVersion, patternFilePath, tmpFileGridsLocationsPath, currentValue);
            
            disp(['Processing results written in excel in : ' num2str(elapsed) ' s']);
                         
            waitbar(0.95, wbf, 'Reading 3sigma values');
            resultingValues = readcell(filenameExcel, 'Sheet','Summary','Range','A7:E79','UseExcel',ispc);
        
            
            % message for the user 
            close(wbf);
            set(information_text,'String',strcat("Analysis complete for loop ", string(loopNumber)))
            
            % trick to set summary sheet as current sheet... and set up the
            % first colmun length (for display) - dont' chaange then number
            % of spaces
            %writematrix("                                                 ",filenameExcel,'Sheet',1,'Range','A1','UseExcel',true);
             % trick to set good colmun length for Trust Index (for display) 
            %writematrix("                        ",filenameExcel,'Sheet',1,'Range','D5','UseExcel',true);
            
            [~,fileName,fileExt] = fileparts(filenameExcel);
            destFilePathName = fullfile(string(rawImagesPath), "Results", "Loops", strcat(fileName, fileExt));
            
            movefile(filenameExcel, destFilePathName);
            
            disp(strcat("------------------- Analysis complete for loop ", string(loopNumber), "-------------------"));
            resultFilePath = destFilePathName;
    end




    function next_callback(~,~,~)
        
        if (~handlingNextButton)
            handlingNextButton = true;
        else
            return;
        end
        
        set(next_button,'Visible','off');
        thibtic=tic;
        
        if generalInfos.compteur<length(infosImages)
            generalInfos.compteur=generalInfos.compteur+1;
            
            calibrationParams.AllFirstSpotX(end+1) = calibrationParams.FirstSpotX;
            calibrationParams.AllFirstSpotY(end+1) = calibrationParams.FirstSpotY;
            calibrationParams.AllFirstColIndex(end+1) = calibrationParams.FirstColIndex;
            
            calibrationParams.CalibrationImageIndex = calibrationParams.CalibrationImageIndex+1;
            calibrationParams.CurrentImage = infosImages(generalInfos.compteur);
            
            setCurrentImageToDisplay(calibrationParams.CurrentImage);
            set(next_button,'Visible','on');
        else
            
            calibrationParams.AllFirstSpotX(end+1) = calibrationParams.FirstSpotX;
            calibrationParams.AllFirstSpotY(end+1) = calibrationParams.FirstSpotY;
            calibrationParams.AllFirstColIndex(end+1) = calibrationParams.FirstColIndex;
            
            tmpFileGridsLocationsPath = strcat(tempdir, 'Grids Locations.txt');
            saveGridsLocations(calibrationParams.AllFirstSpotX, calibrationParams.AllFirstSpotY, calibrationParams.AllFirstColIndex, tmpFileGridsLocationsPath)
            
             % message for the user
            set(information_text,'String','Calibration finie');            
            set(next_button,'Visible','off');
            generalInfos.analysisDateTime = string(datetime('now', 'Format', 'yyyyMMdd-HHmmss'));            
            
            %Actually, the looper increment only circles diameter with a
            %fix value

            noiseROIWidth = calibrationParams.spacingX / 1.5;
            minSpotDiameter = 0.05;
            nbLoop = noiseROIWidth / minSpotDiameter;
            
            paramName = "Spot diameter";
            paramStartValue = minSpotDiameter;
            paramIncrement = 0.05;
            nbLoop = noiseROIWidth / minSpotDiameter;
            
            calibrationParams.CurrentSpotDiameter = calibrationParams.theoricSpotDiameter;
            paramList = [ "Spot diameter" calibrationParams.CurrentSpotDiameter ];
            
            disp_loop_description(generalInfos.analysisDateTime, generalInfos.analysisDateTime, paramList, paramName, paramStartValue, paramIncrement, nbLoop)
            
            currentValue = paramStartValue;
            allResultsFilePath = [];
            allResultingValues = {nbLoop};
            for loop=1:nbLoop
                switch paramName
                    case paramList(1,1)
                        calibrationParams.CurrentSpotDiameter = currentValue;
                    end                
                [resultFilePath, resultingValues] = runAnalysis(loop, nbLoop, paramName, currentValue, tmpFileGridsLocationsPath);                
                allResultsFilePath = [allResultsFilePath resultFilePath];
                allResultingValues{loop} = resultingValues;
                
                currentValue = currentValue + paramIncrement;
            end
            delete(tmpFileGridsLocationsPath);
            
            disp('==================================================================');
            disp('====> Creating Loop Summary');
            set(information_text,'String',strcat(string(nbLoop), " Loops completed"))

            nbSpotType = 6;
            createMultiplexLoopSummary(allResultsFilePath, allResultingValues, paramList, paramName, nbLoop, paramStartValue, paramIncrement, rawImagesPath, generalInfos.name, generalInfos.analysisDateTime, nbSpotType);
            
            disp_loop_description(generalInfos.analysisDateTime, string(datetime('now', 'Format', 'yyyyMMdd-HHmmss')), paramList, paramName, paramStartValue, paramIncrement, nbLoop)
            disp('===================== Loop Analysis complete =====================');
            
        end
        handlingNextButton = false;
    end

    function FirstSpotX_callback(~,~,~)
        
        ValueSliderFirstSpotX=get(FirstSpotX_slider, 'Value');
        set(FirstSpotX_value,'String',num2str(ValueSliderFirstSpotX));
        calibrationParams.FirstSpotX = ValueSliderFirstSpotX;
        
        % draw rectangle
        drawGridOnDisplayedImage(calibrationParams.CurrentImage, calibrationParams, displayedImageAxis, '');

    end

    function FirstSpotY_callback(~,~,~)
        
        ValueSliderFirstSpotY=get(FirstSpotY_slider, 'Value');
        set(FirstSpotY_value,'String',num2str(ValueSliderFirstSpotY));
        calibrationParams.FirstSpotY = ValueSliderFirstSpotY;
        
        % draw rectangle
        drawGridOnDisplayedImage(calibrationParams.CurrentImage, calibrationParams, displayedImageAxis, '');
    end

    function FirstColIndex_callback(~,~,~)
        
        ValueSliderFirstColIndex=get(FirstColIndex_slider, 'Value');
        set(FirstColIndex_value,'String',num2str(ValueSliderFirstColIndex));
        calibrationParams.FirstColIndex = ValueSliderFirstColIndex;
        
        drawGridOnDisplayedImage(calibrationParams.CurrentImage, calibrationParams, displayedImageAxis, '');
    end

end