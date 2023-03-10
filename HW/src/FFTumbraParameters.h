/*******************************************************************************
Dev. by: Keyvan Shahin
Project: FFTUmbra hardware implementation
*******************************************************************************/
#pragma once
#include <ap_fixed.h>
#include <hls_fft.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
//#include <complex>
#include "hls_x_complex.h"

//default tested parameters
#define img 28 //image size
#define fil 5 //filter size
#define fil_clip 2 // int(fil/2)
#define Val 4 // upper limit of image and filter arrays // barely used here, its the upper limit of the generated image and filter
#define P	8 // 3rd dimension of the array 2*Val

#define Dimension1  32 // img+fil-1
#define Dimension2  32 // img+fil-1
#define Dimension3  8 // P

#define FFT_LENGTH_xy 32 // xy fft
#define fft_size_xy 5 // log2(FFT_LENGTH)

#define FFT_LENGTH_z 8 // xy fft
#define fft_size_z 3 // log2(FFT_LENGTH)

//testing parameters
//#define img 60 //image size
//#define fil 5 //filter size
//#define fil_clip 2 // int(fil/2)
//#define Val 4 // upper limit of image and filter arrays // barely used here, its the upper limit of the generated image and filter
//#define P	8 // 3rd dimension of the array 2*Val
//
//#define Dimension1  64 // img+fil-1
//#define Dimension2  64 // img+fil-1
//#define Dimension3  8 // P
//
//#define FFT_LENGTH_xy 64 // xy fft
//#define fft_size_xy 6 // log2(FFT_LENGTH)
//
//#define FFT_LENGTH_z 8 // xy fft
//#define fft_size_z 3 // log2(FFT_LENGTH)


//typedef ap_fixed<2,2> fixed_t;
typedef ap_uint<1> fixed_t;


// for fixed_point fft
//#define FFT_INPUT_WIDTH 16
//#define FFT_OUTPUT_WIDTH 16
//typedef ap_fixed<FFT_INPUT_WIDTH,1> data_in_t;
//typedef hls::x_complex<data_in_t> complex_t;


//for floating_point fft
#define FFT_INPUT_WIDTH 32
#define FFT_OUTPUT_WIDTH 32
typedef float data_in_t;
//typedef std::complex<data_in_t> complex_t;
typedef hls::x_complex<data_in_t> complex_t;

const complex_t FFT_Scale = complex_t(FFT_LENGTH_xy*FFT_LENGTH_xy*FFT_LENGTH_z, 0.0);

//typedef int16_t Array2DVal_t;
//typedef double Array3DVal_t;

