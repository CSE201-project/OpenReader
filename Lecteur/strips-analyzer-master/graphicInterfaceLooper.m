function graphicInterfaceLooper(rawImagesPath,essayNumber,infosImages,generalInfos,initialParams,concentrationList,maxConcentrationList,linesNumber,numberOfPixels)

maxLengthROI = 3;
myColor=[235/255 95/255 14/255];
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

calibrationParams.TestLinesNumber = linesNumber;
calibrationParams.RegionOfInterestLength = 2.3; % mask
calibrationParams.AnalysisRectangle.Padding = 0.5; %long=s(2)-floor(get(linelength_slider, 'Value'));
calibrationParams.AnalysisRectangle.Width = 0.5;
calibrationParams.DistanceCLtoTLMm = 6.0;
calibrationParams.LengthTLMm = 1.0;
calibrationParams.referenceNumberOfPixels = numberOfPixels;



handlingNextButton = false;

%% Figure, panels and axes.

fig = figure('Units', 'normalized', 'Position', [0.05 0.1 0.9 0.8], ...
    'Color',myColor,'Visible', 'On', 'NumberTitle', 'off', ...
    'Name', strcat("Looper (", essayNumber, ") Répertoire : ", rawImagesPath));

p1 = uipanel(fig, 'TitlePosition','lefttop','Units', 'normalized',...
    'FontSize',20,'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'HighlightColor',myColor,...
    'ShadowColor',myColor,...
    'Visible','On','Position',[0.025 0.05 0.15 0.9]);

displayedImageAxis = axes(p1);
displayedImageAxis.Position=[0 0 0 0];


p2 = uipanel(fig,'Title','Settings','FontSize',20,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[0.685 0.05 0.29 0.91]);


p3 = uipanel(fig,'Title','Gauss : CL','FontSize',fontSize12,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.2 0.7 0.1 0.2]);

gaussCLAxis = axes(p3);
gaussCLAxis.Position=[0.09 0.05 0.9 0.9];


p4 = uipanel(fig,'Title','Gauss : TL','FontSize',fontSize12,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.32 0.7 0.1 0.2]);

gaussTLAxis = axes(p4);
gaussTLAxis.Position=[0.09 0.05 0.9 0.9];

gaussSpeed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.32 0.65 0.1 0.05]);


p5 = uipanel(fig,'Title','GaussTrend : CL','FontSize',fontSize12,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.44 0.7 0.1 0.2]);

gaussTrendCLAxis = axes(p5);
gaussTrendCLAxis.Position=[0.09 0.05 0.9 0.9];


p6 = uipanel(fig,'Title','GaussTrend : TL','FontSize',fontSize12,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.56 0.7 0.1 0.2]);

gaussTrendTLAxis = axes(p6);
gaussTrendTLAxis.Position=[0.09 0.05 0.9 0.9];

gauss2Speed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.56 0.65 0.1 0.05]);

p7 = uipanel(fig,'Title','Detrend Poly 1','FontSize',fontSize10,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.2 0.4 0.22 0.27]);

detrendPoly1Axis = axes(p7);
detrendPoly1Axis.Position=[0.08 0.1 0.91 0.9];

poly1Speed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.2 0.35 0.1 0.05]);

p8 = uipanel(fig,'Title','Affine Fit','FontSize',fontSize10,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.44 0.4 0.22 0.27]);

affineFitAxis = axes(p8);
affineFitAxis.Position=[0.08 0.1 0.91 0.9];

affineSpeed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.44 0.35 0.1 0.05]);

p9 = uipanel(fig,'Title','Exp fit','FontSize',fontSize10,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.2 0.1 0.22 0.27]);

exponentialFitAxis = axes(p9);
exponentialFitAxis.Position=[0.08 0.1 0.91 0.9];

expFitSpeed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.2 0.05 0.1 0.05]);

p10 = uipanel(fig,'Title','Poly 2 Fit','FontSize',fontSize10,'Units', 'normalized',...
    'ForegroundColor','white','BackgroundColor',myColor,...
    'Position',[.44 0.1 0.22 0.27]);

poly2FitAxis = axes(p10);
poly2FitAxis.Position=[0.08 0.1 0.91 0.9];

poly2Speed_text = uicontrol(fig,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[.44 0.05 0.1 0.05]);

% PANEL 1

fig_text = uicontrol(p1,'Style', 'Text','Units', 'normalized',...
    'FontSize',fontSize1,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.9 1 0.1]);

% Text

uicontrol(p2,'Style', 'Text','Units', 'normalized', ...
    'String','Theoric distance between CL and TL in millimeters ',...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.72 1 0.1]);

uicontrol(p2,'Style', 'Text','Units', 'normalized',...
    'String','Theoric width of the TL in millimeters',...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0 0.62 1 0.1]);

DistanceCLtoTLMm_value = uicontrol(p2,'Style', 'Text', 'Units', 'normalized', ...
    'String',num2str(calibrationParams.DistanceCLtoTLMm),...
    'FontSize',fontSize10,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0.68 0.744 0.2 0.031]);

LengthTLMm_value = uicontrol(p2,'Style', 'Text', 'Units', 'normalized', ...
    'String',num2str(calibrationParams.LengthTLMm),...
    'FontSize',fontSize2,...
    'ForegroundColor','white',...
    'BackgroundColor',myColor,...
    'Position',[0.68 0.644 0.2 0.031]);

uicontrol(p2,'Style', 'Text','Units', 'normalized', 'Visible', 'off','String','Size of the mask',...
    'FontSize',fontSize2,'ForegroundColor','white',...
    'BackgroundColor',myColor,'Position',[0 0.25 1 0.031]);

mask_text = uicontrol(p2,'Style', 'Text','Units', 'normalized', 'Visible', 'off','String',num2str(calibrationParams.RegionOfInterestLength),...
    'FontSize',fontSize2,'ForegroundColor','white',...
    'BackgroundColor',myColor,'Position',[0.68 0.203 0.2 0.031]);


uicontrol(p2,'Style', 'Text','Units', 'normalized','String','Size of the analysis rectangles',...
    'FontSize',fontSize2,'ForegroundColor','white',...
    'BackgroundColor',myColor,'Position',[0 0.14 1 0.04]);

linelength_text_value = ['Length = ' num2str(calibrationParams.AnalysisRectangle.Padding)];
linelength_text = uicontrol(p2,'Style', 'Text','Units', 'normalized','String',linelength_text_value,...
    'FontSize',fontSize1,'ForegroundColor','white',...
    'BackgroundColor',myColor,'Position',[0.17 0.06 0.25 0.031]);

linewidth_text_value = ['width = ' num2str(calibrationParams.AnalysisRectangle.Width)];
linewidth_text = uicontrol(p2,'Style', 'Text','Units', 'normalized', 'Visible', 'off','String',linewidth_text_value,...
    'FontSize',fontSize1,'ForegroundColor','white',...
    'BackgroundColor',myColor,'Position',[0.57 0.06 0.25 0.031]);


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

DistanceCLtoTLMm_slider = uicontrol(p2,'Style', 'Slider',...
    'Units', 'normalized', ...
    'Position',[0.16 0.74 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.DistanceCLtoTLMm,...
    'Min',0.1,'Max',20.0,...
    'SliderStep',[0.05/19.9, 0.5/19.9],...
    'Callback', @DistanceCLtoTLMm_callback);

LengthTLMm_slider = uicontrol(p2,'Style', 'Slider','Units', 'normalized', ...
    'Position',[0.16 0.64 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.LengthTLMm,...
    'Min',0.1,'Max',5.0,...
    'SliderStep',[0.05/4.9, 0.5/4.9],...
    'Callback', @LengthTLMm_callback);

mask_slider = uicontrol (p2,'Style', 'Slider','Units', 'normalized', 'Visible', 'off',...
    'Position',[0.15 0.2 0.5 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.RegionOfInterestLength,...
    'Min',0.1,'Max',5.0,...
    'SliderStep',[0.05/4.9, 0.5/4.9],...
    'Callback', @mask_callback);

linelength_slider = uicontrol (p2,'Style', 'Slider','Units', 'normalized',...
    'Position',[0.15 0.09 0.3 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.AnalysisRectangle.Padding,...
    'Min',0.1,'Max',maxLengthROI,...
    'SliderStep',[0.05/(maxLengthROI-0.1), 0.5/(maxLengthROI-0.1)],...
    'Callback', @linelength_callback);

linewidth_slider = uicontrol (p2,'Style', 'Slider','Units', 'normalized', 'Visible', 'off',...
    'Position',[0.55 0.09 0.3 0.03], ...
    'FontSize',fontSize21,...
    'Value',calibrationParams.AnalysisRectangle.Width,...
    'Min',0.1,'Max',5.0,...
    'SliderStep',[0.05/4.9, 0.5/4.9],...
    'Callback', @linewidth_callback);


%% Callback functions

axlist=[gaussCLAxis,gaussTLAxis,gaussTrendCLAxis,gaussTrendTLAxis,detrendPoly1Axis,affineFitAxis,exponentialFitAxis,poly2FitAxis];
profilesAxis=[detrendPoly1Axis,affineFitAxis,exponentialFitAxis,poly2FitAxis];

    function setCurrentImageToDisplay(image)
        % Display the name of the image
        set(fig_text,'String',image.name);
        
        profil = computeProfilWithPadding(calibrationParams.CurrentImage.rCroppedImage, calibrationParams.AnalysisRectangle.Padding, generalInfos.lightningCondition, calibrationParams.referenceNumberOfPixels);
        calibrationParams.CurrentImage.CroppedImageProfil = profil;
        
        % Find control line
        calibrationParams.MaxControlLine = findControlLine_calibParams(generalInfos.lightningCondition, generalInfos.channel, calibrationParams.CurrentImage.rgbCroppedPath,calibrationParams.AnalysisRectangle.Padding,calibrationParams.referenceNumberOfPixels);
        
        % Display the cropped image
        displayedImage = calibrationParams.CurrentImage.rCroppedImage;
        displayedImageAxis.Position = [0.3 0 0.33 0.9];
        
        imagesc(displayedImageAxis,displayedImage);
        colormap gray
        
        
        
        % Draw Rectangles on Displayed Image
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, ...
            calibrationParams, ...
            displayedImageAxis, ...
            calibrationParams.TestLinesNumber, ...
            'clearPreviousRectangles');
                
    end


    function [resultFilePath, resultingValues] = runAnalysis(loopNumber, totalLoops, movingValue, currentValue)
            
            % Create the Excel file
            generalInfos.fileNameSuffix = strcat(generalInfos.analysisDateTime,'_',movingValue,string(currentValue));
            infoCurrentLoop = strcat(string(loopNumber), " / ", string(totalLoops));

            wbf = waitbar(0, 'creating excelFile', 'Name', strcat(essayNumber, " - Loop ", infoCurrentLoop));

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
                set(fig_text,'String',infosImages(j).name);
                
                waitbar(j/generalInfos.N * 0.7, wbf, ['ROI preview Remaining : ' num2str(generalInfos.N - j + 1)]);
                
                elapsedTimesUI = [gaussSpeed_text, gauss2Speed_text, poly1Speed_text, affineSpeed_text, expFitSpeed_text, poly2Speed_text];
                [infosImages]=analysis_v4(j,infosImages,generalInfos,rawImagesPath,filenameExcel,calibrationParams,displayedImageAxis,axlist,elapsedTimesUI);
                
                elapsed = toc(timerVal);
                disp(['====> Image processed in ' num2str(elapsed) ' s']);    
            end
            close(wbf);
            wbf = waitbar(0, 'Pica Analyze and Writing in Excel in progress', 'Name', strcat("Loop ", infoCurrentLoop));
            
            croppedFolderPath=strcat(rawImagesPath, 'Cropped');
            appVersion = generalInfos.currentAppVersion.version;
            excelFileCoeffs = strcat(rawImagesPath, 'Coefficients courbes calibrationV4.xlsx');
            
            sizeTLProxNoise = 1.3;
            sizeNoiseProxNoise = 1;
            if(contains(initialParams(4), "_BL"))
                sizeTLProxNoise = 0.7;
            end
            
            computePicaAnalysis(generalInfos.lightningCondition, generalInfos.channel, croppedFolderPath, calibrationParams.AnalysisRectangle.Padding, calibrationParams.AnalysisRectangle.Width, calibrationParams.RegionOfInterestLength, calibrationParams.referenceNumberOfPixels, calibrationParams.DistanceCLtoTLMm, calibrationParams.LengthTLMm, filenameExcel, initialParams, appVersion, excelFileCoeffs, sizeTLProxNoise, sizeNoiseProxNoise);
            
            disp(['Processing results written in excel in : ' num2str(elapsed) ' s']);
            
            % save profils
            waitbar(0.8, wbf, 'Saving Cut Profiles Graph');
            saveProfilsCut_v4(strcat(rawImagesPath, "/Results/Loops"), essayNumber, infosImages, generalInfos.fileNameSuffix, filenameExcel, wbf, 0.8, 0.05);
            
            waitbar(0.85, wbf, 'Saving Adjusted Profiles Graph');
            saveProfilsAdjusted_v4(strcat(rawImagesPath, "/Results/Loops"), essayNumber, infosImages, 'Profils bruts ajustés', generalInfos.fileNameSuffix, filenameExcel, wbf, 0.85, 0.05);
            
            %waitbar(0.9, wbf, 'Saving CutAdjusted Profiles');
            %saveProfilsCutAdjusted_v4(strcat(rawImagesPath, "/Results/Loops"), essayNumber, infosImages, 'Profils fittés ajustés', generalInfos.fileNameSuffix, wbf, 0.9, 0.05);
            
            waitbar(0.95, wbf, 'Reading 3sigma values');
            resultingValues = readcell(filenameExcel, 'Sheet','Summary','Range','A7:E84','UseExcel',ispc);
        
            
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

    function calibration_callback(~,~,~)
        calibrationParams.CalibrationImageIndex = 1;
        calibrationParams.CurrentImage = infosImages(maxConcentrationList(calibrationParams.CalibrationImageIndex));
        
        setCurrentImageToDisplay(calibrationParams.CurrentImage);
        
        next_button.Visible = 'on';
    end

    function disp_loop_description(startTime, endTime, paramList, paramName, paramStartValue, paramIncrement, nbLoop)
        disp(strcat("Loop Started : ", startTime));
        if (startTime ~= endTime)
            disp(strcat("Loop Ended : ", endTime));    
        end        
        disp("");
        
        disp(["Theroric Distance CL --> TL" join(num2str(calibrationParams.DistanceCLtoTLMm), " mm / ");
            paramList;
            "Param Used" paramName; 
            "Param Start Value" num2str(paramStartValue);
            "Param Increment" num2str(paramIncrement)
            "Nb Loops" num2str(nbLoop)]);
    end

    function next_callback(~,~,~)
        
        if (~handlingNextButton)
            handlingNextButton = true;
        else
            return;
        end
        
        set(next_button,'Visible','off');
        
        if generalInfos.compteur<length(maxConcentrationList)
            generalInfos.compteur=generalInfos.compteur+1;
            
            calibrationParams.CalibrationImageIndex = calibrationParams.CalibrationImageIndex+1;
            calibrationParams.CurrentImage = infosImages(maxConcentrationList(calibrationParams.CalibrationImageIndex));
            
            setCurrentImageToDisplay(calibrationParams.CurrentImage);
            set(next_button,'Visible','on');
            
        else
            
            % message for the user
            set(information_text,'String','Calibration finie');            
            set(next_button,'Visible','off');
            generalInfos.analysisDateTime = string(datetime('now', 'Format', 'yyyyMMdd-HHmmss'));            
            
            [paramName, paramStartValue, paramIncrement, nbLoop] = loopParamDialog(calibrationParams.AnalysisRectangle.Width, calibrationParams.AnalysisRectangle.Padding, calibrationParams.RegionOfInterestLength);
            
            paramList = [ "Rectangle LengthROI" calibrationParams.AnalysisRectangle.Padding; "Rectangle Width" calibrationParams.AnalysisRectangle.Width; "Mask Size" calibrationParams.RegionOfInterestLength ];
            
            disp_loop_description(generalInfos.analysisDateTime, generalInfos.analysisDateTime, paramList, paramName, paramStartValue, paramIncrement, nbLoop)
            
            currentValue = paramStartValue;
            allResultsFilePath = [];
            allResultingValues = {nbLoop};
            for loop=1:nbLoop
                switch paramName
                    case paramList(1,1)
                        calibrationParams.AnalysisRectangle.Padding = currentValue;
                    case paramList(2,1)
                        calibrationParams.AnalysisRectangle.Width = currentValue;
                    case paramList(3,1)
                        calibrationParams.RegionOfInterestLength = currentValue;
                end                
                [resultFilePath, resultingValues] = runAnalysis(loop, nbLoop, paramName, currentValue);                
                allResultsFilePath = [allResultsFilePath resultFilePath];
                allResultingValues{loop} = resultingValues;
                
                currentValue = currentValue + paramIncrement;
                if((paramName == "Rectangle LengthROI") & (currentValue > maxLengthROI))
                    nbLoop = loop;
                    break;
                end    
            end
            disp('==================================================================');
            disp('====> Creating Loop Summary');
            set(information_text,'String',strcat(string(nbLoop), " Loops completed"))

            createExcelLoopSummary(allResultsFilePath, allResultingValues, paramList, paramName, nbLoop, paramStartValue, paramIncrement, rawImagesPath, generalInfos.name, generalInfos.analysisDateTime, linesNumber);
            
            disp_loop_description(generalInfos.analysisDateTime, string(datetime('now', 'Format', 'yyyyMMdd-HHmmss')), paramList, paramName, paramStartValue, paramIncrement, nbLoop)
            disp('===================== Loop Analysis complete =====================');
            
        end
                
        handlingNextButton = false;
    end

    function DistanceCLtoTLMm_callback(~,~,~)
        
        ValueSliderDistanceCLtoTLMm=get(DistanceCLtoTLMm_slider, 'Value');
        set(DistanceCLtoTLMm_value,'String',num2str(ValueSliderDistanceCLtoTLMm));
        
        calibrationParams.DistanceCLtoTLMm = ValueSliderDistanceCLtoTLMm;
        
        % draw rectangle
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, calibrationParams, ...
            displayedImageAxis, linesNumber);
                
    end

    function LengthTLMm_callback(~,~,~)
        
        ValueSliderLengthTLMm=get(LengthTLMm_slider, 'Value');
        set(LengthTLMm_value,'String',num2str(ValueSliderLengthTLMm));
        
        calibrationParams.LengthTLMm = ValueSliderLengthTLMm;
        % draw rectangle
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, calibrationParams, ...
            displayedImageAxis, linesNumber);
    end

    function mask_callback (~,~,~)
        
        mask=get(mask_slider, 'Value');
        set(mask_text,'String',num2str(mask));
        
        calibrationParams.RegionOfInterestLength = mask;
              % Draw Rectangles on Displayed Image
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, ...
            calibrationParams, ...
            displayedImageAxis, ...
            calibrationParams.TestLinesNumber);
        
    end

    function linelength_callback (~,~,~)
        
        linelength=get(linelength_slider, 'Value');
        set(linelength_text,'String',['padding = ' num2str(linelength)]);
        
        calibrationParams.AnalysisRectangle.Padding = linelength;
        
        profil = computeProfilWithPadding(calibrationParams.CurrentImage.rCroppedImage, calibrationParams.AnalysisRectangle.Padding, generalInfos.lightningCondition, calibrationParams.referenceNumberOfPixels);
        calibrationParams.CurrentImage.CroppedImageProfil = profil;
        
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, calibrationParams, ...
            displayedImageAxis, linesNumber);
                
    end

    function linewidth_callback (~,~,~)
        
        linewidth=get(linewidth_slider, 'Value');
        set(linewidth_text,'String',['width = ' num2str(linewidth)]);
        
        calibrationParams.AnalysisRectangle.Width = linewidth;
        
        % draw rectangle
        drawRectangleOnDisplayedImage(calibrationParams.CurrentImage, ...
            calibrationParams.MaxControlLine, calibrationParams, ...
            displayedImageAxis, linesNumber);
        
    end
    
end