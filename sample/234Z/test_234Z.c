// @file   test_234Z.c

// @brief  Test program for 234ZCompositor
//         Offline 234Compositor for RGBA32+Z32 Images
//         mpicc -o test_234Z test_234Z.c -lm lib234comp.a 

// @author Jorji Nonaka (jorji@riken.jp)

#include "234compositor.h"

#include <assert.h>
#include <stdio.h>  

// MPI Library
#ifndef _MPI_INCLUDE
	#include <mpi.h>
	#define _MPI_INCLUDE
#endif

int write_rgba_image ( char*, BYTE*, unsigned int, unsigned int, unsigned int );

/*=========================================*/
/**
 *  @brief Main Program
 */
/*=========================================*/
int main( int argc, char* argv[] )
{
	int rank;
	int nnodes;

	unsigned int width, height, image_size;
	unsigned int image_type, pixel_size;

	BYTE* rgba_image;
	BYTE* composited_image;

	float* z_image;

	BYTE* rgbaz_image;

	char* filename_out;
	char* filename_rgba;
	char* filename_z;
	char* node_num;
	
	FILE* in_rgba;
	FILE* in_z;
	
	//=====================================
	// Initialize MPI
	//=====================================
	MPI_Init(&argc, &argv);
	MPI_Comm_rank( MPI_COMM_WORLD, &rank   );
	MPI_Comm_size( MPI_COMM_WORLD, &nnodes );
	//=====================================

	if ( argc == 3 ) {
		width      = atoi(argv[1]);
		height     = atoi(argv[2]);
	}
	else { 
		if ( rank == ROOT_NODE ) {
			printf ("\n Usage: %s Width Height \n", argv[0] );
			exit( EXIT_FAILURE );
		}			
	}

	//=====================================
	global_my_rank = rank;
	global_nnodes  = nnodes;

	image_size = width * height;

	rgbaz_image = (BYTE *)allocate_byte_memory_region( image_size * 8 );  // RGBA(4) + Z(4)
	assert(rgba_image); 

	rgba_image = (BYTE *)allocate_byte_memory_region( image_size * 4 );  // RGBA(4) 
	assert(rgba_image); 

	z_image = (float *)allocate_float_memory_region( image_size );  // Z(4)
	assert(rgba_image); 

    	filename_out = (char *)allocate_byte_memory_region( 255 );
 	assert(filename_out);

    	filename_rgba = (char *)allocate_byte_memory_region( 255 );
 	assert(filename_rgba);

    	filename_z = (char *)allocate_byte_memory_region( 255 );
 	assert(filename_z);

    	node_num =  (char *)allocate_byte_memory_region( 255 );
	assert(node_num);

	//=====================================
	// Read Image Files
	//=====================================
	filename_rgba[0] = '\0';		
	filename_z[0]    = '\0';		

	filename_rgba = strcat ( filename_rgba, "./RGBA_Z_1023x1023/RGBA_" );
	filename_z    = strcat ( filename_z,    "./RGBA_Z_1023x1023/Z_" );

	sprintf( node_num, "%d", rank );

	filename_rgba = strcat ( filename_rgba, node_num );
	filename_z    = strcat ( filename_z,    node_num );

	filename_rgba = strcat ( filename_rgba, ".rgba\0" );
	filename_z    = strcat ( filename_z,    ".z\0" );

	if ( (in_rgba = fopen( filename_rgba, "rb")) == NULL ) 
	{
		printf( "<<< ERROR >>> Cannot open \"%s\" for reading \n", filename_rgba );
		exit ( EXIT_FAILURE );
	}

	if ( (in_z = fopen( filename_z, "rb")) == NULL ) 
	{
		printf( "<<< ERROR >>> Cannot open \"%s\" for reading \n", filename_z );
		exit ( EXIT_FAILURE );
	}

	if ( fread( rgba_image, sizeof(BYTE), image_size * RGBA, in_rgba ) != image_size * RGBA ) 
	{
		printf("<<< ERROR >>> Image reading error: %s ( %d x %d ) \n", filename_rgba, width, height );
		exit ( EXIT_FAILURE );
	}

	if ( fread( z_image, sizeof(float), image_size, in_z ) != image_size ) 
	{
		printf("<<< ERROR >>> Image reading error: %s ( %d x %d ) \n", filename_z, width, height );
		exit ( EXIT_FAILURE );
	}

	pixel_ID = ID_RGBA32;

	//=====================================
	// Execute 234Compositor
	//=====================================
	Init_234Composition ( rank, nnodes, width, height, pixel_ID );
	//=====================================

	// For testing the version that uses Memory Copy
    	Do_234ZComposition  ( rank, nnodes, width, height, pixel_ID, ALPHA, rgba_image, z_image, MPI_COMM_WORLD );

	if ( rank == 0 ) {
		sprintf( filename_out, "output_%d_%dx%d.rgba", (int)nnodes, (int)width, (int)height );
		write_rgba_image( filename_out, rgba_image, width, height, RGBA32 );
	}

	//=====================================
	 Destroy_234Composition ( pixel_ID );
	//=====================================

	MPI_Finalize();
	return ( EXIT_SUCCESS );
}

/*===========================================================================*/
/**
 *  @brief Write output RGBA image. 
 *
 *  @param  rgba_filename [in] Filename 
 *  @param  image         [in] Image to be written
 *  @param  width         [in]  Image width
 *  @param  height        [in]  Image height
 *  @param  pixel_size    [in]  Pixel size
 */
/*===========================================================================*/
int write_rgba_image \
		( char*  rgba_filename, \
		  BYTE* image, \
		  unsigned int width, \
		  unsigned int height, \
		  unsigned int pixel_size )
{

	unsigned int image_size;
	FILE* out_fp;
	
	if ( (out_fp = fopen( rgba_filename, "wb")) == NULL ) 
	{
		printf( "<<< ERROR >>> Cannot open \"%s\" for writing \n", \
			    rgba_filename );
		return EXIT_FAILURE;
	}

	image_size = width * height * pixel_size;

	if ( fwrite( image, image_size, 1, out_fp ) != 1 ) 
	{
		printf("<<< ERROR >>> Cannot write image \n" );
		return EXIT_FAILURE;
	}

	fclose( out_fp );
	return EXIT_SUCCESS;
}

