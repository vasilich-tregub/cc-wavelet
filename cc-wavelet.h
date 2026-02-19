// cc-wavelet.h : Include file for standard system include files,
// or project specific include files.

#pragma once

int forward_transform(uint8_t* imageData, int _width, int _height, int _horLevels, int _vertLevels);
int inverse_transform(uint8_t* imageData, int _width, int _height, int _horLevels, int _vertLevels);

extern int width;
extern int height;
extern int horLevels;
extern int vertLevels;
