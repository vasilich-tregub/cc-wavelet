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
    document.getElementById("idCanvas").width = width;
    document.getElementById("idCanvas").height = height;
    const ctx = document.getElementById("idCanvas").getContext("2d", { willReadFrequently: true });
    ctx.drawImage(img, 0, 0);
    imageData = ctx.getImageData(0, 0, width, height);
    // packed to planar color representation, and dwt for each bitplane
    imR = new Int32Array(new ArrayBuffer(width * height * 4));
    imG = new Int32Array(new ArrayBuffer(width * height * 4));
    imB = new Int32Array(new ArrayBuffer(width * height * 4));
    for (let ih = 0; ih < height; ++ih) {
        for (let iw = 0; iw < width; ++iw) {
            imR[ih * width + iw] = imageData.data[(ih * width + iw) * 4 + 0] << 12;
            imG[ih * width + iw] = imageData.data[(ih * width + iw) * 4 + 1] << 12;
            imB[ih * width + iw] = imageData.data[(ih * width + iw) * 4 + 2] << 12;
        }
    }

    horLevels = idHorizontalLevels.value;
    vertLevels = idVerticalLevels.value;
    let startTime = performance.now();
    for (let level = 0; level < vertLevels; ++level) {
        forward_transform_vertical(level);
    }
    for (let level = 0; level < horLevels; ++level) {
        forward_transform_horizontal(level);
    }
    let finishTime = performance.now();
    for (let ih = 0; ih < height; ++ih) {
        for (let iw = 0; iw < width; ++iw) {
            imageData.data[(ih * width + iw) * 4 + 0] = (imR[ih * width + iw] >> 12);
            imageData.data[(ih * width + iw) * 4 + 1] = (imG[ih * width + iw] >> 12);
            imageData.data[(ih * width + iw) * 4 + 2] = (imB[ih * width + iw] >> 12);
        }
    }
    ctx.putImageData(imageData, 0, 0);
    idPerf.value = (finishTime - startTime).toString();
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

    let startTime = performance.now();
    for (let level = horLevels - 1; level >= 0; --level) {
        inverse_transform_horizontal(level);
    }
    for (let level = vertLevels - 1; level >= 0; --level) {
        inverse_transform_vertical(level);
    }
    let finishTime = performance.now();
    for (let ih = 0; ih < height; ++ih) {
        for (let iw = 0; iw < width; ++iw) {
            imageData.data[(ih * width + iw) * 4 + 0] = imR[ih * width + iw] >> 12;
            imageData.data[(ih * width + iw) * 4 + 1] = imG[ih * width + iw] >> 12;
            imageData.data[(ih * width + iw) * 4 + 2] = imB[ih * width + iw] >> 12;
        }
    }
    ctx.putImageData(imageData, 0, 0);
    idPerf.value = (finishTime - startTime).toString();
}
function inverse_transform_horizontal(level) {
    for (let ih = 0; ih < height; ++ih) {
        dwt_inverse(imR, ih * width, width, 1, level);
        dwt_inverse(imG, ih * width, width, 1, level);
        dwt_inverse(imB, ih * width, width, 1, level);
    }
}
function inverse_transform_vertical(level) {
    for (let iw = 0; iw < idCanvas.width; ++iw) {
        dwt_inverse(imR, iw, imgsize, width, level);
        dwt_inverse(imG, iw, imgsize, width, level);
        dwt_inverse(imB, iw, imgsize, width, level);
    }
}
function dwt_forward(im, beg, maxindexval, indexdiff, level) { // indexdiff = (hor vs. vert) ? 1 : bitmap_stride;
    const inc = indexdiff << level;
    const end = beg + maxindexval;
    //assert(inc < end && "stepping outside source image");

    let i = beg + inc;
    // high pass filter, {-1./2, 1., -1./2}
    for (; i < end - inc; i += 2 * inc) {
        im[i] -= (im[i - inc] + im[i + inc]) >> 1;
    }
    if (i < end) {
        im[i] -= im[i - inc];
    }

    i = beg;
    // low pass filter, 
    // successive convolutions with {-1./2, 1., -1./2} for odd pixels
    // and {1./4, 1., 1./4} for even pixels
    // for im[n] result is -im[n-2]/8 + im[n-1]/4 + 6*im[n]/8 + im[n+1]/4 - im[n+2]/8
    // i.e., {-1./8, 2./8, 6./8, 2./8, -1./8}
    im[i] += (im[i + inc] + 1) >> 1;
    i += 2 * inc;
    for (; i < end - inc; i += 2 * inc) {
        im[i] += (im[i - inc] + im[i + inc] + 2) >> 2;
    }
    if (i < end) {
        im[i] += (im[i - inc] + 1) >> 1;
    }
}
function dwt_inverse(im, beg, maxindexval, indexdiff, level) { // indexdiff = (hor vs. vert) ? 1 : bitmap_stride;
    const inc = indexdiff << level;
    const end = beg + maxindexval;
    //assert(inc < end && "stepping outside source image");

    // low pass filter, {-1./4, 1./4, -1./4}
    let i = beg;
    im[i] -= (im[i + inc] + 1) >> 1;
    i += 2 * inc;
    for (; i < end - inc; i += 2 * inc) {
        im[i] -= (im[i - inc] + im[i + inc] + 2) >> 2;
    }
    if (i < end) {
        im[i] -= (im[i - inc] + 1) >> 1;
    }

    // high pass filter, {-1./8, 1./8, 6./8, 1./8 -1./8}
    // successive convolutions with {-1./4, 1., -1./4} for even pixels
    // and {1./2, 1., 1./2} for even pixels
    // for im[n] result is -im[n-2]/8 + im[n-1]/8 + 6*im[n]/8 + im[n+1]/8 - im[n+2]/8
    i = beg + inc;
    for (; i < end - inc; i += 2 * inc) {
        im[i] += (im[i - inc] + im[i + inc]) >> 1;
    }
    if (i < end) {
        im[i] += im[i - inc];
    }
}
