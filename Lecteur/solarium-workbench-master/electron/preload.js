const { ipcRenderer } = require('electron');
const { app, dialog } = require('electron').remote;
const fs = require('fs');
const path = require('path');
const Jimp = require("jimp");
const {NodeSSH} = require('node-ssh');

window.ipcRenderer = ipcRenderer;
window.dialog = dialog;
window.fs = fs;
window.Jimp = Jimp;
window.configDir = app.getPath('userData');
window.path = path;
window.nodeSsh = NodeSSH;