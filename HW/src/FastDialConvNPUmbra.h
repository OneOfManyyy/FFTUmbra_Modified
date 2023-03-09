/*******************************************************************************
Dev. by: Keyvan Shahin
Project: FFTUmbra hardware implementation
*******************************************************************************/
#pragma once

#include "FFTumbraParameters.h"
#include <stdint.h>
#include <math.h>
#include <cmath>
#include <hls_fft.h>
#include "hls_stream.h"


void FastDialConvNPUmbra (
		hls::stream<int16_t> &iImageArray2D_stream,
		hls::stream<int16_t> &iFilterArray2D_stream,
		hls::stream<int16_t> &oArray2D_stream
//		,hls::stream<complex_t> &oTest_stream
	);
