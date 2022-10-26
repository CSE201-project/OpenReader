function resultingImagesInfos = orderImagesByConcentration(imagesInfos, measureType)

    concentrationIndexesMap = containers.Map('KeyType','double','ValueType','any');    
    for i=1:length(imagesInfos)
        
        concentration = imagesInfos(i).FileInfo.Concentration;
        concentrationIndexes = [];
        if (concentrationIndexesMap.isKey(concentration))
            concentrationIndexes = concentrationIndexesMap(concentration); 
        end
        concentrationIndexes = [concentrationIndexes i];
        concentrationIndexesMap(concentration) = concentrationIndexes;

    end
           
    sortDirection = 'ascend';
%     if strcmp (measureType,'Competition/Inhibition')
%         sortDirection = 'descend';
%     end
    
    concentrationOrder = sort(cell2mat(keys(concentrationIndexesMap)), sortDirection);
    resultingImagesInfosIdx = 1;
    for i=1:length(concentrationOrder)
        indexes = concentrationIndexesMap(concentrationOrder(i));
        if (~isempty(indexes))
            for j=1:length(indexes)
                t = imagesInfos(indexes(j));
                resultingImagesInfos(resultingImagesInfosIdx) = (t);
                resultingImagesInfosIdx = resultingImagesInfosIdx + 1;
            end
        end
    end

end
