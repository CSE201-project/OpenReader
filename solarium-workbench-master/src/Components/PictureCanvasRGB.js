'use strict';
import React from 'react';
import {
    Button,
    Box
} from '@material-ui/core';
import { red, green, blue, grey } from '@material-ui/core/colors';
import { rgba } from 'jimp';

const validChannelsValues = ['RGB', 'R', 'G', 'B'];
const canvasId = 'canvasRGB_Id';
        
export default function PictureCanvasRGB (props) {
    
    const [currentImageData, setCurrentImageData] = React.useState({});
    const [currentChannel, setCurrentChannel] = React.useState('RGB');

    const loadImage = async (image) => {
        return new Promise(resolve => { 
            let img = new Image(); 
            img.onload = (() => resolve(img)); 
            img.src = image 
        });
    }

    const loadImageInCanvas = async (img, rotated) => {
        try {
            let image = await loadImage(img);
            let canvas = document.getElementById(canvasId);        
            let imageContext = canvas.getContext('2d');

            if (rotated) {
                canvas.width = image.height;
                canvas.height = image.width;
                imageContext.clearRect(0, 0, canvas.width, canvas.height);
                imageContext.save();
                imageContext.translate(canvas.width/2, canvas.height/2);
                imageContext.rotate(-90*Math.PI/180);
                imageContext.drawImage(image, -image.width/2, -image.height/2);
                imageContext.restore();

            } else {
                canvas.width = image.width;
                canvas.height = image.height;
                imageContext.clearRect(0, 0, canvas.width, canvas.height);
                imageContext.drawImage(image, 0, 0, image.width, image.height);
            }

            let imageData = imageContext.getImageData(0, 0, canvas.width, canvas.height);        
            setCurrentImageData(imageData);
            loadChannelImage(currentChannel, imageData);
        }
        catch (err) {
            console.log('could not load image in canvas :', err);
        }
    };
    
    React.useEffect(() => {
        loadImageInCanvas(props.image, props.rotated);
    }, [props.image]);
    
    const handleMouseOver = (event) => {
        let relativeX = event.clientX - this.canvasRect.x;
        let relativeY = event.clientY - this.canvasRect.y;
        console.log("mouse over : ", relativeX, relativeY);

/*
        imageContext.save();
        imageContext.setTransform(1,0,0,1,0,0);
        imageContext.clearRect(0,0,this.canvas.width,this.canvas.height);
        imageContext.restore();
        */
    }
  
    const extractColorChannel = (imageData, channel) => {
        const colorMap = {R:0, G:1, B:2};
        const colorOffset = colorMap[channel] || 0;

        if (!imageData.data) { return imageData; }

        const channelImage = new Uint8ClampedArray(imageData.data);
        for (let i = 0; i < imageData.data.length; i += 4) {
            let channelValue = imageData.data[i + colorOffset];
            channelImage[i + 0] = channelValue;
            channelImage[i + 1] = channelValue;
            channelImage[i + 2] = channelValue;
            channelImage[i + 3] = imageData.data[i + 3];
        }
        return new ImageData(channelImage, imageData.width, imageData.height);
    }

    const loadChannelImage = (colorChannel, imageData) => {
        if (imageData && imageData.data) {
            let imageContext = document.getElementById(canvasId).getContext('2d');

            if (colorChannel == 'RGB') {
                imageContext.putImageData(imageData, 0, 0);
            } else {
                let colorChannelData = extractColorChannel(imageData, colorChannel);
                imageContext.putImageData(colorChannelData, 0, 0);
            }
        }
    }
    
    const showChannel = (channel) => {
        let colorChannel = validChannelsValues.includes(channel) ? channel : 'RGB';
        setCurrentChannel(colorChannel);
        loadChannelImage(colorChannel, currentImageData);
    }

    return (
        <Box>
            <canvas id={canvasId} style={styles.canvas}></canvas>
            <Box>
                {validChannelsValues.map((value) => {
                    let variant = (currentChannel === value) ? 'outlined' : 'contained'; 
                    let size = (currentChannel === value) ? 'large' : 'small';                     
                    let textStyle = (currentChannel === value) ? {textDecoration: 'underline'} : {};  
                    return <Button variant={variant} size={size} label={styles.currentButton} style={{...styles.buttons[value], ...textStyle}} onClick={() => showChannel(value)}>{value}</Button>
                
                })}
            </Box>
        </Box>
    );
}

const styles = {
    canvas: {
        border: 'solid 1px black',
        width: '90%',
        height: '90%',
        marginLeft: '60px'
    },
    buttons : {
        RGB : {
            backgroundColor: grey[900],
            color: 'white'
        },
        R : {
            backgroundColor: red[900],
            color: 'white'
        },
        G : {
            backgroundColor: green[900],
            color: 'white'
        },
        B : {
            backgroundColor: blue[900],
            color: 'white'
        }
    }
}
