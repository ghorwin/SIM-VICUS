/*	Copyright (c) 2001-2017, Institut für Bauklimatik, TU Dresden, Germany

	Written by A. Nicolai, H. Fechner, St. Vogelsang, A. Paepcke, J. Grunewald
	All rights reserved.

	This file is part of the IBK Library.

	Redistribution and use in source and binary forms, with or without modification,
	are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
	ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
	ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


	This library contains derivative work based on other open-source libraries.
	See OTHER_LICENCES and source code headers for details.

*/

#include "IBK_configuration.h"

#include <iterator>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstring>
#include <exception>
#include <iomanip>
#include <cstdlib>

#include "IBK_crypt.h"
#include "IBK_assert.h"
#include "IBK_StringUtils.h"

namespace IBK {

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
					   +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif


uint32_t SuperFastHash ( const std::string & myString ) {

	return SuperFastHash( myString.c_str(), (int)myString.length() );

}


uint32_t SuperFastHash (const char * data, int len) {
	uint32_t hash = len, tmp;
	int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (;len > 0; len--) {
		hash  += get16bits (data);
		tmp    = (get16bits (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
		case 3: hash += get16bits (data);
				hash ^= hash << 16;
				hash ^= data[sizeof (uint16_t)] << 18;
				hash += hash >> 11;
				break;
		case 2: hash += get16bits (data);
				hash ^= hash << 11;
				hash += hash >> 17;
				break;
		case 1: hash += *data;
				hash ^= hash << 10;
				hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}



uint32_t SuperFastHashIncremental (const char * data, int len, uint32_t hash) {
uint32_t tmp;
int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (;len > 0; len--) {
		hash  += get16bits (data);
		tmp    = (get16bits (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
		case 3: hash += get16bits (data);
				hash ^= hash << 16;
				hash ^= data[sizeof (uint16_t)] << 18;
				hash += hash >> 11;
				break;
		case 2: hash += get16bits (data);
				hash ^= hash << 11;
				hash += hash >> 17;
				break;
		case 1: hash += *data;
				hash ^= hash << 10;
				hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}
// -----------------------------------------------------------------------------



//const unsigned char B64[64] = { 65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
//	81,82,83,84,85,86,87,88,89,90,97,98,99,100,101,102,103,104,105,106,107,108,
//	109,110,111,112,113,114,115,116,117,118,119,120,121,122,48,49,50,51,52,53,
//	54,55,56,57,43,47 };

// *** MD5 Code ***

// Class md5_hash handles the creating of MD5 hashes.
class md5_hash {
public:
	md5_hash();

	//RSA MD5 Implementierung
	bool transform(const std::vector<unsigned char>& Block, int& error);
	void update(std::vector<unsigned char>& Input, int& error);
	std::string finalStr(int& iErrorCalculate);
	std::vector<unsigned char> final(int& iErrorCalculate);
	bool final(std::vector<unsigned int>& res, int& error);

private:
	unsigned int RotateLeft(unsigned int x, int n);
	void FF( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T);
	void GG( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T);
	void HH( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T);
	void II( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T);

	//Nebenfunktionen
	bool DWordToByte(std::vector<unsigned char>& Output, const std::vector<unsigned int>& Input, int& error);
	bool ByteToDWord(std::vector<unsigned int>& Output, const std::vector<unsigned char>& Input, int& error);
	void MemoryMove(unsigned char* from, unsigned char* to, unsigned int size);

	std::vector<unsigned char> m_Buffer;   //Eingabepuffer
	std::vector<unsigned int> m_Count;        //Anzahl der bits, modulo 2^64 (lsb zuerst)
	std::vector<unsigned int> m_lMD5;          //MD5 checksum
};

//Initialisierungskonstanten
const unsigned int  MD5_INIT_STATE_0 = 0x67452301;
const unsigned int  MD5_INIT_STATE_1 = 0xefcdab89;
const unsigned int  MD5_INIT_STATE_2 = 0x98badcfe;
const unsigned int  MD5_INIT_STATE_3 = 0x10325476;

//Konstanten für den Transformierungsprozess.
const unsigned int  MD5_S11 =  7;
const unsigned int  MD5_S12 = 12;
const unsigned int  MD5_S13 = 17;
const unsigned int  MD5_S14 = 22;
const unsigned int  MD5_S21 =  5;
const unsigned int  MD5_S22 =  9;
const unsigned int  MD5_S23 = 14;
const unsigned int  MD5_S24 = 20;
const unsigned int  MD5_S31 =  4;
const unsigned int  MD5_S32 = 11;
const unsigned int  MD5_S33 = 16;
const unsigned int  MD5_S34 = 23;
const unsigned int  MD5_S41 =  6;
const unsigned int  MD5_S42 = 10;
const unsigned int  MD5_S43 = 15;
const unsigned int  MD5_S44 = 21;

//Transformierungskonstanten - Runde 1
const unsigned int  MD5_T01 =  0xd76aa478;
const unsigned int  MD5_T02 =  0xe8c7b756;
const unsigned int  MD5_T03 =  0x242070db;
const unsigned int  MD5_T04 =  0xc1bdceee;
const unsigned int  MD5_T05 =  0xf57c0faf;
const unsigned int  MD5_T06 =  0x4787c62a;
const unsigned int  MD5_T07 =  0xa8304613;
const unsigned int  MD5_T08 =  0xfd469501;
const unsigned int  MD5_T09 =  0x698098d8;
const unsigned int  MD5_T10 =  0x8b44f7af;
const unsigned int  MD5_T11 =  0xffff5bb1;
const unsigned int  MD5_T12 =  0x895cd7be;
const unsigned int  MD5_T13 =  0x6b901122;
const unsigned int  MD5_T14 =  0xfd987193;
const unsigned int  MD5_T15 =  0xa679438e;
const unsigned int  MD5_T16 =  0x49b40821;

//Transformierungskonstanten - Runde 2
const unsigned int  MD5_T17 =  0xf61e2562;
const unsigned int  MD5_T18 =  0xc040b340;
const unsigned int  MD5_T19 =  0x265e5a51;
const unsigned int  MD5_T20 =  0xe9b6c7aa;
const unsigned int  MD5_T21 =  0xd62f105d;
const unsigned int  MD5_T22 =  0x02441453;
const unsigned int  MD5_T23 =  0xd8a1e681;
const unsigned int  MD5_T24 =  0xe7d3fbc8;
const unsigned int  MD5_T25 =  0x21e1cde6;
const unsigned int  MD5_T26 =  0xc33707d6;
const unsigned int  MD5_T27 =  0xf4d50d87;
const unsigned int  MD5_T28 =  0x455a14ed;
const unsigned int  MD5_T29 =  0xa9e3e905;
const unsigned int  MD5_T30 =  0xfcefa3f8;
const unsigned int  MD5_T31 =  0x676f02d9;
const unsigned int  MD5_T32 =  0x8d2a4c8a;

//Transformierungskonstanten - Runde 3
const unsigned int  MD5_T33 =  0xfffa3942;
const unsigned int  MD5_T34 =  0x8771f681;
const unsigned int  MD5_T35 =  0x6d9d6122;
const unsigned int  MD5_T36 =  0xfde5380c;
const unsigned int  MD5_T37 =  0xa4beea44;
const unsigned int  MD5_T38 =  0x4bdecfa9;
const unsigned int  MD5_T39 =  0xf6bb4b60;
const unsigned int  MD5_T40 =  0xbebfbc70;
const unsigned int  MD5_T41 =  0x289b7ec6;
const unsigned int  MD5_T42 =  0xeaa127fa;
const unsigned int  MD5_T43 =  0xd4ef3085;
const unsigned int  MD5_T44 =  0x04881d05;
const unsigned int  MD5_T45 =  0xd9d4d039;
const unsigned int  MD5_T46 =  0xe6db99e5;
const unsigned int  MD5_T47 =  0x1fa27cf8;
const unsigned int  MD5_T48 =  0xc4ac5665;

//Transformierungskonstanten - Runde 4
const unsigned int  MD5_T49 =  0xf4292244;
const unsigned int  MD5_T50 =  0x432aff97;
const unsigned int  MD5_T51 =  0xab9423a7;
const unsigned int  MD5_T52 =  0xfc93a039;
const unsigned int  MD5_T53 =  0x655b59c3;
const unsigned int  MD5_T54 =  0x8f0ccc92;
const unsigned int  MD5_T55 =  0xffeff47d;
const unsigned int  MD5_T56 =  0x85845dd1;
const unsigned int  MD5_T57 =  0x6fa87e4f;
const unsigned int  MD5_T58 =  0xfe2ce6e0;
const unsigned int  MD5_T59 =  0xa3014314;
const unsigned int  MD5_T60 =  0x4e0811a1;
const unsigned int  MD5_T61 =  0xf7537e82;
const unsigned int  MD5_T62 =  0xbd3af235;
const unsigned int  MD5_T63 =  0x2ad7d2bb;
const unsigned int  MD5_T64 =  0xeb86d391;

static unsigned char PADDING[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


unsigned int md5_hash::RotateLeft(unsigned int x, int n) {
	//verschieben und x zurückgeben
	return (x << n) | (x >> (32-n));
}

void md5_hash::FF( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T) {
	unsigned int F = (B & C) | (~B & D);
	A += F + X + T;
	A = RotateLeft(A, S);
	A += B;
}

void md5_hash::GG( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T) {
	unsigned int G = (B & D) | (C & ~D);
	A += G + X + T;
	A = RotateLeft(A, S);
	A += B;
}

void md5_hash::HH( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T) {
	unsigned int H = (B ^ C ^ D);
	A += H + X + T;
	A = RotateLeft(A, S);
	A += B;
}

void md5_hash::II( unsigned int& A, unsigned int B, unsigned int C, unsigned int D, unsigned int X, unsigned int S, unsigned int T) {
	unsigned int I = (C ^ (B | ~D));
	A += I + X + T;
	A = RotateLeft(A, S);
	A += B;
}

bool md5_hash::ByteToDWord(std::vector<unsigned int>& Output, const std::vector<unsigned char>& Input, int& error) {
	//entry invariants
	unsigned int nLength(static_cast<unsigned int>(Input.size()));
	if( nLength % 4 != 0 ) {
		error = -24;
		return false;
	}
	//Initialisierung
	unsigned int j=0;       //index des Quellarrays
	//transferiere die Daten durch shifting und kopieren
	Output.clear();
	for ( ; j < nLength; j += 4) {
		Output.push_back(
			(unsigned int)Input[j] | (unsigned int)Input[j+1] << 8  |
			(unsigned int)Input[j+2] << 16 | (unsigned int)Input[j+3] << 24
			);
	}
	return true;
}

bool md5_hash::transform(const std::vector<unsigned char>& Block, int& error) {
	//initialisiere die lokalen Daten mit der aktuellen checksum
	unsigned int a = m_lMD5[0];
	unsigned int b = m_lMD5[1];
	unsigned int c = m_lMD5[2];
	unsigned int d = m_lMD5[3];

	//Kopiert BYTES aus input 'Block' in ein Array von ULONGS 'X'
	std::vector<unsigned int> X;
	if( !ByteToDWord( X, Block, error))
		return false;
	//Runde 1 Transformation
	FF (a, b, c, d, X[ 0], MD5_S11, MD5_T01);
	FF (d, a, b, c, X[ 1], MD5_S12, MD5_T02);
	FF (c, d, a, b, X[ 2], MD5_S13, MD5_T03);
	FF (b, c, d, a, X[ 3], MD5_S14, MD5_T04);
	FF (a, b, c, d, X[ 4], MD5_S11, MD5_T05);
	FF (d, a, b, c, X[ 5], MD5_S12, MD5_T06);
	FF (c, d, a, b, X[ 6], MD5_S13, MD5_T07);
	FF (b, c, d, a, X[ 7], MD5_S14, MD5_T08);
	FF (a, b, c, d, X[ 8], MD5_S11, MD5_T09);
	FF (d, a, b, c, X[ 9], MD5_S12, MD5_T10);
	FF (c, d, a, b, X[10], MD5_S13, MD5_T11);
	FF (b, c, d, a, X[11], MD5_S14, MD5_T12);
	FF (a, b, c, d, X[12], MD5_S11, MD5_T13);
	FF (d, a, b, c, X[13], MD5_S12, MD5_T14);
	FF (c, d, a, b, X[14], MD5_S13, MD5_T15);
	FF (b, c, d, a, X[15], MD5_S14, MD5_T16);

	//Runde 2 Transformation
	GG (a, b, c, d, X[ 1], MD5_S21, MD5_T17);
	GG (d, a, b, c, X[ 6], MD5_S22, MD5_T18);
	GG (c, d, a, b, X[11], MD5_S23, MD5_T19);
	GG (b, c, d, a, X[ 0], MD5_S24, MD5_T20);
	GG (a, b, c, d, X[ 5], MD5_S21, MD5_T21);
	GG (d, a, b, c, X[10], MD5_S22, MD5_T22);
	GG (c, d, a, b, X[15], MD5_S23, MD5_T23);
	GG (b, c, d, a, X[ 4], MD5_S24, MD5_T24);
	GG (a, b, c, d, X[ 9], MD5_S21, MD5_T25);
	GG (d, a, b, c, X[14], MD5_S22, MD5_T26);
	GG (c, d, a, b, X[ 3], MD5_S23, MD5_T27);
	GG (b, c, d, a, X[ 8], MD5_S24, MD5_T28);
	GG (a, b, c, d, X[13], MD5_S21, MD5_T29);
	GG (d, a, b, c, X[ 2], MD5_S22, MD5_T30);
	GG (c, d, a, b, X[ 7], MD5_S23, MD5_T31);
	GG (b, c, d, a, X[12], MD5_S24, MD5_T32);

	//Runde 3 Transformation
	HH (a, b, c, d, X[ 5], MD5_S31, MD5_T33);
	HH (d, a, b, c, X[ 8], MD5_S32, MD5_T34);
	HH (c, d, a, b, X[11], MD5_S33, MD5_T35);
	HH (b, c, d, a, X[14], MD5_S34, MD5_T36);
	HH (a, b, c, d, X[ 1], MD5_S31, MD5_T37);
	HH (d, a, b, c, X[ 4], MD5_S32, MD5_T38);
	HH (c, d, a, b, X[ 7], MD5_S33, MD5_T39);
	HH (b, c, d, a, X[10], MD5_S34, MD5_T40);
	HH (a, b, c, d, X[13], MD5_S31, MD5_T41);
	HH (d, a, b, c, X[ 0], MD5_S32, MD5_T42);
	HH (c, d, a, b, X[ 3], MD5_S33, MD5_T43);
	HH (b, c, d, a, X[ 6], MD5_S34, MD5_T44);
	HH (a, b, c, d, X[ 9], MD5_S31, MD5_T45);
	HH (d, a, b, c, X[12], MD5_S32, MD5_T46);
	HH (c, d, a, b, X[15], MD5_S33, MD5_T47);
	HH (b, c, d, a, X[ 2], MD5_S34, MD5_T48);

	//Runde 4 Transformation
	II (a, b, c, d, X[ 0], MD5_S41, MD5_T49);
	II (d, a, b, c, X[ 7], MD5_S42, MD5_T50);
	II (c, d, a, b, X[14], MD5_S43, MD5_T51);
	II (b, c, d, a, X[ 5], MD5_S44, MD5_T52);
	II (a, b, c, d, X[12], MD5_S41, MD5_T53);
	II (d, a, b, c, X[ 3], MD5_S42, MD5_T54);
	II (c, d, a, b, X[10], MD5_S43, MD5_T55);
	II (b, c, d, a, X[ 1], MD5_S44, MD5_T56);
	II (a, b, c, d, X[ 8], MD5_S41, MD5_T57);
	II (d, a, b, c, X[15], MD5_S42, MD5_T58);
	II (c, d, a, b, X[ 6], MD5_S43, MD5_T59);
	II (b, c, d, a, X[13], MD5_S44, MD5_T60);
	II (a, b, c, d, X[ 4], MD5_S41, MD5_T61);
	II (d, a, b, c, X[11], MD5_S42, MD5_T62);
	II (c, d, a, b, X[ 2], MD5_S43, MD5_T63);
	II (b, c, d, a, X[ 9], MD5_S44, MD5_T64);

	//Füge die veränderten Werte zur aktuellen Checksum hinzu
	m_lMD5[0] += a;
	m_lMD5[1] += b;
	m_lMD5[2] += c;
	m_lMD5[3] += d;
	return true;
}

md5_hash::md5_hash() : m_Buffer(64, (unsigned char)(0)), m_Count(2, int(0)), m_lMD5(4) {
	// Lade magic state Initialisierungskonstanten
	m_lMD5[0] = MD5_INIT_STATE_0;
	m_lMD5[1] = MD5_INIT_STATE_1;
	m_lMD5[2] = MD5_INIT_STATE_2;
	m_lMD5[3] = MD5_INIT_STATE_3;
}

// bool md5_hash::DWordToByte(unsigned char* Output, unsigned int* Input, unsigned int nLength, int& error)
// {
//		  //entry invariants
//		 if( nLength % 4 != 0 )
//		 {
//			error = -22;
//			return false;
//		 }
//		 //transferiere die Daten durch shifting und kopieren
//         unsigned int i = 0;
//		 unsigned int j = 0;
//		 for ( ; j < nLength; i++, j += 4)
//		 {
//				 Output[j] =   (unsigned char)(Input[i] & 0xff);
//				 Output[j+1] = (unsigned char)((Input[i] >> 8) & 0xff);
//				 Output[j+2] = (unsigned char)((Input[i] >> 16) & 0xff);
//				 Output[j+3] = (unsigned char)((Input[i] >> 24) & 0xff);
//		 }
// }

bool md5_hash::DWordToByte(std::vector<unsigned char>& Output, const std::vector<unsigned int>& Input, int& error) {
	//entry invariants
	unsigned int nLength(static_cast<unsigned int>(Output.size()));
	if( nLength % 4 != 0 ) {
		error = -22;
		return false;
	}
	if( Input.size() < nLength / 4 ) {
		error = -23;
		return false;
	}
	//transferiere die Daten durch shifting und kopieren
	unsigned int i = 0;
	for (unsigned int j = 0; j < nLength; i++, j += 4) {
		Output[j] =   (unsigned char)(Input[i] & 0xff);
		Output[j+1] = (unsigned char)((Input[i] >> 8) & 0xff);
		Output[j+2] = (unsigned char)((Input[i] >> 16) & 0xff);
		Output[j+3] = (unsigned char)((Input[i] >> 24) & 0xff);
	}
	return true;
}

std::string md5_hash::finalStr(int& error) {
	//Sichere Anzahl der Bits
	std::vector<unsigned char> Bits(8);
	error = 0;
	if (!DWordToByte( Bits, m_Count, error))
		return "";

	//fill up to 56 mod 64.
	unsigned int nIndex = (unsigned int)((m_Count[0] >> 3) & 0x3f);
	unsigned int nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
	std::vector<unsigned char> padding(PADDING, PADDING + nPadLen);
	update( padding, error );
	//add lenght (before filling)
	update( Bits, error );

	//save final state in 'lpszMD5'
	const int nMD5Size = 16;
	std::vector<unsigned char> MD5(nMD5Size);
	if (!DWordToByte( MD5, m_lMD5, error) )
		return "";

	//convert hexadecimale checksum into string
	std::string strMD5;
	for ( int i=0; i < nMD5Size; i++) {
		std::string Str;
		if (MD5[i] == 0) {
			Str = "00";
		}
		else if (MD5[i] <= 15) {
			Str = "0";
			std::stringstream tstr;
			tstr << std::hex << (unsigned short)MD5[i];
			Str += tstr.str();
		}
		else {
			std::stringstream tstr;
			tstr << std::hex << (unsigned short)MD5[i];
			Str += tstr.str();
		}

		if(Str.size() != 2) {
			error = -20;
			break;
		}
		strMD5 += Str;
	}
	if (strMD5.size() != 32)
		error = -21;
	return strMD5;
}


std::vector<unsigned char> md5_hash::final(int& error) {
	//Sichere Anzahl der Bits
	std::vector<unsigned char> Bits(8);
	std::vector<unsigned char> res;
	error = 0;
	if( !DWordToByte( Bits, m_Count, error))
		return res;
	//Auffüllen bis 56 mod 64.
	unsigned int nIndex = (unsigned int)((m_Count[0] >> 3) & 0x3f);
	unsigned int nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
	std::vector<unsigned char> padding(PADDING, PADDING + nPadLen);
	update( padding, error );
	//Füge die Länge hinzu (vorm Auffüllen)
	update( Bits, error );

	//Sichere final state in 'lpszMD5'
	const int nMD5Size = 16;
	std::vector<unsigned char> MD5(nMD5Size);
	if( !DWordToByte( MD5, m_lMD5, error ))
		return res;
	//Konvertiere die hexadecimale Checksum in einen CString
	res = MD5;
	if(res.size() != 4)
		error = -21;
	return res;
}


bool md5_hash::final(std::vector<unsigned int>& res, int& error) {
	//Sichere Anzahl der Bits
	std::vector<unsigned char> Bits(8);
	error = 0;
	if( !DWordToByte( Bits, m_Count, error))
		return false;
	//fill up to 56 mod 64.
	unsigned int nIndex = (unsigned int)((m_Count[0] >> 3) & 0x3f);
	unsigned int nPadLen = (nIndex < 56) ? (56 - nIndex) : (120 - nIndex);
	std::vector<unsigned char> padding(PADDING, PADDING + nPadLen);
	update( padding, error );
	//add length (before filling)
	update( Bits, error );
	res = m_lMD5;
	return true;
}


void md5_hash::update(std::vector<unsigned char>& Input, int& error) {
	//Anzahl der Bytes mod 64 berechnen
	unsigned int nInputLen(static_cast<unsigned int>(Input.size()));
	unsigned int nIndex = (unsigned int)((m_Count[0] >> 3) & 0x3F);

	//Anzahl der Bits aktualisieren
	if ((m_Count[0] += nInputLen << 3) < (nInputLen << 3)) {
		m_Count[1]++;
	}
	m_Count[1] += (nInputLen >> 29);

	// transform as often as possible
	unsigned int i;
	unsigned int nPartLen = 64 - nIndex;
	if (nInputLen >= nPartLen) {
		//				 std::memmove( &m_Buffer[nIndex], Input, nPartLen );
		std::copy(Input.begin(), Input.begin() + nPartLen, &m_Buffer[nIndex]);
		transform( m_Buffer, error );
		for (i = nPartLen; i + 63 < nInputLen; i += 64)
		{
			const std::vector<unsigned char> temp(Input.begin()+i, Input.begin()+i+64);
			transform( temp, error );
		}
		nIndex = 0;
	}
	else {
		i = 0;
	}

	// remaining input in buffer

	std::copy(Input.begin() + i, Input.begin() + nInputLen, &m_Buffer[nIndex]);
	//		 std::memmove( &m_Buffer[nIndex], &Input[i], nInputLen-i);
}

// *** BLOWFISH Code ***

class BlowFish {
public:
	class BlowFishKeyError : public std::logic_error	{
	public:
		BlowFishKeyError( const std::string& s) : std::logic_error(s) {}
	};

	class BlowFishBufferError : public std::logic_error 	{
	public:
		BlowFishBufferError( const std::string& s) : std::logic_error(s) {}
	};

	enum { ECB=0, CBC=1, CFB=2 };

	//Constructor - Initialize the P and S boxes for a given Key
	BlowFish(const std::vector<unsigned char>& ucKey, unsigned int l_block = 0UL, unsigned int r_block = 0UL);

	//Resetting the chaining block
	void reset_chain() { m_oChain = m_oChain0; }

	// Encrypt/Decrypt Buffer in Place
	void encrypt(std::vector<unsigned char>& ubuf, int iMode=ECB);
	void decrypt(std::vector<unsigned char>& ubuf, int iMode=ECB);

	// Encrypt/Decrypt from Input Buffer to Output Buffer
	void encrypt(const std::vector<unsigned char>& in, std::vector<unsigned char>& out, int iMode=ECB);
	void decrypt(const std::vector<unsigned char>& uin, std::vector<unsigned char>& uout, int iMode=ECB);

	//Private Functions
private:
	struct sblock {
		sblock(unsigned int l=0, unsigned int r=0) : m_uil(l), m_uir(r) {}
		sblock(const sblock& roBlock) : m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) {}
		sblock& operator^=(sblock& b) { m_uil ^= b.m_uil; m_uir ^= b.m_uir; return *this; }
		unsigned int m_uil, m_uir;
		void BytesToBlock(std::vector<unsigned char>::const_iterator p);
		void BlockToBytes(std::vector<unsigned char>::iterator p);
	};

	static unsigned char get_byte(unsigned int ui) {
		return (unsigned char)(ui & 0xff);
	}
	unsigned int funct(unsigned int ui) {
		return ((m_auiS[0][get_byte(ui>>24)] + m_auiS[1][get_byte(ui>>16)]) ^ m_auiS[2][get_byte(ui>>8)]) + m_auiS[3][get_byte(ui)];
	}
	void encrypt(sblock&);
	void decrypt(sblock&);
	//Extract low order byte

private:
	//The Initialization Vector, by default {0, 0}
	sblock m_oChain0;
	sblock m_oChain;
	unsigned int m_auiP[18];
	unsigned int m_auiS[4][256];
	static const unsigned int scm_auiInitP[18];
	static const unsigned int scm_auiInitS[4][256];
	friend struct sblock;
};

//Initialization with a fixed string which consists of the hexadecimal digits of PI (less the initial 3)
//P-array, 18 32-bit subkeys
const unsigned int BlowFish::scm_auiInitP[18] = {
	0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
	0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
	0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
	0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
	0x9216d5d9, 0x8979fb1b
};

//Four 32-bit S-boxes with 256 entries each
const unsigned int BlowFish::scm_auiInitS[4][256] = {
	//0
	{0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7,
	0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99,
	0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16,
	0x636920d8, 0x71574e69, 0xa458fea3, 0xf4933d7e,
	0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee,
	0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013,
	0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef,
	0x8e79dcb0, 0x603a180e, 0x6c9e0e8b, 0xb01e8a3e,
	0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60,
	0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440,
	0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce,
	0xa15486af, 0x7c72e993, 0xb3ee1411, 0x636fbc2a,
	0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e,
	0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677,
	0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193,
	0x61d809cc, 0xfb21a991, 0x487cac60, 0x5dec8032,
	0xef845d5d, 0xe98575b1, 0xdc262302, 0xeb651b88,
	0x23893e81, 0xd396acc5, 0x0f6d6ff3, 0x83f44239,
	0x2e0b4482, 0xa4842004, 0x69c8f04a, 0x9e1f9b5e,
	0x21c66842, 0xf6e96c9a, 0x670c9c61, 0xabd388f0,
	0x6a51a0d2, 0xd8542f68, 0x960fa728, 0xab5133a3,
	0x6eef0b6c, 0x137a3be4, 0xba3bf050, 0x7efb2a98,
	0xa1f1651d, 0x39af0176, 0x66ca593e, 0x82430e88,
	0x8cee8619, 0x456f9fb4, 0x7d84a5c3, 0x3b8b5ebe,
	0xe06f75d8, 0x85c12073, 0x401a449f, 0x56c16aa6,
	0x4ed3aa62, 0x363f7706, 0x1bfedf72, 0x429b023d,
	0x37d0d724, 0xd00a1248, 0xdb0fead3, 0x49f1c09b,
	0x075372c9, 0x80991b7b, 0x25d479d8, 0xf6e8def7,
	0xe3fe501a, 0xb6794c3b, 0x976ce0bd, 0x04c006ba,
	0xc1a94fb6, 0x409f60c4, 0x5e5c9ec2, 0x196a2463,
	0x68fb6faf, 0x3e6c53b5, 0x1339b2eb, 0x3b52ec6f,
	0x6dfc511f, 0x9b30952c, 0xcc814544, 0xaf5ebd09,
	0xbee3d004, 0xde334afd, 0x660f2807, 0x192e4bb3,
	0xc0cba857, 0x45c8740f, 0xd20b5f39, 0xb9d3fbdb,
	0x5579c0bd, 0x1a60320a, 0xd6a100c6, 0x402c7279,
	0x679f25fe, 0xfb1fa3cc, 0x8ea5e9f8, 0xdb3222f8,
	0x3c7516df, 0xfd616b15, 0x2f501ec8, 0xad0552ab,
	0x323db5fa, 0xfd238760, 0x53317b48, 0x3e00df82,
	0x9e5c57bb, 0xca6f8ca0, 0x1a87562e, 0xdf1769db,
	0xd542a8f6, 0x287effc3, 0xac6732c6, 0x8c4f5573,
	0x695b27b0, 0xbbca58c8, 0xe1ffa35d, 0xb8f011a0,
	0x10fa3d98, 0xfd2183b8, 0x4afcb56c, 0x2dd1d35b,
	0x9a53e479, 0xb6f84565, 0xd28e49bc, 0x4bfb9790,
	0xe1ddf2da, 0xa4cb7e33, 0x62fb1341, 0xcee4c6e8,
	0xef20cada, 0x36774c01, 0xd07e9efe, 0x2bf11fb4,
	0x95dbda4d, 0xae909198, 0xeaad8e71, 0x6b93d5a0,
	0xd08ed1d0, 0xafc725e0, 0x8e3c5b2f, 0x8e7594b7,
	0x8ff6e2fb, 0xf2122b64, 0x8888b812, 0x900df01c,
	0x4fad5ea0, 0x688fc31c, 0xd1cff191, 0xb3a8c1ad,
	0x2f2f2218, 0xbe0e1777, 0xea752dfe, 0x8b021fa1,
	0xe5a0cc0f, 0xb56f74e8, 0x18acf3d6, 0xce89e299,
	0xb4a84fe0, 0xfd13e0b7, 0x7cc43b81, 0xd2ada8d9,
	0x165fa266, 0x80957705, 0x93cc7314, 0x211a1477,
	0xe6ad2065, 0x77b5fa86, 0xc75442f5, 0xfb9d35cf,
	0xebcdaf0c, 0x7b3e89a0, 0xd6411bd3, 0xae1e7e49,
	0x00250e2d, 0x2071b35e, 0x226800bb, 0x57b8e0af,
	0x2464369b, 0xf009b91e, 0x5563911d, 0x59dfa6aa,
	0x78c14389, 0xd95a537f, 0x207d5ba2, 0x02e5b9c5,
	0x83260376, 0x6295cfa9, 0x11c81968, 0x4e734a41,
	0xb3472dca, 0x7b14a94a, 0x1b510052, 0x9a532915,
	0xd60f573f, 0xbc9bc6e4, 0x2b60a476, 0x81e67400,
	0x08ba6fb5, 0x571be91f, 0xf296ec6b, 0x2a0dd915,
	0xb6636521, 0xe7b9f9b6, 0xff34052e, 0xc5855664,
	0x53b02d5d, 0xa99f8fa1, 0x08ba4799, 0x6e85076a},

	//1
	{0x4b7a70e9, 0xb5b32944, 0xdb75092e, 0xc4192623,
	0xad6ea6b0, 0x49a7df7d, 0x9cee60b8, 0x8fedb266,
	0xecaa8c71, 0x699a17ff, 0x5664526c, 0xc2b19ee1,
	0x193602a5, 0x75094c29, 0xa0591340, 0xe4183a3e,
	0x3f54989a, 0x5b429d65, 0x6b8fe4d6, 0x99f73fd6,
	0xa1d29c07, 0xefe830f5, 0x4d2d38e6, 0xf0255dc1,
	0x4cdd2086, 0x8470eb26, 0x6382e9c6, 0x021ecc5e,
	0x09686b3f, 0x3ebaefc9, 0x3c971814, 0x6b6a70a1,
	0x687f3584, 0x52a0e286, 0xb79c5305, 0xaa500737,
	0x3e07841c, 0x7fdeae5c, 0x8e7d44ec, 0x5716f2b8,
	0xb03ada37, 0xf0500c0d, 0xf01c1f04, 0x0200b3ff,
	0xae0cf51a, 0x3cb574b2, 0x25837a58, 0xdc0921bd,
	0xd19113f9, 0x7ca92ff6, 0x94324773, 0x22f54701,
	0x3ae5e581, 0x37c2dadc, 0xc8b57634, 0x9af3dda7,
	0xa9446146, 0x0fd0030e, 0xecc8c73e, 0xa4751e41,
	0xe238cd99, 0x3bea0e2f, 0x3280bba1, 0x183eb331,
	0x4e548b38, 0x4f6db908, 0x6f420d03, 0xf60a04bf,
	0x2cb81290, 0x24977c79, 0x5679b072, 0xbcaf89af,
	0xde9a771f, 0xd9930810, 0xb38bae12, 0xdccf3f2e,
	0x5512721f, 0x2e6b7124, 0x501adde6, 0x9f84cd87,
	0x7a584718, 0x7408da17, 0xbc9f9abc, 0xe94b7d8c,
	0xec7aec3a, 0xdb851dfa, 0x63094366, 0xc464c3d2,
	0xef1c1847, 0x3215d908, 0xdd433b37, 0x24c2ba16,
	0x12a14d43, 0x2a65c451, 0x50940002, 0x133ae4dd,
	0x71dff89e, 0x10314e55, 0x81ac77d6, 0x5f11199b,
	0x043556f1, 0xd7a3c76b, 0x3c11183b, 0x5924a509,
	0xf28fe6ed, 0x97f1fbfa, 0x9ebabf2c, 0x1e153c6e,
	0x86e34570, 0xeae96fb1, 0x860e5e0a, 0x5a3e2ab3,
	0x771fe71c, 0x4e3d06fa, 0x2965dcb9, 0x99e71d0f,
	0x803e89d6, 0x5266c825, 0x2e4cc978, 0x9c10b36a,
	0xc6150eba, 0x94e2ea78, 0xa5fc3c53, 0x1e0a2df4,
	0xf2f74ea7, 0x361d2b3d, 0x1939260f, 0x19c27960,
	0x5223a708, 0xf71312b6, 0xebadfe6e, 0xeac31f66,
	0xe3bc4595, 0xa67bc883, 0xb17f37d1, 0x018cff28,
	0xc332ddef, 0xbe6c5aa5, 0x65582185, 0x68ab9802,
	0xeecea50f, 0xdb2f953b, 0x2aef7dad, 0x5b6e2f84,
	0x1521b628, 0x29076170, 0xecdd4775, 0x619f1510,
	0x13cca830, 0xeb61bd96, 0x0334fe1e, 0xaa0363cf,
	0xb5735c90, 0x4c70a239, 0xd59e9e0b, 0xcbaade14,
	0xeecc86bc, 0x60622ca7, 0x9cab5cab, 0xb2f3846e,
	0x648b1eaf, 0x19bdf0ca, 0xa02369b9, 0x655abb50,
	0x40685a32, 0x3c2ab4b3, 0x319ee9d5, 0xc021b8f7,
	0x9b540b19, 0x875fa099, 0x95f7997e, 0x623d7da8,
	0xf837889a, 0x97e32d77, 0x11ed935f, 0x16681281,
	0x0e358829, 0xc7e61fd6, 0x96dedfa1, 0x7858ba99,
	0x57f584a5, 0x1b227263, 0x9b83c3ff, 0x1ac24696,
	0xcdb30aeb, 0x532e3054, 0x8fd948e4, 0x6dbc3128,
	0x58ebf2ef, 0x34c6ffea, 0xfe28ed61, 0xee7c3c73,
	0x5d4a14d9, 0xe864b7e3, 0x42105d14, 0x203e13e0,
	0x45eee2b6, 0xa3aaabea, 0xdb6c4f15, 0xfacb4fd0,
	0xc742f442, 0xef6abbb5, 0x654f3b1d, 0x41cd2105,
	0xd81e799e, 0x86854dc7, 0xe44b476a, 0x3d816250,
	0xcf62a1f2, 0x5b8d2646, 0xfc8883a0, 0xc1c7b6a3,
	0x7f1524c3, 0x69cb7492, 0x47848a0b, 0x5692b285,
	0x095bbf00, 0xad19489d, 0x1462b174, 0x23820e00,
	0x58428d2a, 0x0c55f5ea, 0x1dadf43e, 0x233f7061,
	0x3372f092, 0x8d937e41, 0xd65fecf1, 0x6c223bdb,
	0x7cde3759, 0xcbee7460, 0x4085f2a7, 0xce77326e,
	0xa6078084, 0x19f8509e, 0xe8efd855, 0x61d99735,
	0xa969a7aa, 0xc50c06c2, 0x5a04abfc, 0x800bcadc,
	0x9e447a2e, 0xc3453484, 0xfdd56705, 0x0e1e9ec9,
	0xdb73dbd3, 0x105588cd, 0x675fda79, 0xe3674340,
	0xc5c43465, 0x713e38d8, 0x3d28f89e, 0xf16dff20,
	0x153e21e7, 0x8fb03d4a, 0xe6e39f2b, 0xdb83adf7},

	//2
	{0xe93d5a68, 0x948140f7, 0xf64c261c, 0x94692934,
	0x411520f7, 0x7602d4f7, 0xbcf46b2e, 0xd4a20068,
	0xd4082471, 0x3320f46a, 0x43b7d4b7, 0x500061af,
	0x1e39f62e, 0x97244546, 0x14214f74, 0xbf8b8840,
	0x4d95fc1d, 0x96b591af, 0x70f4ddd3, 0x66a02f45,
	0xbfbc09ec, 0x03bd9785, 0x7fac6dd0, 0x31cb8504,
	0x96eb27b3, 0x55fd3941, 0xda2547e6, 0xabca0a9a,
	0x28507825, 0x530429f4, 0x0a2c86da, 0xe9b66dfb,
	0x68dc1462, 0xd7486900, 0x680ec0a4, 0x27a18dee,
	0x4f3ffea2, 0xe887ad8c, 0xb58ce006, 0x7af4d6b6,
	0xaace1e7c, 0xd3375fec, 0xce78a399, 0x406b2a42,
	0x20fe9e35, 0xd9f385b9, 0xee39d7ab, 0x3b124e8b,
	0x1dc9faf7, 0x4b6d1856, 0x26a36631, 0xeae397b2,
	0x3a6efa74, 0xdd5b4332, 0x6841e7f7, 0xca7820fb,
	0xfb0af54e, 0xd8feb397, 0x454056ac, 0xba489527,
	0x55533a3a, 0x20838d87, 0xfe6ba9b7, 0xd096954b,
	0x55a867bc, 0xa1159a58, 0xcca92963, 0x99e1db33,
	0xa62a4a56, 0x3f3125f9, 0x5ef47e1c, 0x9029317c,
	0xfdf8e802, 0x04272f70, 0x80bb155c, 0x05282ce3,
	0x95c11548, 0xe4c66d22, 0x48c1133f, 0xc70f86dc,
	0x07f9c9ee, 0x41041f0f, 0x404779a4, 0x5d886e17,
	0x325f51eb, 0xd59bc0d1, 0xf2bcc18f, 0x41113564,
	0x257b7834, 0x602a9c60, 0xdff8e8a3, 0x1f636c1b,
	0x0e12b4c2, 0x02e1329e, 0xaf664fd1, 0xcad18115,
	0x6b2395e0, 0x333e92e1, 0x3b240b62, 0xeebeb922,
	0x85b2a20e, 0xe6ba0d99, 0xde720c8c, 0x2da2f728,
	0xd0127845, 0x95b794fd, 0x647d0862, 0xe7ccf5f0,
	0x5449a36f, 0x877d48fa, 0xc39dfd27, 0xf33e8d1e,
	0x0a476341, 0x992eff74, 0x3a6f6eab, 0xf4f8fd37,
	0xa812dc60, 0xa1ebddf8, 0x991be14c, 0xdb6e6b0d,
	0xc67b5510, 0x6d672c37, 0x2765d43b, 0xdcd0e804,
	0xf1290dc7, 0xcc00ffa3, 0xb5390f92, 0x690fed0b,
	0x667b9ffb, 0xcedb7d9c, 0xa091cf0b, 0xd9155ea3,
	0xbb132f88, 0x515bad24, 0x7b9479bf, 0x763bd6eb,
	0x37392eb3, 0xcc115979, 0x8026e297, 0xf42e312d,
	0x6842ada7, 0xc66a2b3b, 0x12754ccc, 0x782ef11c,
	0x6a124237, 0xb79251e7, 0x06a1bbe6, 0x4bfb6350,
	0x1a6b1018, 0x11caedfa, 0x3d25bdd8, 0xe2e1c3c9,
	0x44421659, 0x0a121386, 0xd90cec6e, 0xd5abea2a,
	0x64af674e, 0xda86a85f, 0xbebfe988, 0x64e4c3fe,
	0x9dbc8057, 0xf0f7c086, 0x60787bf8, 0x6003604d,
	0xd1fd8346, 0xf6381fb0, 0x7745ae04, 0xd736fccc,
	0x83426b33, 0xf01eab71, 0xb0804187, 0x3c005e5f,
	0x77a057be, 0xbde8ae24, 0x55464299, 0xbf582e61,
	0x4e58f48f, 0xf2ddfda2, 0xf474ef38, 0x8789bdc2,
	0x5366f9c3, 0xc8b38e74, 0xb475f255, 0x46fcd9b9,
	0x7aeb2661, 0x8b1ddf84, 0x846a0e79, 0x915f95e2,
	0x466e598e, 0x20b45770, 0x8cd55591, 0xc902de4c,
	0xb90bace1, 0xbb8205d0, 0x11a86248, 0x7574a99e,
	0xb77f19b6, 0xe0a9dc09, 0x662d09a1, 0xc4324633,
	0xe85a1f02, 0x09f0be8c, 0x4a99a025, 0x1d6efe10,
	0x1ab93d1d, 0x0ba5a4df, 0xa186f20f, 0x2868f169,
	0xdcb7da83, 0x573906fe, 0xa1e2ce9b, 0x4fcd7f52,
	0x50115e01, 0xa70683fa, 0xa002b5c4, 0x0de6d027,
	0x9af88c27, 0x773f8641, 0xc3604c06, 0x61a806b5,
	0xf0177a28, 0xc0f586e0, 0x006058aa, 0x30dc7d62,
	0x11e69ed7, 0x2338ea63, 0x53c2dd94, 0xc2c21634,
	0xbbcbee56, 0x90bcb6de, 0xebfc7da1, 0xce591d76,
	0x6f05e409, 0x4b7c0188, 0x39720a3d, 0x7c927c24,
	0x86e3725f, 0x724d9db9, 0x1ac15bb4, 0xd39eb8fc,
	0xed545578, 0x08fca5b5, 0xd83d7cd3, 0x4dad0fc4,
	0x1e50ef5e, 0xb161e6f8, 0xa28514d9, 0x6c51133c,
	0x6fd5c7e7, 0x56e14ec4, 0x362abfce, 0xddc6c837,
	0xd79a3234, 0x92638212, 0x670efa8e, 0x406000e0},

	//3
	{0x3a39ce37, 0xd3faf5cf, 0xabc27737, 0x5ac52d1b,
	0x5cb0679e, 0x4fa33742, 0xd3822740, 0x99bc9bbe,
	0xd5118e9d, 0xbf0f7315, 0xd62d1c7e, 0xc700c47b,
	0xb78c1b6b, 0x21a19045, 0xb26eb1be, 0x6a366eb4,
	0x5748ab2f, 0xbc946e79, 0xc6a376d2, 0x6549c2c8,
	0x530ff8ee, 0x468dde7d, 0xd5730a1d, 0x4cd04dc6,
	0x2939bbdb, 0xa9ba4650, 0xac9526e8, 0xbe5ee304,
	0xa1fad5f0, 0x6a2d519a, 0x63ef8ce2, 0x9a86ee22,
	0xc089c2b8, 0x43242ef6, 0xa51e03aa, 0x9cf2d0a4,
	0x83c061ba, 0x9be96a4d, 0x8fe51550, 0xba645bd6,
	0x2826a2f9, 0xa73a3ae1, 0x4ba99586, 0xef5562e9,
	0xc72fefd3, 0xf752f7da, 0x3f046f69, 0x77fa0a59,
	0x80e4a915, 0x87b08601, 0x9b09e6ad, 0x3b3ee593,
	0xe990fd5a, 0x9e34d797, 0x2cf0b7d9, 0x022b8b51,
	0x96d5ac3a, 0x017da67d, 0xd1cf3ed6, 0x7c7d2d28,
	0x1f9f25cf, 0xadf2b89b, 0x5ad6b472, 0x5a88f54c,
	0xe029ac71, 0xe019a5e6, 0x47b0acfd, 0xed93fa9b,
	0xe8d3c48d, 0x283b57cc, 0xf8d56629, 0x79132e28,
	0x785f0191, 0xed756055, 0xf7960e44, 0xe3d35e8c,
	0x15056dd4, 0x88f46dba, 0x03a16125, 0x0564f0bd,
	0xc3eb9e15, 0x3c9057a2, 0x97271aec, 0xa93a072a,
	0x1b3f6d9b, 0x1e6321f5, 0xf59c66fb, 0x26dcf319,
	0x7533d928, 0xb155fdf5, 0x03563482, 0x8aba3cbb,
	0x28517711, 0xc20ad9f8, 0xabcc5167, 0xccad925f,
	0x4de81751, 0x3830dc8e, 0x379d5862, 0x9320f991,
	0xea7a90c2, 0xfb3e7bce, 0x5121ce64, 0x774fbe32,
	0xa8b6e37e, 0xc3293d46, 0x48de5369, 0x6413e680,
	0xa2ae0810, 0xdd6db224, 0x69852dfd, 0x09072166,
	0xb39a460a, 0x6445c0dd, 0x586cdecf, 0x1c20c8ae,
	0x5bbef7dd, 0x1b588d40, 0xccd2017f, 0x6bb4e3bb,
	0xdda26a7e, 0x3a59ff45, 0x3e350a44, 0xbcb4cdd5,
	0x72eacea8, 0xfa6484bb, 0x8d6612ae, 0xbf3c6f47,
	0xd29be463, 0x542f5d9e, 0xaec2771b, 0xf64e6370,
	0x740e0d8d, 0xe75b1357, 0xf8721671, 0xaf537d5d,
	0x4040cb08, 0x4eb4e2cc, 0x34d2466a, 0x0115af84,
	0xe1b00428, 0x95983a1d, 0x06b89fb4, 0xce6ea048,
	0x6f3f3b82, 0x3520ab82, 0x011a1d4b, 0x277227f8,
	0x611560b1, 0xe7933fdc, 0xbb3a792b, 0x344525bd,
	0xa08839e1, 0x51ce794b, 0x2f32c9b7, 0xa01fbac9,
	0xe01cc87e, 0xbcc7d1f6, 0xcf0111c3, 0xa1e8aac7,
	0x1a908749, 0xd44fbd9a, 0xd0dadecb, 0xd50ada38,
	0x0339c32a, 0xc6913667, 0x8df9317c, 0xe0b12b4f,
	0xf79e59b7, 0x43f5bb3a, 0xf2d519ff, 0x27d9459c,
	0xbf97222c, 0x15e6fc2a, 0x0f91fc71, 0x9b941525,
	0xfae59361, 0xceb69ceb, 0xc2a86459, 0x12baa8d1,
	0xb6c1075e, 0xe3056a0c, 0x10d25065, 0xcb03a442,
	0xe0ec6e0e, 0x1698db3b, 0x4c98a0be, 0x3278e964,
	0x9f1f9532, 0xe0d392df, 0xd3a0342b, 0x8971f21e,
	0x1b0a7441, 0x4ba3348c, 0xc5be7120, 0xc37632d8,
	0xdf359f8d, 0x9b992f2e, 0xe60b6f47, 0x0fe3f11d,
	0xe54cda54, 0x1edad891, 0xce6279cf, 0xcd3e7e6f,
	0x1618b166, 0xfd2c1d05, 0x848fd2c5, 0xf6fb2299,
	0xf523f357, 0xa6327623, 0x93a83531, 0x56cccd02,
	0xacf08162, 0x5a75ebb5, 0x6e163697, 0x88d273cc,
	0xde966292, 0x81b949d0, 0x4c50901b, 0x71c65614,
	0xe6c6c7bd, 0x327a140a, 0x45e1d006, 0xc3f27b9a,
	0xc9aa53fd, 0x62a80f00, 0xbb25bfe2, 0x35bdd2f6,
	0x71126905, 0xb2040222, 0xb6cbcf7c, 0xcd769c2b,
	0x53113ec0, 0x1640e3d3, 0x38abbd60, 0x2547adf0,
	0xba38209c, 0xf746ce76, 0x77afa1c5, 0x20756060,
	0x85cbfe4e, 0x8ae88dd8, 0x7aaaf9b0, 0x4cf9aa7e,
	0x1948c25c, 0x02fb8a8c, 0x01c36ae4, 0xd6ebe1f9,
	0x90d4f869, 0xa65cdea0, 0x3f09252d, 0xc208e69f,
	0xb74e6132, 0xce77e25b, 0x578fdfe3, 0x3ac372e6}
};


BlowFish::BlowFish(const std::vector<unsigned char>& ucKey, unsigned int l_block, unsigned int r_block) : m_oChain0(sblock(l_block,r_block)), m_oChain(sblock(l_block,r_block))
{
	size_t keysize(ucKey.size());
	if(keysize<1)
		throw BlowFishKeyError("Incorrect key length");
	//Check the Key - the key length should be between 1 and 56 bytes
	if(keysize>56)
		keysize = 56;
	std::vector<unsigned char> aucLocalKey(56);
	std::copy(ucKey.begin(), ucKey.begin()+keysize, aucLocalKey.begin());
	//Reflexive Initialization of the BlowFish.
	//Generating the Subkeys from the Key flood P and S boxes with PI
	std::memcpy(m_auiP, scm_auiInitP, sizeof m_auiP);
	std::memcpy(m_auiS, scm_auiInitS, sizeof m_auiS);
	//Load P boxes with key bytes
	std::vector<unsigned char>::iterator p = aucLocalKey.begin();
	//Repeatedly cycle through the key bits until the entire P array has been XORed with key bits
	size_t iCount = 0;
	for(size_t i=0; i<18; i++)
	{
		unsigned int x(0);
		for(int n=4; n--; )
		{
			x <<= 8;
			x |= *(p++);
			iCount++;
			if(iCount == keysize)
			{
				//All bytes used, so recycle bytes
				iCount = 0;
				p = aucLocalKey.begin();
			}
		}
		m_auiP[i] ^= x;
	}
	//Reflect P and S boxes through the evolving BlowFish
	sblock block(0UL,0UL); //all-zero block
	for(size_t i=0; i<18; )
		encrypt(block), m_auiP[i++] = block.m_uil, m_auiP[i++] = block.m_uir;
	for(size_t j=0; j<4; j++)
		for(int k=0; k<256; )
			encrypt(block), m_auiS[j][k++] = block.m_uil, m_auiS[j][k++] = block.m_uir;
}


//Sixteen Round Encipher of Block
void BlowFish::encrypt(sblock& block)
{
	unsigned int uiLeft = block.m_uil;
	unsigned int uiRight = block.m_uir;
	uiLeft ^= m_auiP[0];
	uiRight ^= funct(uiLeft)^m_auiP[1]; uiLeft ^= funct(uiRight)^m_auiP[2];
	uiRight ^= funct(uiLeft)^m_auiP[3]; uiLeft ^= funct(uiRight)^m_auiP[4];
	uiRight ^= funct(uiLeft)^m_auiP[5]; uiLeft ^= funct(uiRight)^m_auiP[6];
	uiRight ^= funct(uiLeft)^m_auiP[7]; uiLeft ^= funct(uiRight)^m_auiP[8];
	uiRight ^= funct(uiLeft)^m_auiP[9]; uiLeft ^= funct(uiRight)^m_auiP[10];
	uiRight ^= funct(uiLeft)^m_auiP[11]; uiLeft ^= funct(uiRight)^m_auiP[12];
	uiRight ^= funct(uiLeft)^m_auiP[13]; uiLeft ^= funct(uiRight)^m_auiP[14];
	uiRight ^= funct(uiLeft)^m_auiP[15]; uiLeft ^= funct(uiRight)^m_auiP[16];
	uiRight ^= m_auiP[17];
	block.m_uil = uiRight;
	block.m_uir = uiLeft;
}


//Sixteen Round Decipher of sblock
void BlowFish::decrypt(sblock& block)
{
	unsigned int uiLeft = block.m_uil;
	unsigned int uiRight = block.m_uir;
	uiLeft ^= m_auiP[17];
	uiRight ^= funct(uiLeft)^m_auiP[16]; uiLeft ^= funct(uiRight)^m_auiP[15];
	uiRight ^= funct(uiLeft)^m_auiP[14]; uiLeft ^= funct(uiRight)^m_auiP[13];
	uiRight ^= funct(uiLeft)^m_auiP[12]; uiLeft ^= funct(uiRight)^m_auiP[11];
	uiRight ^= funct(uiLeft)^m_auiP[10]; uiLeft ^= funct(uiRight)^m_auiP[9];
	uiRight ^= funct(uiLeft)^m_auiP[8]; uiLeft ^= funct(uiRight)^m_auiP[7];
	uiRight ^= funct(uiLeft)^m_auiP[6]; uiLeft ^= funct(uiRight)^m_auiP[5];
	uiRight ^= funct(uiLeft)^m_auiP[4]; uiLeft ^= funct(uiRight)^m_auiP[3];
	uiRight ^= funct(uiLeft)^m_auiP[2]; uiLeft ^= funct(uiRight)^m_auiP[1];
	uiRight ^= m_auiP[0];
	block.m_uil = uiRight;
	block.m_uir = uiLeft;
}


//Semi-Portable Byte Shuffling
void BlowFish::sblock::BytesToBlock(std::vector<unsigned char>::const_iterator p)
{
	unsigned int y;
	//Left
	m_uil = 0;
	y = *p++;
	y <<= 24;
	m_uil |= y;
	y = *p++;
	y <<= 16;
	m_uil |= y;
	y = *p++;
	y <<= 8;
	m_uil |= y;
	y = *p++;
	m_uil |= y;
	//Right
	m_uir = 0;
	y = *p++;
	y <<= 24;
	m_uir |= y;
	y = *p++;
	y <<= 16;
	m_uir |= y;
	y = *p++;
	y <<= 8;
	m_uir |= y;
	y = *p++;
	m_uir |= y;
}


void BlowFish::sblock::BlockToBytes(std::vector<unsigned char>::iterator p)
{
	unsigned int y;
	//Right
	y = m_uir;
	*--p = get_byte(y);
	y = m_uir >> 8;
	*--p = get_byte(y);
	y = m_uir >> 16;
	*--p = get_byte(y);
	y = m_uir >> 24;
	*--p = get_byte(y);
	//Left
	y = m_uil;
	*--p = get_byte(y);
	y = m_uil >> 8;
	*--p = get_byte(y);
	y = m_uil >> 16;
	*--p = get_byte(y);
	y = m_uil >> 24;
	*--p = get_byte(y);
}


//Encrypt Buffer in Place
//Returns false if n is multiple of 8
void BlowFish::encrypt(std::vector<unsigned char>& buf, int iMode)
{
	size_t n(buf.size());
	size_t m8  = n % 8 != 0 ? 8 - n % 8 : 0;
	for( size_t i=0; i<m8; ++i)
		buf.push_back(' ');
	n = buf.size();
	//Check the buffer's length - should be > 0 and multiple of 8
	if((n==0)||(n%8!=0))
		throw BlowFishBufferError("Incorrect buffer length");
	sblock work;
	std::vector<unsigned char>::iterator buf_iter = buf.begin();
	if(iMode == CBC) //CBC mode, using the Chain
	{
		sblock chain(m_oChain);
		for(; n >= 8; n -= 8)
		{
			work.BytesToBlock(buf_iter);
			work ^= chain;
			encrypt(work);
			chain = work;
			work.BlockToBytes(buf_iter+=8);
		}
	}
	else if(iMode == CFB) //CFB mode, using the Chain
	{
		sblock chain(m_oChain);
		for(; n >= 8; n -= 8)
		{
			encrypt(chain);
			work.BytesToBlock(buf_iter);
			work ^= chain;
			chain = work;
			work.BlockToBytes(buf_iter+=8);
		}
	}
	else //ECB mode, not using the Chain
	{
		for(; n >= 8; n -= 8)
		{
			work.BytesToBlock(buf_iter);
			encrypt(work);
			work.BlockToBytes(buf_iter+=8);
		}
	}
}


//Decrypt Buffer in Place
//Returns false if n is multiple of 8
void BlowFish::decrypt(std::vector<unsigned char>& buf, int iMode)
{
	size_t n(buf.size());
	//Check the buffer's length - should be > 0 and multiple of 8
	if((n==0)||(n%8!=0))
		throw BlowFishBufferError("Incorrect buffer length");
	sblock work;
	std::vector<unsigned char>::iterator buf_iter = buf.begin();
	if(iMode == CBC) //CBC mode, using the Chain
	{
		sblock crypt, chain(m_oChain);
		for(; n >= 8; n -= 8)
		{
			work.BytesToBlock(buf_iter);
			crypt = work;
			decrypt(work);
			work ^= chain;
			chain = crypt;
			work.BlockToBytes(buf_iter+=8);
		}
	}
	else if(iMode == CFB) //CFB mode, using the Chain, not using Decrypt()
	{
		sblock crypt, chain(m_oChain);
		for(; n >= 8; n -= 8)
		{
			work.BytesToBlock(buf_iter);
			encrypt(chain);
			crypt = work;
			work ^= chain;
			chain = crypt;
			work.BlockToBytes(buf_iter+=8);
		}
	}
	else //ECB mode, not using the Chain
	{
		for(; n >= 8; n -= 8)
		{
			work.BytesToBlock(buf_iter);
			decrypt(work);
			work.BlockToBytes(buf_iter+=8);
		}
	}
}


//Encrypt from Input Buffer to Output Buffer
void BlowFish::encrypt(const std::vector<unsigned char>& in, std::vector<unsigned char>& out, int iMode)
{
	size_t n(in.size());
	std::vector<unsigned char> tin(in);
	//Check the buffer's length - should be > 0 and multiple of 8
	size_t m8  = n % 8 != 0 ? 8 - n % 8 : 0;
	for( size_t i=0; i<m8; ++i)
		tin.push_back(' ');
	n = tin.size();
	if((n==0)||(n%8!=0))
		throw BlowFishBufferError("Incorrect buffer length");
	sblock work;
	std::vector<unsigned char>::const_iterator in_iter = tin.begin();
	{
		std::vector<unsigned char>(n).swap(out);
	}
	std::vector<unsigned char>::iterator out_iter = out.begin();
	if(iMode == CBC) //CBC mode, using the Chain
	{
		sblock chain(m_oChain);
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			work.BytesToBlock(in_iter);
			work ^= chain;
			encrypt(work);
			chain = work;
			work.BlockToBytes(out_iter+=8);
		}
	}
	else if(iMode == CFB) //CFB mode, using the Chain
	{
		sblock chain(m_oChain);
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			encrypt(chain);
			work.BytesToBlock(in_iter);
			work ^= chain;
			chain = work;
			work.BlockToBytes(out_iter+=8);
		}
	}
	else //ECB mode, not using the Chain
	{
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			work.BytesToBlock(in_iter);
			encrypt(work);
			work.BlockToBytes(out_iter+=8);
		}
	}
}


//Decrypt from Input Buffer to Output Buffer
//Returns false if n is not a multiple of 8
void BlowFish::decrypt(const std::vector<unsigned char>& in, std::vector<unsigned char>& out, int iMode)
{
	size_t n(in.size());
	//Check the buffer's length - should be > 0 and multiple of 8
	if((n==0)||(n%8!=0)) {
		throw BlowFishBufferError("Incorrect buffer length");
	}
	sblock work;
	std::vector<unsigned char>::const_iterator in_iter = in.begin();
	{
		std::vector<unsigned char>(n).swap(out);
	}
	std::vector<unsigned char>::iterator out_iter = out.begin();
	if(iMode == CBC) //CBC mode, using the Chain
	{
		sblock crypt, chain(m_oChain);
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			work.BytesToBlock(in_iter);
			crypt = work;
			decrypt(work);
			work ^= chain;
			chain = crypt;
			work.BlockToBytes(out_iter+=8);
		}
	}
	else if(iMode == CFB) //CFB mode, using the Chain, not using Decrypt()
	{
		sblock crypt, chain(m_oChain);
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			work.BytesToBlock(in_iter);
			encrypt(chain);
			crypt = work;
			work ^= chain;
			chain = crypt;
			work.BlockToBytes(out_iter+=8);
		}
	}
	else //ECB mode, not using the Chain
	{
		for(; n >= 8; n -= 8, in_iter += 8)
		{
			work.BytesToBlock(in_iter);
			decrypt(work);
			work.BlockToBytes(out_iter+=8);
		}
	}
}

// ** Function interfaces **
static const std::string base64_chars =
			 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			 "abcdefghijklmnopqrstuvwxyz"
			 "0123456789+/";

bool base64_encode(const std::vector<unsigned char>& input, std::vector<unsigned char>& output) {

	int i = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	int inputLength = (int)input.size();
	const unsigned char* bytesToEncode = &input[0];
	while (inputLength--) {
		char_array_3[i++] = *(bytesToEncode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				output.push_back(base64_chars[char_array_4[i]]);
			i = 0;
		}
	}

	if (i) {
		for(int j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (int j = 0; (j < i + 1); j++)
			output.push_back(base64_chars[char_array_4[j]]);

		while((i++ < 3))
			output.push_back('=');

	}
	return true;
}
/*
bool base64_encode(const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output) {
	if( Input.empty())
		return false;

	Output.clear();
	size_t size(Input.size());
	if( size % 4 != 0 )
		return false;

	size_t index;
	for( size_t i=0; i<size; i+=3)
	{
		index = Input[i] >> 2;
		Output.push_back(B64[index]);
		index = (Input[i] & 3) << 4;
		index += Input[i+1] >> 4;
		Output.push_back(B64[index]);
		index = (Input[i+1] & 15) << 2;
		index += Input[i+2] >> 6;
		Output.push_back(B64[index]);
		index = Input[i+2] & 63;
		Output.push_back(B64[index]);
	}
	switch(size % 3)
	{
	case 1:
		{
			index = Input[size-2] >> 2;
			Output.push_back(B64[index]);
			index = (Input[size-2] & 3) << 4;
			Output.push_back(B64[index]);
			Output.push_back((unsigned char)('='));
			Output.push_back((unsigned char)('='));
			break;
		}
	case 2:
		{
			index = Input[size-2] >> 2;
			Output.push_back(B64[index]);
			index = (Input[size-2] & 3) << 4;
			index += Input[size-1] >> 4;
			Output.push_back(B64[index]);
			index = (Input[size-1] & 15) << 2;
			Output.push_back(B64[index]);
			Output.push_back((unsigned char)('='));
			break;
		}
	}
	return true;
}
*/

std::string base64_encodeStr(const std::string& Value) {
	std::vector<unsigned char> itemp, otemp;
	std::copy(Value.begin(), Value.end(), std::back_inserter(itemp));
	if( base64_encode(itemp, otemp) ) {
		std::string res(otemp.begin(), otemp.end());
		return res;
	}
	return "";
}

static inline bool is_base64(unsigned char c) {
#if defined(_MSC_VER)
  return (isalnum(c) || (c == '+') || (c == '/'));
#else
  return (std::isalnum(c) || (c == '+') || (c == '/'));
#endif
}

bool base64_decode(const std::vector<unsigned char>& input, std::vector<unsigned char>& output) {
	int in_len = (int)input.size();
	int i = 0;
	int in_ = 0;
	unsigned char char_array_4[4];
	unsigned char char_array_3[3];

	while (in_len-- && ( input[in_] != '=') && is_base64(input[in_])) {
		char_array_4[i++] = input[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				output.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (int j = i; j <4; j++)
			char_array_4[j] = 0;

		for (int j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (int j = 0; (j < i - 1); j++)
			output.push_back(char_array_3[j]);
	}

	return true;
}

/*
bool base64_decode(const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output) {
	if( Input.empty())
  return false;
	unsigned char stemp[4];
	size_t size(Input.size());
	for( size_t i=0; i<size; i+=4) {
		for( size_t j=0; j<4; ++j) {
			switch(Input[i+j]) {
				case 43: stemp[j] = 62; break;
				case 47: stemp[j] = 63; break;
				case 61: stemp[j] = 0xFF; break;

				default:
					if( Input[i+j] >= 65 && Input[i+j] <= 90) {
						stemp[j] = Input[i+j] - 'A';
					} else
					if( Input[i+j] >= 97 && Input[i+j] <= 122) {
						stemp[j] = Input[i+j] - 'a' + 26;
					} else
					if( Input[i+j] >= 48 && Input[i+j] <= 57) {
						stemp[j] = Input[i+j] - '0' + 52;
					} else {
					  return false;
		}
			}
		}
		Output.push_back((stemp[0] << 2) | (stemp[1] >> 4));
		if( stemp[2] != 0xFF && stemp[3] == 0xFF)
			Output.push_back((stemp[1] << 4) | (stemp[2] >> 2));
		else if( stemp[2] != 0xFF )
		{
			Output.push_back((stemp[1] << 4) | (stemp[2] >> 2));
			Output.push_back((stemp[2] << 6) | stemp[3]);
		}
	}
	return true;
}
*/

std::string base64_decodeStr(const std::string& Value) {
	std::vector<unsigned char> itemp, otemp;
	std::copy(Value.begin(), Value.end(), std::back_inserter(itemp));
	if( base64_decode(itemp, otemp) )
	{
		std::string res(otemp.begin(), otemp.end());
		return res;
	}
	return "";
}

std::vector<unsigned int> md5(const std::string& val) {
	md5_hash TestHash;
	std::vector<unsigned char> TestIn(val.begin(), val.end());
	int error;
	TestHash.update(TestIn, error);
	std::vector<unsigned int> TestOut;
	if( !TestHash.final(TestOut, error)) return std::vector<unsigned int>();
	return TestOut;
}

std::string md5_str(const std::string& val) {
	md5_hash TestHash;
	std::vector<unsigned char> TestIn(val.begin(), val.end());
	int error;
	TestHash.update(TestIn, error);
	std::string TestOut = TestHash.finalStr(error);
	return TestOut;
}

std::string md5_str(const std::string& val, unsigned int length) {
	md5_hash TestHash;
	std::vector<unsigned char> TestIn(val.begin(), val.end());
	int error;
	TestHash.update(TestIn, error);
	std::string TestOut = TestHash.finalStr(error);
	if( length >= TestOut.size() || length == 0) {
		length = TestOut.size();
	}
	std::stringstream str;
	int maxPos = TestOut.size() - length;
	int pos = maxPos > 0 ? std::rand() % maxPos : 0;
	str << std::hex << std::setfill('0') << std::setw(2) << pos;
	std::string posStr = str.str();
	TestOut = posStr + TestOut.substr(pos, length);
	return TestOut;
}

bool checkShortMD5(const std::string& val, const std::string& md5) {
	IBK_ASSERT(md5.size()>=4);
	try {
		int pos = IBK::Hex2DToShort(md5[0], md5[1]);
		int length = md5.size() - 2;
		std::string shortMD5 = md5.substr(2, length);
		std::string checkMD5 = IBK::md5_str(val);
		checkMD5 = checkMD5.substr(pos, length);
		return checkMD5 == shortMD5;
	}
	catch(...) {
		return false;
	}
}

bool blowfish_encode(const std::vector<unsigned char>& key, const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output) {
	try {
		BlowFish bf(key);
		bf.encrypt(Input, Output);
		return true;
	}
	catch(...) {
		return false;
	}
}


std::string blowfish_encodeStr(const std::string& key, const std::string& Value)
{
	try {
		const std::vector<unsigned char> Key(key.begin(), key.end());
		BlowFish bf(Key);
		std::vector<unsigned char> Input(Value.begin(), Value.end());
		std::vector<unsigned char> Output;
		bf.encrypt(Input, Output);
		return std::string(Output.begin(), Output.end());
	}
	catch(...) {
		return "";
	}
}


bool blowfish_decode(const std::vector<unsigned char>& key, const std::vector<unsigned char>& Input, std::vector<unsigned char>& Output) {
	unsigned int inSize = Input.size();
	if(inSize == 0 || inSize % 8 != 0) {
		return false;
	}
	try {
		BlowFish bf(key);
		bf.decrypt(Input, Output);
		return true;
	}
	catch(...) {
		return false;
	}
}


std::string blowfish_decodeStr(const std::string& key, const std::string& Value) {
	try {
		const std::vector<unsigned char> Key(key.begin(), key.end());
		BlowFish bf(Key);
		const std::vector<unsigned char> Input(Value.begin(), Value.end());
		unsigned int inSize = Input.size();
		if(inSize == 0 || inSize % 8 != 0) {
			return std::string();
		}
		std::vector<unsigned char> Output;
		bf.decrypt(Input, Output);
		return std::string(Output.begin(), Output.end());
	}
	catch(...) {
		return std::string();
	}
}


}	// namespace IBK

