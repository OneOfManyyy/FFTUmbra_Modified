#include <stdio.h>
#include "ap_int.h"

#include "FastDialConvNPUmbra.h"

int main() {
//    const int img = 32;  // input image size
//    const int fil = 3;  // filter size

//    FILE *test = fopen("../../../test.txt", "w");
//    if (!test) {
//        printf("Failed to open test file.\n");
//        return 1;
//    }
//
//    printf("opened Test.\n");
//    fprintf(test, "%s ", "testing....");
//    printf("wrote to Test.\n");
//    fclose(test);

    // Open input files
    FILE *imgFile = fopen("../../../simfiles/image.txt", "r");
    FILE *filFile = fopen("../../../simfiles/filter.txt", "r");
    if (!imgFile || !filFile) {
        printf("Failed to open input files.\n");
        return 1;
    }

    printf("opened input files.\n");

    // Read input arrays from files
    int16_t iImageArray2D[img][img];
    int16_t iFilterArray2D[fil][fil];
    for (int i = 0; i < img; ++i) {
        for (int j = 0; j < img; ++j) {
            fscanf(imgFile, "%hd", &iImageArray2D[i][j]);
//            printf("%d %d %d \n",iImageArray2D[i][j], i, j);
        }
    }
    for (int i = 0; i < fil; ++i) {
        for (int j = 0; j < fil; ++j) {
            fscanf(filFile, "%hd", &iFilterArray2D[i][j]);
        }
    }

    // Close input files
    fclose(imgFile);
    fclose(filFile);

    printf("closed input files.\n");

    // Open output file
    FILE* outFile = fopen("../../../simfiles/c_hls.txt", "w");
//    FILE* testFile = fopen("../../../simfiles/test.txt", "w");
    if (!outFile) {
        printf("Failed to open output file.\n");
        return 1;
    }

    printf("opened output files.\n");

    // Stream input arrays into DUT
    hls::stream<int16_t> iImageArray2D_stream("iImageArray2D_stream");
    hls::stream<int16_t> iFilterArray2D_stream("iFilterArray2D_stream");
    hls::stream<int16_t> oArray2D_stream("oArray2D_stream");
    hls::stream<complex_t> oTest_stream("oTest_stream");

    for (int i = 0; i < img; ++i) {
        for (int j = 0; j < img; ++j) {
            iImageArray2D_stream.write(iImageArray2D[i][j]);
        }
    }
    for (int i = 0; i < fil; ++i) {
        for (int j = 0; j < fil; ++j) {
            iFilterArray2D_stream.write(iFilterArray2D[i][j]);
        }
    }

    printf("will run FastDialConvNPUmbra now!\n");

    // Run DUT
//    FastDialConvNPUmbra(iImageArray2D_stream, iFilterArray2D_stream, oArray2D_stream, oTest_stream);
    FastDialConvNPUmbra(iImageArray2D_stream, iFilterArray2D_stream, oArray2D_stream);
    printf("FastDialConvNPUmbra done!\n");

    // Wait until oArray2D_stream has data available
    while (oArray2D_stream.empty()) {}

    printf("oArray2D_stream ready!\n");

    // Read output from DUT and write to file
//    for (int i = 0; i < img-fil+1; ++i) {
//        for (int j = 0; j < img-fil+1; ++j) {
    for (int i = 0; i < img; ++i) {
    	for (int j = 0; j < img; ++j) {
    		int16_t o;
            oArray2D_stream.read(o);
//            printf("reading output!\n");
            fprintf(outFile, "%d ", o);
        }
        fprintf(outFile, "\n");
    }
    printf("reading output done!\n");
//test output
//    for (int i = 0; i < Dimension1; ++i) {
//        for (int j = 0; j < Dimension2; ++j) {
//            for (int k = 0; k < Dimension3; ++k) {
//            	complex_t co;
//            	oTest_stream.read(co);
//            	fprintf(testFile, "%f,%f\n", real(co),imag(co));
//            }
//        }
//    }
    // Close output file
    fclose(outFile);
//    fclose(testFile);

    if (system("diff -w ../../../simfiles/c_hls.txt ../../../simfiles/c.txt")) {

  	fprintf(stdout, "*******************************************\n");
  	fprintf(stdout, "FAIL: Output DOES NOT match the golden output\n");
  	fprintf(stdout, "*******************************************\n");
       return 1;
    } else {
  	fprintf(stdout, "*******************************************\n");
  	fprintf(stdout, "PASS: The output matches the golden output!\n");
  	fprintf(stdout, "*******************************************\n");
       return 0;
    }
}
