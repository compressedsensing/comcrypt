# Practical Open source DSP implementations for Contiki NG and C

### Get started
In general this project follows the same structure as this [sister repository](https://github.com/compressedsensing/compressed_sensing/tree/master), as it is part of the same project. To get started follow the Prerequisites and Getting started from over there.

### Description
This repository is a side effect from the project "Design and Prototype of Energy Concealment
Encryption using Compressive Sensing for IoT".
[Insert link to paper here when done]

The code consists of a practical implementation of a Discrete Wavelet Transformation (DWT), Discrete Cosine Transformation (DCT), a 4-bit huffman encoding and 128-bit AES encryption in Counter mode. All implementations is written purely in C and have no external dependencies (Except for the block encoding of AES). The code is created as a [Contiki NG](https://github.com/contiki-ng/contiki-ng/wiki) project and is meant for very constrained IoT devices. The code is created with a TelosB as testing device, but can probably be used for other devices aswell. As the the device developed for is a TelosB mote with no floating point unit, the fixed point format is used for all Real number operations.

### Methodology
This repository serves as the custom implementations of the state of art solutions to compression and encryption. The role of this is to use these implementations to compare other compression and encryption techniques to, and based on that conclude if any gains was achieved.


### List of important files
#### configuration.h
Configuration stuff. This is where you specify the signal length on which you want to do the processing. This is specified in the **SIGNAL_LEN**. The official supported sizes of the function implementations are 128, 256 and 512.

A **DEBUG** flag is located there along with the **LOG_LEVEL** flag. For testing purposes, while measuring the energy uses during processing **DEBUG** should be set to **0** and **LOG_LEVEL** should be set to **LOG_LEVEL_NONE**.

#### project-conf.h
Project configurations. Should not be changed. CUrrent configuration is using nullnet and the largest CCA threshold possible for consistent energy measurements.

#### fixedpoint.c and fixedpoint.h
Fixed point operations used for calculations in the transformations.

#### encrypt.c and encrypt.h
Implementation of a counter mode of 128-bit AES.

#### compression.c and compression.h
Implementation of:
- A DWT
- A DCT using a naive approach using the formula for DCT-2 directly
- A DCT using a faster precomputed approach called Fast Cosine Transform (FCT) in the code. This needs one to precompute **SIGNAL_LEN** number of the first basis of a DCT. It can be retrieved from [this python script](https://drive.google.com/drive/folders/1vnTTLqZaBvs1BCDa4sogXyUq3D5xcy4m?usp=sharing) which is also mentioned in the referenced repository from earlier. In the **Test signals** folder see the **Mote_test_data_generator** script and retrieve the DCT constants which fit to the signal length.
- A 4-bit huffman encoding example. To actually use the Huffman encoding method one needs to predetermine the Huffman Codebook. And a Huffman EOF bitstring. This is done by analysing the signals for compression, and creating a histogram over the probability of certain symbols. An example made from the ECG database from the MIT-BIH Arrhythmia Database, for after the DWT transformation can be seen in the following figure:

![Image of the distribution of 4 bit symbols in the DWT transformed data](./figures/dwt_huffman_distribution.png?raw=true "Image of the distribution of 4 bit symbols in the DWT transformed data")

From the above histogram one can construct a Huffman tree and from that fill the codebook. One way to do this is with this python package and script:
```
from dahuffman import HuffmanCodec

codec = HuffmanCodec.from_frequencies({0x0: 0, 0x1:3, 0x2:4, 0x3: 6, 0x4:13, 0x5:18, 0x6:392, 0x7:499783, 0x8:499399, 0x9:426, 0xA:20, 0xB:15, 0xC:8, 0xD:5, 0xE:2, 0xF:1})
codec.print_code_table()
```
This will print out the Huffman tree and EOF symbol which should be used.

An example codebook looks like this:

```
const huffman_codeword huffman_codebook[16] = {
    {0b1, 1}, {0b0000, 4}, {0b00100, 5}, {0b001111, 6},
    {0b001010, 6}, {0b0010111, 7}, {0b0001100, 7}, {0b000110101, 9},
    {0b0001101001, 10}, {0b00011011, 8}, {0b0010110, 7}, {0b000111, 6},
    {0b001110, 6}, {0b00010, 5}, {0b00110, 5}, {0b01, 2}};
```
Looking at huffman_codebook[0], it means that the symbol with the value 0000 shall be encoded with the binary string 0b1, which is of length 1 bit. Looking at huffman_codebook[1] it means that the symbol 0001 shall be encoded by the value 0b0000 which has the length of 4 bits, and so on...



#### comcrypt_end_to_end.c
Program written to test the schemes. The program is structured in roughly the same way as **compressed_sensing_end_to_end.c** in the [sister repository](https://github.com/compressedsensing/compressed_sensing/tree/master). So go there for further thoughts about the structure of the file.

Important deviations is in the top where you need to specify the correct Huffman codebook and Huffman EOF. You also need to specify a 128-bit AES key and IV.

### Verification and testing
To be able to verify the schemes a signal reconstruction script has been made in a python notebook located in colab. In the following folder [https://drive.google.com/drive/folders/1vnTTLqZaBvs1BCDa4sogXyUq3D5xcy4m?usp=sharing](https://drive.google.com/drive/folders/1vnTTLqZaBvs1BCDa4sogXyUq3D5xcy4m?usp=sharing) lies both a reconstruction folder and a test signals folder.

The reconstruction folder contains scripts to reconstruct a compressed signal from its hex format. The following signal can be reconstructed:
1. Signal compressed with first DCT -> Huffman Encoding --> AES called **Comcrypt_DCT_reconstruction**
2. Signal compressed with first DWT -> Huffman Encoding --> AES called **Comcrypt_DWT_reconstruction**
3. Signal encrypted with AES called **Comcrypt_AES_TX**
4. Signal with no processing called **Comcrypt_TX**.

The final signal is the output from the the test program (comcrypt_end_to_end.c) in DEBUG state 1. A compare signal should be provided aswell to be able to see how well the signal was reconstructed and to calculate a PRD.

To be able test the schemes a test signal should be provided in 16-bit format. In the folder mentioned above there is also a folder called **Test signals**. There lies a script called **Mote_test_data_generator** generating a test signal in different block sizes based on a record in the ECG database. This signal can be copied into the test program. This signal can also be copied in as the compare signal for the reconstruction script mentioned above. In the same script the DCT basis c-variable is also generated as mentioned earlier. This should be pasted in the **compression.c**. Where one example of this already lies. Replace that with the one fitting your signal length.

### Testing the schemes
The testing is done in a similar way as in the [sister repository](https://github.com/compressedsensing/compressed_sensing/tree/master).
