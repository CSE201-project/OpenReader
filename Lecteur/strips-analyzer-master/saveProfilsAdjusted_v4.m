function saveProfilsAdjusted_v4(newpath, studyName, infosImages, figureName, fileNameSuffix, filenameExcel, waitbarFigure, wbPercentStart, wbPercentRange)

    getAbscisses = @(infosImage)(1-infosImage.maxControlLine+infosImage.minStrip):(length(infosImage.cutProfilImage)-infosImage.maxControlLine+infosImage.minStrip);

    mergeProfilesIntoGraph(infosImages, studyName, ...
        newpath, figureName, fileNameSuffix, ...
        @(infoImage)(infoImage.cutProfilImage), getAbscisses, ...
        waitbarFigure, wbPercentStart, wbPercentRange);

    xstart=[];
    xend=[];
   
    N = length(infosImages);
    
    for i=1:N
        
        infosImages(i).x = getAbscisses(infosImages(i));
        
        % Save values to then write adjusted profiles in the Excel file. 
        xstart=[xstart,infosImages(i).x(1)];
        xend=[xend,infosImages(i).x(end)];        
    end
    
    waitbar(wbPercentStart + wbPercentRange, waitbarFigure, 'Adjusted Profiles : Writing to Excel');
        
    % save profil adjusted in an Excel file.
    matNames = [];
    matProfiles = [];
    for i=1:N
        
        profil=infosImages(i).cutProfilImage;
        Name=infosImages(i).name;
        
        x=infosImages(i).x;
                
        profiladjusted=[zeros(1,abs(min(xstart)-min(x))),profil',zeros(1,abs(max(xend)-max(x)))];
      
        matNames = [matNames; string(Name)];
        matProfiles = [matProfiles; profiladjusted];
        
    end

end