// cc-wavelet.h : Include file for standard system include files,
// or project specific include files.

#pragma once

int forward_transform(char* imageData, int width, int height, int horLevels, int vertLevels);
int inverse_transform(char* imageData, int width, int height, int horLevels, int vertLevels);
