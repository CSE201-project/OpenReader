function [studyName, concentrationText, concentration, imageNumber]=parseFileName(filename)

pointsIndex = strfind(filename, '.');
filename = filename(1:pointsIndex(length(pointsIndex)) - 1);

k = strfind(filename,'_');
if length(k) == 2
    idxStudyName = [1 (k(1) - 1)];
    idxConcentration = [(k(1) + 1) (k(2) - 1)];
    idxNumber = [(k(2) + 1) (length(filename))];
else 
    len = length(k);
    underscore = [(k(len - 2) + 1) k(len -1) k(len)];
    idxStudyName = [underscore(1) (underscore(2) - 1)];
    idxConcentration = [(underscore(2) + 1) (underscore(3) - 1)];
    idxNumber = [(underscore(3) + 1) (length(filename))];
end

studyName = filename(idxStudyName(1):idxStudyName(2));
concentrationText = filename(idxConcentration(1):idxConcentration(2));
concentration = str2double(concentrationText(1: length(concentrationText) - 4));
imageNumber = filename(idxNumber(1):idxNumber(2));

end
