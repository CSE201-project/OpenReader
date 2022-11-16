function safe_writeMatrix(mat, filename, sheetIndex, range)

shouldTryToWrite = true;
nbTries = 0;
while shouldTryToWrite
    try
        nbTries = nbTries + 1;
        writematrix(mat,filename,'Sheet',sheetIndex,'Range',range,'UseExcel',ispc);
        shouldTryToWrite = false;
    catch ME
        if (nbTries > 3)
            rethrow(ME);
        end
        disp(strcat("****************** Error writing to Excel File (try nb ", num2str(nbTries), ") : ", ME.message, " **********************"));
    end
end

end