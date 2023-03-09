/*******************************************************************************
Dev. by: Keyvan Shahin
Project: FFTUmbra hardware implementation
*******************************************************************************/

#include "FastDialConvNPUmbra.h"

    struct param1 : hls::ip_fft::params_t {
        static const unsigned input_width = FFT_INPUT_WIDTH;
        static const unsigned output_width = FFT_INPUT_WIDTH;
        static const unsigned max_nfft = fft_size_xy;
        static const unsigned config_width = 8;
        static const unsigned phase_factor_width = 24;
        static const unsigned ordering_opt = hls::ip_fft::natural_order;
    };
    struct param2 : hls::ip_fft::params_t {
        static const unsigned input_width = FFT_INPUT_WIDTH;
        static const unsigned output_width = FFT_INPUT_WIDTH;
        static const unsigned max_nfft = fft_size_z;
        static const unsigned config_width = 8;
        static const unsigned phase_factor_width = 24;
        static const unsigned ordering_opt = hls::ip_fft::natural_order;
    };

    // Define FFT configuration
    typedef hls::ip_fft::config_t<param1> config1_t;
    typedef hls::ip_fft::status_t<param1> status1_t;
    typedef hls::ip_fft::config_t<param2> config2_t;
    typedef hls::ip_fft::status_t<param2> status2_t;

    config1_t fft_config1;
    status1_t fft_status1;
    config2_t fft_config2;
    status2_t fft_status2;


void FastDialConvNPUmbra (
				hls::stream<int16_t> &iImageArray2D_stream,
				hls::stream<int16_t> &iFilterArray2D_stream,
				hls::stream<int16_t> &oArray2D_stream
//				,hls::stream<complex_t> &oTest_stream
	) {
//Default pargmas for ports
#pragma HLS STREAM depth=28 variable=iImageArray2D_stream
#pragma HLS STREAM depth=6 variable=iFilterArray2D_stream
#pragma HLS STREAM depth=28 variable=oArray2D_stream

//#pragma HLS STREAM depth=60 variable=iImageArray2D_stream
//#pragma HLS STREAM depth=5 variable=iFilterArray2D_stream
//#pragma HLS STREAM depth=60 variable=oArray2D_stream

//#pragma HLS STREAM depth=28 variable=oTest_stream


int16_t iImageArray2D[img][img];
int16_t iFilterArray2D[fil][fil];
//float oArray2D[img][img];



#pragma HLS array_partition dim=2 complete variable=iImageArray2D
#pragma HLS array_partition dim=2 complete variable=iFilterArray2D
//#pragma HLS array_partition dim=2 complete variable=oArray2D

for (int i = 0; i < img; i++) {
    for (int j = 0; j < img; j++) {
    	iImageArray2D[i][j] = iImageArray2D_stream.read();
//    	printf("iImageArray2D[%d][%d]: %d\n", i,j, iImageArray2D[i][j]);

    }
}
for (int i = 0; i < fil; i++) {
    for (int j = 0; j < fil; j++) {
    	iFilterArray2D[i][j] = iFilterArray2D_stream.read();
    }
}

printf("Inside FastDialConvNPUmbra: Start \n");

	static fixed_t sImageArray3D[img][img][P];
#pragma HLS array_partition dim=3 complete variable=sImageArray3D
	static fixed_t sFilterArray3D[fil][fil][P];
#pragma HLS array_partition dim=3 complete variable=sFilterArray3D
	int s2DArraySelectedIndexValue;


////////////////////////////////// Convert to 3D Arrays ///////////////////////////////////
////////////////////////////////////////// Start //////////////////////////////////////////

//	ConvertTo3D Image
//	fixed_t testvar;
	for(int x=0; x<img; x++) {
////#pragma HLS DATAFLOW
		for(int y=0; y<img; y++) {
			for(int z=0; z<P; z++) {  //set all 3d array elements to 0
#pragma HLS PIPELINE II=1
				sImageArray3D[x][y][z] = 0;
			}
			if (iImageArray2D[x][y] >= 0){
				if (iImageArray2D[x][y] > Val-1){
					sImageArray3D[x][y][Val-1] = fixed_t(1);
					printf("Saturated!!! \n"); //should not happen
				} else
				{
					sImageArray3D[x][y][iImageArray2D[x][y]] = fixed_t(1);
				}
			}

		}
	}



//	ConvertTo3D Filter

	for(int x=0; x<fil; x++) {
////#pragma HLS DATAFLOW
		for(int y=0; y<fil; y++) {
			for(int z=0; z<P; z++) {  //set all 3d array elements to 0
#pragma HLS PIPELINE II=1
				sFilterArray3D[x][y][z] = 0;
			}

			if (iFilterArray2D[x][y] >= 0){
				if (iFilterArray2D[x][y] > Val-1){
					sFilterArray3D[x][y][Val-1] = fixed_t(1);
				} else
				{
					sFilterArray3D[x][y][iFilterArray2D[x][y]] = fixed_t(1);
				}
			}
		}
	}

////////////////////////////////// Convert to 3D Arrays ///////////////////////////////////
/////////////////////////////////////////// End ///////////////////////////////////////////

printf("Inside FastDialConvNPUmbra: Convert3D Over! \n");

////////////////////////////////////// Conv3DnpTest ///////////////////////////////////////
////////////////////////////////////////// Start //////////////////////////////////////////

	static fixed_t a_pad[Dimension1][Dimension2][Dimension3];
#pragma HLS array_partition dim=3 complete variable=a_pad
	static fixed_t k_pad[Dimension1][Dimension2][Dimension3];
#pragma HLS array_partition dim=3 complete variable=k_pad

	// Padding
    for (int x = 0; x < Dimension1; x++) {
////#pragma HLS DATAFLOW
        for (int y = 0; y < Dimension2; y++) {
            for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                if (x < img && y<img ) {
                    a_pad[x][y][z] = sImageArray3D[x][y][z];
                } else {
                    a_pad[x][y][z] = 0;
                }
                if (x < fil && y<fil ) {
                    k_pad[x][y][z] = sFilterArray3D[x][y][z];
                } else {
                    k_pad[x][y][z] = 0;
                }
            }
        }
    }


	fft_config1.setDir(1); // Forward FFT
	fft_config2.setDir(1); // Forward FFT


    /////////////////////////////// FFT3D for a_pad and k_pad /////////////////////////////////
    ////////////////////////////////////////// Start //////////////////////////////////////////
	// Create a temporary array to hold the output data
	static complex_t img_output[Dimension1][Dimension2][Dimension3];
#pragma HLS array_partition dim=3 complete variable=img_output
	static complex_t fil_output[Dimension1][Dimension2][Dimension3];
#pragma HLS array_partition dim=3 complete variable=fil_output

printf("Inside FastDialConvNPUmbra: FFT Start! \n");

    // Perform the 1D FFT along the first dimension (rows)
    	for (int z = 0; z < Dimension3; z++) {
////#pragma HLS DATAFLOW
            for (int y = 0; y < Dimension2; y++) {
            	complex_t img_row_input[Dimension1];
            	complex_t img_row_output[Dimension1];
            	complex_t fil_row_input[Dimension1];
            	complex_t fil_row_output[Dimension1];

                for (int x = 0; x < Dimension1; x++) {
#pragma HLS PIPELINE II=1
                	img_row_input[x] = complex_t(a_pad[x][y][z],0);
                	fil_row_input[x] = complex_t(k_pad[x][y][z],0);
                }
                // Perform the FFTs
#pragma HLS INLINE off
                hls::fft<param1>(img_row_input, img_row_output, &fft_status1, &fft_config1); //param1 for x dimension
#pragma HLS INLINE off
                hls::fft<param1>(fil_row_input, fil_row_output, &fft_status1, &fft_config1);

                for (int x = 0; x < Dimension1; x++) {
#pragma HLS PIPELINE II=1
                	img_output[x][y][z] = img_row_output[x];
                	fil_output[x][y][z] = fil_row_output[x];
                }
            }
        }

printf("Inside FastDialConvNPUmbra: FFT Rows Over! \n");
        // Perform the 1D FFT along the second dimension (columns)
        for (int z = 0; z < Dimension3; z++) {
            for (int x = 0; x < Dimension1; x++) {
            	complex_t img_col_input[Dimension2];
            	complex_t img_col_output[Dimension2];
            	complex_t fil_col_input[Dimension2];
            	complex_t fil_col_output[Dimension2];

                for (int y = 0; y < Dimension2; y++) {
#pragma HLS PIPELINE II=1
                	img_col_input[y] = img_output[x][y][z];
                	fil_col_input[y] = fil_output[x][y][z];
                }
                // Perform the FFTs
#pragma HLS INLINE off
                hls::fft<param1>(img_col_input, img_col_output, &fft_status1, &fft_config1); //param1 for y dimension
#pragma HLS INLINE off
                hls::fft<param1>(fil_col_input, fil_col_output, &fft_status1, &fft_config1);

                for (int y = 0; y < Dimension2; y++) {
#pragma HLS PIPELINE II=1
                	img_output[x][y][z] = img_col_output[y];
                	fil_output[x][y][z] = fil_col_output[y];
                }
            }
        }

        // Perform the 1D FFT along the third dimension (depth)
        for (int y = 0; y < Dimension2; y++) {
////#pragma HLS DATAFLOW
            for (int x = 0; x < Dimension1; x++) {
            	complex_t img_depth_input[Dimension3];
            	complex_t img_depth_output[Dimension3];
            	complex_t fil_depth_input[Dimension3];
            	complex_t fil_depth_output[Dimension3];

                for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                	img_depth_input[z] = img_output[x][y][z];
                	fil_depth_input[z] = fil_output[x][y][z];
                }
                // Perform the FFTs
#pragma HLS INLINE off
                hls::fft<param2>(img_depth_input, img_depth_output, &fft_status2, &fft_config2); //param2 for z dimension
#pragma HLS INLINE off
                hls::fft<param2>(fil_depth_input, fil_depth_output, &fft_status2, &fft_config2);

                for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                	img_output[x][y][z] = img_depth_output[z]; // This is "A" from python code
                	fil_output[x][y][z] = fil_depth_output[z]; // This is "K" from python code
                }
            }
        }

        /////////////////////////////// FFT3D for a_pad and k_pad /////////////////////////////////
        /////////////////////////////////////////// End ///////////////////////////////////////////

        //element-wise multiplication
        static complex_t CMult[Dimension1][Dimension2][Dimension3];
        for (int x = 0; x < Dimension1; x++) {
            for (int y = 0; y < Dimension2; y++) {
                for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                	CMult[x][y][z]=img_output[x][y][z] * fil_output[x][y][z] / FFT_Scale; // doing the scaling required for inverse FFT here before the FFTs themselves. this helps with avoiding the overflow
                }
            }
        }



    	fft_config1.setDir(0); // Inverse FFT
    	fft_config2.setDir(0); // Inverse FFT

        //////////////////////////////// Inverse FFT3D for CMult //////////////////////////////////
        ////////////////////////////////////////// Start //////////////////////////////////////////
    	// Create a temporary array to hold the output data
        static complex_t C_output[Dimension1][Dimension2][Dimension3];
#pragma HLS array_partition dim=3 complete variable=C_output

        // Perform the 1D FFT along the first dimension (rows)
        	for (int z = 0; z < Dimension3; z++) {
                for (int y = 0; y < Dimension2; y++) {
                	static complex_t C_row_input[Dimension1];
                	static complex_t C_row_output[Dimension1];

                    for (int x = 0; x < Dimension1; x++) {
#pragma HLS PIPELINE II=1
                    	C_row_input[x] = CMult[x][y][z];
                    }
                    // Perform the FFTs
#pragma HLS INLINE off
                    hls::fft<param1>(C_row_input, C_row_output, &fft_status1, &fft_config1); //param1 for x dimension

                    for (int x = 0; x < Dimension1; x++) {
#pragma HLS PIPELINE II=1
                    	C_output[x][y][z] = C_row_output[x];
                    }
                }
            }

            // Perform the 1D FFT along the second dimension (columns)
            for (int z = 0; z < Dimension3; z++) {
                for (int x = 0; x < Dimension1; x++) {
                	static complex_t C_col_input[Dimension2];
                	static complex_t C_col_output[Dimension2];

                    for (int y = 0; y < Dimension2; y++) {
#pragma HLS PIPELINE II=1
                    	C_col_input[y] = C_output[x][y][z];
                    }
                    // Perform the FFTs
#pragma HLS INLINE off
                    hls::fft<param1>(C_col_input, C_col_output, &fft_status1, &fft_config1); //param1 for y dimension

                    for (int y = 0; y < Dimension2; y++) {
#pragma HLS PIPELINE II=1
                    	C_output[x][y][z] = C_col_output[y];
                    }
                }
            }

            // Perform the 1D FFT along the third dimension (depth)
            for (int y = 0; y < Dimension2; y++) {
////#pragma HLS DATAFLOW
                for (int x = 0; x < Dimension1; x++) {
                	static complex_t C_depth_input[Dimension3];
                	static complex_t C_depth_output[Dimension3];

                    for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                    	C_depth_input[z] = C_output[x][y][z];
                    }
                    // Perform the FFTs
#pragma HLS INLINE off
                    hls::fft<param2>(C_depth_input, C_depth_output, &fft_status2, &fft_config2); //param2 for z dimension

                    for (int z = 0; z < Dimension3; z++) {
#pragma HLS PIPELINE II=1
                    	C_output[x][y][z] = C_depth_output[z]; // This is "c" from python code before cutting
                    }
                }
            }

            //////////////////////////////// Inverse FFT3D for CMult //////////////////////////////////
            /////////////////////////////////////////// End ///////////////////////////////////////////



// Combined 3D to 2D array and c Array cropping to overcome the problems with II Violations
int real_part;
// Copy the subarray of c into the center of the padded array
for (int x = 0; x < Dimension1; x++) {
	for (int y = 0; y < Dimension2; y++) {
        for (int z = P-1; z >= 0; z--) {
#pragma HLS PIPELINE II=1
			if (x>=fil_clip && x<(img+fil_clip) && y>=fil_clip && y<(img+fil_clip)){
				 real_part = hls::round(C_output[x][y][z].real()); // Final c
				 if (real_part >= 1) {
					 if (z >= Val-1) {
						 oArray2D_stream.write(Val-1);
					 } else
					 {
						 oArray2D_stream.write(z);
					 }
					 break;
				 }
			}
		}
	}
}

////////////////////////////////////// Conv3DnpTest ///////////////////////////////////////
/////////////////////////////////////////// End ///////////////////////////////////////////
//test ouput
//for (int x = 0; x < Dimension1; x++) {
//	for (int y = 0; y < Dimension2; y++) {
//        for (int z = 0; z < Dimension3; z++) {
//        	oTest_stream.write(C_output[x][y][z]);
//        }
//	}
//}
}
