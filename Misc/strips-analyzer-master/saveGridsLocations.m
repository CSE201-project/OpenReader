function saveGridsLocations(AllFirstSpotX, AllFirstSpotY, AllFirstColIndex, tmpFileGridsLocationsPath)
    if isfile(tmpFileGridsLocationsPath)
        delete(tmpFileGridsLocationsPath);
    end
    
    file = fopen(tmpFileGridsLocationsPath,'w');
    for i=1:length(AllFirstSpotX)
        lineData = '%2.2f,%2.2f,%d\n';
        fprintf(file, lineData, AllFirstSpotX(i), AllFirstSpotY(i), AllFirstColIndex(i));
    end
    fclose(file);
end