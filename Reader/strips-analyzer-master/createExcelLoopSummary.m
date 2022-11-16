function createExcelLoopSummary(allResultsFilePath, allResultingValues, paramList, paramName, maxLoop, paramStartValue, paramValueIncrement, rawImagesPath, studyName, analysisDateTime, linesNumber)
 
    % Determine the Excel model file to get 
    templateExcelFile='AnalysisLoopSummary.xlsx';
    
    [currentfunctionpath,~,~]=fileparts(matlab.desktop.editor.getActiveFilename);
    templateFilePath=strcat(currentfunctionpath, '/', templateExcelFile);

    % Name the new Excel file
    excelFilename=strcat('AnalysisLoopSummary_',studyName,'_',paramName,'_',analysisDateTime,'.xlsx');
    
    % Copy, rename and move the Excel file in the Results folder
    filenameExcel=strcat(tempdir,excelFilename);
    copyfile(templateFilePath,filenameExcel);
    
    matInfo = [analysisDateTime; string(rawImagesPath); studyName; paramName];
    safe_writeMatrix(matInfo,filenameExcel,'Summary','C3');
    
    matInfo = [];
    for i=1:3
        if (paramList(i, 1) ~= paramName)
           matInfo = [matInfo; paramList(i, 1) paramList(i, 2)];
        end
    end    
    safe_writeMatrix(matInfo,filenameExcel,'Summary','I5');
    
    mat = {};    
    allLods = {};
    nbAlgos = 16;
    for i=1:maxLoop
        resultingValues = allResultingValues{i};
        min3sigmaAt100Trusted = NaN;
        min3sigmaByTestLine = 10000000;
        bestLineAt3s = [""];
        bestAlgos = [""];
        allTestline3s = [];
        allTestline3sWithStdDv = [];
        allTestlineAllAlgos = [];
        for j=1:4
            if (j < linesNumber)
                lineTestOffset = (20 * (j-1));
                testline3s = resultingValues{14 + lineTestOffset, 5};
                if ismissing(testline3s) || testline3s == 0
                    allTestline3s(j) = NaN;
                    allTestline3sWithStdDv(j*2-1) = NaN;
                    allTestline3sWithStdDv(j*2) = NaN;
                    
                    allTestlineAllAlgos = [allTestlineAllAlgos NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN NaN] ;
                else
                    allTestline3s(j) = round(testline3s, 4);
                    allTestline3sWithStdDv(j*2-1) = round(testline3s, 4);
                    if (min3sigmaByTestLine > testline3s)
                        min3sigmaByTestLine = testline3s;
                    end

                    usefulCell = resultingValues(1 + lineTestOffset:nbAlgos + lineTestOffset, 1:4);
                    
                    S=size(usefulCell);
                    h=S(1);
                    for k=1:h
                        TwoSigmaCellContent = usefulCell{k, 2};
                        if (ischar(TwoSigmaCellContent))
                            nb = str2double(TwoSigmaCellContent(2:length(TwoSigmaCellContent)));
                            usefulCell{k, 2} = nb;
                        end
                        ThreeSigmaCellContent = usefulCell{k, 3};
                        if (ischar(ThreeSigmaCellContent))
                            nb = str2double(ThreeSigmaCellContent(2:length(ThreeSigmaCellContent)));
                            usefulCell{k, 3} = nb;
                        end
                    end    
                    
                    idx = cellfun(@(x)(x >= 0.9), usefulCell(:,4));
                    allTestline3sWithStdDv(j*2) = round(std(cell2mat(usefulCell(idx, 3))), 2);
                    
                    usefulMat = cell2mat(usefulCell(idx, 3:4));
                    tmp = min(usefulMat,[],1);
                    if (size(tmp, 1) > 0 && (isnan(min3sigmaAt100Trusted) || min3sigmaAt100Trusted > tmp(1)))
                       min3sigmaAt100Trusted = tmp(1);
                       curLine = strcat("T", num2str(j));
                       bestLineAt3s = [ curLine ];
                       idx2 = cellfun(@(x) isequal(x,min3sigmaAt100Trusted), usefulCell(:,3));
                       bestAlgos = [ strcat(curLine, ":", join(string(find(idx2)), strcat(",", curLine, ":"))) ];
                    elseif (size(tmp, 1) > 0 && min3sigmaAt100Trusted == tmp(1))
                       curLine = strcat("T", num2str(j));
                       bestLineAt3s = [ bestLineAt3s curLine ];
                       
                       idx2 = cellfun(@(x) isequal(x,min3sigmaAt100Trusted), usefulCell(:,3));
                       bestAlgos = [ bestAlgos ; strcat(curLine, ":", join(string(find(idx2)), strcat(",", curLine, ":"))) ];
                    end
                                        
                    for k=1:nbAlgos                        
                        if (usefulCell{k,4} >= 0.9)
                            lod = usefulCell{k,3};
                        else
                            lod = NaN;
                        end
                        allTestlineAllAlgos = [allTestlineAllAlgos lod] ;
                    end
                end
            else
                allTestline3s(j) = NaN;
                allTestline3sWithStdDv(j*2-1) = NaN;
                allTestline3sWithStdDv(j*2) = NaN;
            end            
        end
        mean3s = round(mean(allTestline3s, 'omitnan'), 4);
        b = join(bestLineAt3s, ",");
        b2 = string(join(unique(bestAlgos), ","));
        allTestline3sWithStdDv(7) = resultingValues{28 , 5};
        mat(i,:) = {[(paramStartValue + ((i-1) * paramValueIncrement)) mean3s allTestline3sWithStdDv min3sigmaAt100Trusted], b, b2, toExcelLink(allResultsFilePath(i))};
        allLods(i,:) = {allTestlineAllAlgos};
    end
    
    writecell(mat,filenameExcel,'Sheet','Summary','Range','B32','UseExcel',true);
    writecell(allLods,filenameExcel,'Sheet','Summary','Range','R32','UseExcel',true);
    
    [~,fileName,fileExt] = fileparts(filenameExcel);
    destFilePathName = fullfile(string(rawImagesPath), "Results", strcat(fileName, fileExt));

    movefile(filenameExcel, destFilePathName);
    if (ispc)
        winopen(destFilePathName);
    end
end

function [link] = toExcelLink(filename)
    %C:\TEMP\V2\9547-3F9-118A - Lite\Results\Loops\Results_9547-3F9-118A_20210128-151555_Rectangle Width10.xlsx
    [~,name,ext] = fileparts(filename);
    link = strcat('Loops\', name, ext);    
end
