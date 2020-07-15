# Compile Test Program

$ mpicc -o test_234Z test_234Z.c -I./../../include ../../lib/lib234comp.a -lm

# Run Test Program

$ mpirun -np 8 test_234Z 1023 1023

#  Convert RAW Image (RGBA) to JPG via ImageMagick (convert)

$ convert -depth 8 -size 1023x1023 output_8_1023x1023.rgba output_8_1023x1023.jpg


