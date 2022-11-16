const Colors = {
    red: '#B40513',
    brightRed: '#D60513',
    orange: 'rgba(219,77,0,0.75)',
    brightOrange: 'rgb(219,77,0)',
    grey: 'rgba(179,172,167,0.99)',
};

const mainTheme = {
    Colors: Colors,
    Button: {
        buttonStyle: {
            backgroundColor: Colors.red,
        },
    },
    MainView: {
        backgroundColor: '#fff',
    },
};

export {mainTheme as default, Colors};
