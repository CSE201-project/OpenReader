exports.colorChannels =
{
    Red: 0,
    Green: 1,
    Blue: 2
};

exports.histogramRGB = function (channel, jimpImage, colorDepth) {
    var depth;

    if (colorDepth === 12)
        depth = 4096;
    else if (colorDepth === 10)
        depth = 1024;
    else
        depth = 256;
    const colourFrequencies = getColourFrequencies(channel, jimpImage, depth);
    const histogram = createHistogram(channel, colourFrequencies.colourFrequencies, colourFrequencies.maxFrequency, depth);
    return histogram;
}

function getFrequenciesArray(min, max) {
    let array = [];
    for (let i = 0; min < max; i++) {
        array[i] = min;
        min++;
    }
    return array;
}

function getMedian(array){
    array.sort(function(a, b){ return a - b; });
    var i = array.length / 2;
    return i % 1 === 0 ? (array[i - 1] + array[i]) / 2 : array[Math.floor(i)];
  }

function getColourFrequencies(channel, jimpImage, depth) {
    const startIndex = channel;
    let maxFrequency = 0;
    const colourFrequencies = Array(depth).fill(0);

    for (let i = startIndex, len = jimpImage.bitmap.data.length; i < len; i += 4) {
        colourFrequencies[jimpImage.bitmap.data[i]]++;

        if (colourFrequencies[jimpImage.bitmap.data[i]] > maxFrequency) {
            maxFrequency++;
        }
    }

    const result = {
        colourFrequencies: colourFrequencies,
        maxFrequency: maxFrequency
    }
    return result;
}

function createHistogram(channel, colourFrequencies, maxFrequency, depth) {
    const histWidth = 512;
    const histHeight = 105;
    const columnWidth = 2;
    const pixelsPerUnit = histHeight / maxFrequency;

    let hexColour;
    let x = 0;
    let columnHeight;

    let svgstring = `<svg width='${histWidth}px' height='${histHeight}px' xmlns='http://www.w3.org/2000/svg' version='1.1' xmlns:xlink='http://www.w3.org/1999/xlink'>\n`;

    let histObject = {};
    let min = 1000000;
    let moy = 0;
    let pos;
    let max;

    for (pos = 0; pos <= (depth - 1); pos++) {
        switch (channel) {
            case exports.colorChannels.Red:
                hexColour = "#d32f2f";
                break;
            case exports.colorChannels.Green:
                hexColour = "#388e3c";
                break;
            case exports.colorChannels.Blue:
                hexColour = "#1976d2";
                break;
            default:
                break;
        }
        columnHeight = colourFrequencies[pos] * pixelsPerUnit;
        if (min > colourFrequencies[pos])
        {
            var tmp = pos;
            min = colourFrequencies[pos];
        }
        if (maxFrequency === colourFrequencies[pos])
            max = pos;
        svgstring += `<rect fill='${hexColour}' stroke='${hexColour}' stroke-width='0.1px' width='${columnWidth}' height='${columnHeight}' y='${histHeight - columnHeight}' x='${x}' />\n`;
        x += columnWidth;
        moy += pos;
    }
    let frequenciesArray = getFrequenciesArray(min, max);
    let med = getMedian(frequenciesArray);
    moy /= depth;
    min = tmp;
    svgstring += "</svg>";

    histObject.svgstring = svgstring;
    histObject.min = min;
    histObject.max = max;
    histObject.moy = moy;
    histObject.med = med;
    histObject.mod = max;

    return histObject;
}