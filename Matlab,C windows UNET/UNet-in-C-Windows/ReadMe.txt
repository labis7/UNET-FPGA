- main.h header files includes information about each data structure(Starting reading this file first)
- Prediction.c : In this file, the U-NET neural network backbone is built using libraries and function from the other files.
- Utilities.c : Includes main function, and some important tools used for creating dynamic space or assisting the functionality of the program.
- File Managment.c : Everything has to do with reading and loading from disk files is there(load images,labels and every weight type)
- Convolutions.c: Convolution, Transposed Convolution, concatenation and 'crop to half' functions are included.
- Pooling: maxpooling function is included plus the backpropagation function that is needed during training phase(currently is not supported)

*More Information about the Neural network architecture and data structures read the detailed Thesis by Labis Skoufis
