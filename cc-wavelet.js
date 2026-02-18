/* TERMS OF USE
 * This source code is subject to the terms of the MIT License. 
 * Copyright(c) 2026 Vladimir Vasilich Tregub
*/
var imR;
var imG;
var imB;
var imageData;
var width;
var height;
var imgsize;
var horLevels;
var vertLevels;
function forward_transform(img) {
    width = img.naturalWidth;
    height = img.naturalHeight;
    imgsize = width * height;
    horLevels = idHorizontalLevels.value;
    vertLevels = idVerticalLevels.value;
    document.getElementById("idCanvas").width = width;
    document.getElementById("idCanvas").height = height;
    const ctx = document.getElementById("idCanvas").getContext("2d", { willReadFrequently: true });
    ctx.drawImage(img, 0, 0);

    const xhr = new XMLHttpRequest();
    xhr.open("GET", '/?width=' + width.toString() + '&height=' + height.toString() +
        '&horLevels=' + horLevels.toString() + '&vertLevels=' + vertLevels.toString(), false);
    try {
        xhr.send();
        if (xhr.status != 200) {
            alert(xhr.statusText);
        }
        else {
            idPerf.value = xhr.response;
        }
    }
    catch (err) {
        alert('xhttp GET request failed');
    }

    imageData = ctx.getImageData(0, 0, width, height);

    xhr.open("POST", '/DWT', false);
    let starttime = performance.now();
    xhr.send(imageData.data);
    let finishtime = performance.now();

    let responseString = xhr.responseText;
    for (let i = 0; i < imageData.data.length; ++i) {
        imageData.data[i] = (responseString.charCodeAt(2 * i) & 0x7f) + responseString.charCodeAt(2 * i + 1) * 128;
    }
    idPerf.value = ((finishtime - starttime).toString());
    ctx.putImageData(imageData, 0, 0);

}

function forward_transform_horizontal(level) {
    for (let ih = 0; ih < height; ++ih) {
        dwt_forward(imR, ih * width, width, 1, level);
        dwt_forward(imG, ih * width, width, 1, level);
        dwt_forward(imB, ih * width, width, 1, level);
    }
}
function forward_transform_vertical(level) {
    for (let iw = 0; iw < width; ++iw) {
        dwt_forward(imR, iw, imgsize, width, level);
        dwt_forward(imG, iw, imgsize, width, level);
        dwt_forward(imB, iw, imgsize, width, level);
    }
}
function inverse_transform() {
    const ctx = document.getElementById("idCanvas").getContext("2d", { willReadFrequently: true });
    const xhr = new XMLHttpRequest();
    xhr.open("POST", '/iDWT', false);
    let starttime = performance.now();
    xhr.send(imageData.data);
    let finishtime = performance.now();

    let responseString = xhr.responseText;
    for (let i = 0; i < imageData.data.length; ++i) {
        imageData.data[i] = (responseString.charCodeAt(2 * i) & 0x7f) + responseString.charCodeAt(2 * i + 1) * 128;
    }
    idPerf.value = ((finishtime - starttime).toString());
    ctx.putImageData(imageData, 0, 0);
}
