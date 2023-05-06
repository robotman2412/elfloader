/*
	MIT License

	Copyright (c) 2023 Julian Scheffers

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include "mpu.hpp"
#include <iostream>



// PMP region count: either 16 or 64.
#define PMP_REGIONS 16
// Lock entries when appending.
#define PMP_LOCK 0
// Enable TOR merging optimisation.
#define PMP_TOR_MERGING 1

// pmp*cfg A field values.
enum class PMPA {
	// Disabled entry.
	OFF = 0,
	// Top-of-range entry.
	TOR = 1,
	// Naturally aligned four-byte unit.
	NA4 = 2,
	// Naturally aligned power of 2 unit.
	NAPOT = 3,
};

// pmp*cfg bitfield.
struct PMPCFG {
	// Read enable.
	bool r;
	// Write enable.
	bool w;
	// Exec enable.
	bool x;
	// Entry type.
	PMPA a;
	// Locked entry.
	bool l;
	
	static PMPCFG decode(uint8_t data) {
		PMPCFG out;
		out.r = data & 0x01;
		out.w = data & 0x02;
		out.x = data & 0x04;
		out.a = (PMPA) ((data >> 3) & 3);
		out.l = data & 0x80;
		return out;
	}
};



namespace mpu {

// Whether this platform has an MPU.
bool supported() {
	// Unfortunately, there is currently no way to determine this :(
	return true;
}

// Whether this platform has a privilege-based MPU.
bool hasPrivilege() {
	return false;
}



// Minimum (least trusted) privilege level.
// Not necessarily lower than `MAX_PRIVILEGE`.
const int PRIV_MIN = 0;
// Maximum (most trusted) privilege level.
// Not necessarily higher than `MIN_PRIVILEGE`.
const int PRIV_MAX = 0;



// Determine whether the MPU is currently enabled.
bool isActive() {
	return true;
}

// Try to activate the MPU.
// Returns success status.
bool setActive(bool activity) {
	return activity;
}


// Read all PMP regions and addresses.
static void readAll(size_t *pmpcfgRaw, size_t *pmpaddr) {
	// Unfortunately, the CSRs are not indexable, only addressable.
	// This requires manually performing CSR reads.
	asm volatile ("csrr %0, pmpcfg0"  : "=r" (pmpcfgRaw[0]));
	asm volatile ("csrr %0, pmpcfg1"  : "=r" (pmpcfgRaw[1]));
	asm volatile ("csrr %0, pmpcfg2"  : "=r" (pmpcfgRaw[2]));
	asm volatile ("csrr %0, pmpcfg3"  : "=r" (pmpcfgRaw[3]));
	#if PMP_REGIONS == 64
	asm volatile ("csrr %0, pmpcfg4"  : "=r" (pmpcfgRaw[4]));
	asm volatile ("csrr %0, pmpcfg5"  : "=r" (pmpcfgRaw[5]));
	asm volatile ("csrr %0, pmpcfg6"  : "=r" (pmpcfgRaw[6]));
	asm volatile ("csrr %0, pmpcfg7"  : "=r" (pmpcfgRaw[7]));
	asm volatile ("csrr %0, pmpcfg8"  : "=r" (pmpcfgRaw[8]));
	asm volatile ("csrr %0, pmpcfg9"  : "=r" (pmpcfgRaw[9]));
	asm volatile ("csrr %0, pmpcfg10" : "=r" (pmpcfgRaw[10]));
	asm volatile ("csrr %0, pmpcfg11" : "=r" (pmpcfgRaw[11]));
	asm volatile ("csrr %0, pmpcfg12" : "=r" (pmpcfgRaw[12]));
	asm volatile ("csrr %0, pmpcfg13" : "=r" (pmpcfgRaw[13]));
	asm volatile ("csrr %0, pmpcfg14" : "=r" (pmpcfgRaw[14]));
	asm volatile ("csrr %0, pmpcfg15" : "=r" (pmpcfgRaw[15]));
	#endif
	
	// Same type of indexing by hand applies to addresses.
	asm volatile ("csrr %0, pmpaddr0"  : "=r" (pmpaddr[0]));
	asm volatile ("csrr %0, pmpaddr1"  : "=r" (pmpaddr[1]));
	asm volatile ("csrr %0, pmpaddr2"  : "=r" (pmpaddr[2]));
	asm volatile ("csrr %0, pmpaddr3"  : "=r" (pmpaddr[3]));
	asm volatile ("csrr %0, pmpaddr4"  : "=r" (pmpaddr[4]));
	asm volatile ("csrr %0, pmpaddr5"  : "=r" (pmpaddr[5]));
	asm volatile ("csrr %0, pmpaddr6"  : "=r" (pmpaddr[6]));
	asm volatile ("csrr %0, pmpaddr7"  : "=r" (pmpaddr[7]));
	asm volatile ("csrr %0, pmpaddr8"  : "=r" (pmpaddr[8]));
	asm volatile ("csrr %0, pmpaddr9"  : "=r" (pmpaddr[9]));
	asm volatile ("csrr %0, pmpaddr10" : "=r" (pmpaddr[10]));
	asm volatile ("csrr %0, pmpaddr11" : "=r" (pmpaddr[11]));
	asm volatile ("csrr %0, pmpaddr12" : "=r" (pmpaddr[12]));
	asm volatile ("csrr %0, pmpaddr13" : "=r" (pmpaddr[13]));
	asm volatile ("csrr %0, pmpaddr14" : "=r" (pmpaddr[14]));
	asm volatile ("csrr %0, pmpaddr15" : "=r" (pmpaddr[15]));
	#if PMP_REGIONS == 64
	asm volatile ("csrr %0, pmpaddr16" : "=r" (pmpaddr[16]));
	asm volatile ("csrr %0, pmpaddr17" : "=r" (pmpaddr[17]));
	asm volatile ("csrr %0, pmpaddr18" : "=r" (pmpaddr[18]));
	asm volatile ("csrr %0, pmpaddr19" : "=r" (pmpaddr[19]));
	asm volatile ("csrr %0, pmpaddr20" : "=r" (pmpaddr[20]));
	asm volatile ("csrr %0, pmpaddr21" : "=r" (pmpaddr[21]));
	asm volatile ("csrr %0, pmpaddr22" : "=r" (pmpaddr[22]));
	asm volatile ("csrr %0, pmpaddr23" : "=r" (pmpaddr[23]));
	asm volatile ("csrr %0, pmpaddr24" : "=r" (pmpaddr[24]));
	asm volatile ("csrr %0, pmpaddr25" : "=r" (pmpaddr[25]));
	asm volatile ("csrr %0, pmpaddr26" : "=r" (pmpaddr[26]));
	asm volatile ("csrr %0, pmpaddr27" : "=r" (pmpaddr[27]));
	asm volatile ("csrr %0, pmpaddr28" : "=r" (pmpaddr[28]));
	asm volatile ("csrr %0, pmpaddr29" : "=r" (pmpaddr[29]));
	asm volatile ("csrr %0, pmpaddr30" : "=r" (pmpaddr[30]));
	asm volatile ("csrr %0, pmpaddr31" : "=r" (pmpaddr[31]));
	asm volatile ("csrr %0, pmpaddr32" : "=r" (pmpaddr[32]));
	asm volatile ("csrr %0, pmpaddr33" : "=r" (pmpaddr[33]));
	asm volatile ("csrr %0, pmpaddr34" : "=r" (pmpaddr[34]));
	asm volatile ("csrr %0, pmpaddr35" : "=r" (pmpaddr[35]));
	asm volatile ("csrr %0, pmpaddr36" : "=r" (pmpaddr[36]));
	asm volatile ("csrr %0, pmpaddr37" : "=r" (pmpaddr[37]));
	asm volatile ("csrr %0, pmpaddr38" : "=r" (pmpaddr[38]));
	asm volatile ("csrr %0, pmpaddr39" : "=r" (pmpaddr[39]));
	asm volatile ("csrr %0, pmpaddr40" : "=r" (pmpaddr[40]));
	asm volatile ("csrr %0, pmpaddr41" : "=r" (pmpaddr[41]));
	asm volatile ("csrr %0, pmpaddr42" : "=r" (pmpaddr[42]));
	asm volatile ("csrr %0, pmpaddr43" : "=r" (pmpaddr[43]));
	asm volatile ("csrr %0, pmpaddr44" : "=r" (pmpaddr[44]));
	asm volatile ("csrr %0, pmpaddr45" : "=r" (pmpaddr[45]));
	asm volatile ("csrr %0, pmpaddr46" : "=r" (pmpaddr[46]));
	asm volatile ("csrr %0, pmpaddr47" : "=r" (pmpaddr[47]));
	asm volatile ("csrr %0, pmpaddr48" : "=r" (pmpaddr[48]));
	asm volatile ("csrr %0, pmpaddr49" : "=r" (pmpaddr[49]));
	asm volatile ("csrr %0, pmpaddr50" : "=r" (pmpaddr[50]));
	asm volatile ("csrr %0, pmpaddr51" : "=r" (pmpaddr[51]));
	asm volatile ("csrr %0, pmpaddr52" : "=r" (pmpaddr[52]));
	asm volatile ("csrr %0, pmpaddr53" : "=r" (pmpaddr[53]));
	asm volatile ("csrr %0, pmpaddr54" : "=r" (pmpaddr[54]));
	asm volatile ("csrr %0, pmpaddr55" : "=r" (pmpaddr[55]));
	asm volatile ("csrr %0, pmpaddr56" : "=r" (pmpaddr[56]));
	asm volatile ("csrr %0, pmpaddr57" : "=r" (pmpaddr[57]));
	asm volatile ("csrr %0, pmpaddr58" : "=r" (pmpaddr[58]));
	asm volatile ("csrr %0, pmpaddr59" : "=r" (pmpaddr[59]));
	asm volatile ("csrr %0, pmpaddr60" : "=r" (pmpaddr[60]));
	asm volatile ("csrr %0, pmpaddr61" : "=r" (pmpaddr[61]));
	asm volatile ("csrr %0, pmpaddr62" : "=r" (pmpaddr[62]));
	asm volatile ("csrr %0, pmpaddr63" : "=r" (pmpaddr[63]));
	#endif
}

// Read pmpcfg* register by index.
static size_t readCfgRaw(int index) {
	size_t out;
	
	// Long live the stupidly long switch case!
	switch (index) {
		case (0): asm ("csrr %0, pmpcfg0" : "=r" (out)); break;
		case (1): asm ("csrr %0, pmpcfg1" : "=r" (out)); break;
		case (2): asm ("csrr %0, pmpcfg2" : "=r" (out)); break;
		case (3): asm ("csrr %0, pmpcfg3" : "=r" (out)); break;
		#if PMP_REGIONS == 64
		case (4): asm ("csrr %0, pmpcfg4" : "=r" (out)); break;
		case (5): asm ("csrr %0, pmpcfg5" : "=r" (out)); break;
		case (6): asm ("csrr %0, pmpcfg6" : "=r" (out)); break;
		case (7): asm ("csrr %0, pmpcfg7" : "=r" (out)); break;
		case (8): asm ("csrr %0, pmpcfg8" : "=r" (out)); break;
		case (9): asm ("csrr %0, pmpcfg9" : "=r" (out)); break;
		case (10): asm ("csrr %0, pmpcfg10" : "=r" (out)); break;
		case (11): asm ("csrr %0, pmpcfg11" : "=r" (out)); break;
		case (12): asm ("csrr %0, pmpcfg12" : "=r" (out)); break;
		case (13): asm ("csrr %0, pmpcfg13" : "=r" (out)); break;
		case (14): asm ("csrr %0, pmpcfg14" : "=r" (out)); break;
		case (15): asm ("csrr %0, pmpcfg15" : "=r" (out)); break;
		#endif
		default: out = 0; break;
	}
	return out;
}

// Read mpm*cfg by index.
static uint8_t readCfg(int index) {
	size_t raw = readCfgRaw(index / 4);
	return raw >> (index*8);
}

// Read pmpaddr* register by index.
static size_t readAddr(int index) {
	size_t out;
	
	// Long live the stupidly long switch case!
	switch (index) {
		case (0): asm ("csrr %0, pmpaddr0" : "=r" (out)); break;
		case (1): asm ("csrr %0, pmpaddr1" : "=r" (out)); break;
		case (2): asm ("csrr %0, pmpaddr2" : "=r" (out)); break;
		case (3): asm ("csrr %0, pmpaddr3" : "=r" (out)); break;
		case (4): asm ("csrr %0, pmpaddr4" : "=r" (out)); break;
		case (5): asm ("csrr %0, pmpaddr5" : "=r" (out)); break;
		case (6): asm ("csrr %0, pmpaddr6" : "=r" (out)); break;
		case (7): asm ("csrr %0, pmpaddr7" : "=r" (out)); break;
		case (8): asm ("csrr %0, pmpaddr8" : "=r" (out)); break;
		case (9): asm ("csrr %0, pmpaddr9" : "=r" (out)); break;
		case (10): asm ("csrr %0, pmpaddr10" : "=r" (out)); break;
		case (11): asm ("csrr %0, pmpaddr11" : "=r" (out)); break;
		case (12): asm ("csrr %0, pmpaddr12" : "=r" (out)); break;
		case (13): asm ("csrr %0, pmpaddr13" : "=r" (out)); break;
		case (14): asm ("csrr %0, pmpaddr14" : "=r" (out)); break;
		case (15): asm ("csrr %0, pmpaddr15" : "=r" (out)); break;
		#if PMP_REGIONS == 64
		case (16): asm ("csrr %0, pmpaddr16" : "=r" (out)); break;
		case (17): asm ("csrr %0, pmpaddr17" : "=r" (out)); break;
		case (18): asm ("csrr %0, pmpaddr18" : "=r" (out)); break;
		case (19): asm ("csrr %0, pmpaddr19" : "=r" (out)); break;
		case (20): asm ("csrr %0, pmpaddr20" : "=r" (out)); break;
		case (21): asm ("csrr %0, pmpaddr21" : "=r" (out)); break;
		case (22): asm ("csrr %0, pmpaddr22" : "=r" (out)); break;
		case (23): asm ("csrr %0, pmpaddr23" : "=r" (out)); break;
		case (24): asm ("csrr %0, pmpaddr24" : "=r" (out)); break;
		case (25): asm ("csrr %0, pmpaddr25" : "=r" (out)); break;
		case (26): asm ("csrr %0, pmpaddr26" : "=r" (out)); break;
		case (27): asm ("csrr %0, pmpaddr27" : "=r" (out)); break;
		case (28): asm ("csrr %0, pmpaddr28" : "=r" (out)); break;
		case (29): asm ("csrr %0, pmpaddr29" : "=r" (out)); break;
		case (30): asm ("csrr %0, pmpaddr30" : "=r" (out)); break;
		case (31): asm ("csrr %0, pmpaddr31" : "=r" (out)); break;
		case (32): asm ("csrr %0, pmpaddr32" : "=r" (out)); break;
		case (33): asm ("csrr %0, pmpaddr33" : "=r" (out)); break;
		case (34): asm ("csrr %0, pmpaddr34" : "=r" (out)); break;
		case (35): asm ("csrr %0, pmpaddr35" : "=r" (out)); break;
		case (36): asm ("csrr %0, pmpaddr36" : "=r" (out)); break;
		case (37): asm ("csrr %0, pmpaddr37" : "=r" (out)); break;
		case (38): asm ("csrr %0, pmpaddr38" : "=r" (out)); break;
		case (39): asm ("csrr %0, pmpaddr39" : "=r" (out)); break;
		case (40): asm ("csrr %0, pmpaddr40" : "=r" (out)); break;
		case (41): asm ("csrr %0, pmpaddr41" : "=r" (out)); break;
		case (42): asm ("csrr %0, pmpaddr42" : "=r" (out)); break;
		case (43): asm ("csrr %0, pmpaddr43" : "=r" (out)); break;
		case (44): asm ("csrr %0, pmpaddr44" : "=r" (out)); break;
		case (45): asm ("csrr %0, pmpaddr45" : "=r" (out)); break;
		case (46): asm ("csrr %0, pmpaddr46" : "=r" (out)); break;
		case (47): asm ("csrr %0, pmpaddr47" : "=r" (out)); break;
		case (48): asm ("csrr %0, pmpaddr48" : "=r" (out)); break;
		case (49): asm ("csrr %0, pmpaddr49" : "=r" (out)); break;
		case (50): asm ("csrr %0, pmpaddr50" : "=r" (out)); break;
		case (51): asm ("csrr %0, pmpaddr51" : "=r" (out)); break;
		case (52): asm ("csrr %0, pmpaddr52" : "=r" (out)); break;
		case (53): asm ("csrr %0, pmpaddr53" : "=r" (out)); break;
		case (54): asm ("csrr %0, pmpaddr54" : "=r" (out)); break;
		case (55): asm ("csrr %0, pmpaddr55" : "=r" (out)); break;
		case (56): asm ("csrr %0, pmpaddr56" : "=r" (out)); break;
		case (57): asm ("csrr %0, pmpaddr57" : "=r" (out)); break;
		case (58): asm ("csrr %0, pmpaddr58" : "=r" (out)); break;
		case (59): asm ("csrr %0, pmpaddr59" : "=r" (out)); break;
		case (60): asm ("csrr %0, pmpaddr60" : "=r" (out)); break;
		case (61): asm ("csrr %0, pmpaddr61" : "=r" (out)); break;
		case (62): asm ("csrr %0, pmpaddr62" : "=r" (out)); break;
		case (63): asm ("csrr %0, pmpaddr63" : "=r" (out)); break;
		#endif
		default: out = 0; break;
	}
	return out;
}

// Write pmpcfg* register by index.
static void writeCfgRaw(int index, size_t in) {
	// Long live the stupidly long switch case!
	switch (index) {
		case (0): asm ("csrw pmpcfg0, %0" :: "r" (in)); break;
		case (1): asm ("csrw pmpcfg1, %0" :: "r" (in)); break;
		case (2): asm ("csrw pmpcfg2, %0" :: "r" (in)); break;
		case (3): asm ("csrw pmpcfg3, %0" :: "r" (in)); break;
		#if PMP_REGIONS == 64
		case (4): asm ("csrw pmpcfg4, %0" :: "r" (in)); break;
		case (5): asm ("csrw pmpcfg5, %0" :: "r" (in)); break;
		case (6): asm ("csrw pmpcfg6, %0" :: "r" (in)); break;
		case (7): asm ("csrw pmpcfg7, %0" :: "r" (in)); break;
		case (8): asm ("csrw pmpcfg8, %0" :: "r" (in)); break;
		case (9): asm ("csrw pmpcfg9, %0" :: "r" (in)); break;
		case (10): asm ("csrw pmpcfg10, %0" :: "r" (in)); break;
		case (11): asm ("csrw pmpcfg11, %0" :: "r" (in)); break;
		case (12): asm ("csrw pmpcfg12, %0" :: "r" (in)); break;
		case (13): asm ("csrw pmpcfg13, %0" :: "r" (in)); break;
		case (14): asm ("csrw pmpcfg14, %0" :: "r" (in)); break;
		case (15): asm ("csrw pmpcfg15, %0" :: "r" (in)); break;
		#endif
	}
}

// Write pmp*cfg by index.
static void writeCfg(int index, uint8_t in) {
	// This is packed, read the entire CSR.
	size_t orig = readCfgRaw(index/4);
	// Mask out the part to write.
	orig &= ~(0xff << (index*8));
	// Put in new data.
	orig |= in << (index*8);
	// Write back.
	writeCfgRaw(index/4, orig);
}

// Write pmpaddr* register by index.
static void writeAddr(int index, size_t in) {
	// Long live the stupidly long switch case!
	switch (index) {
		case (0): asm ("csrw pmpaddr0, %0" :: "r" (in)); break;
		case (1): asm ("csrw pmpaddr1, %0" :: "r" (in)); break;
		case (2): asm ("csrw pmpaddr2, %0" :: "r" (in)); break;
		case (3): asm ("csrw pmpaddr3, %0" :: "r" (in)); break;
		case (4): asm ("csrw pmpaddr4, %0" :: "r" (in)); break;
		case (5): asm ("csrw pmpaddr5, %0" :: "r" (in)); break;
		case (6): asm ("csrw pmpaddr6, %0" :: "r" (in)); break;
		case (7): asm ("csrw pmpaddr7, %0" :: "r" (in)); break;
		case (8): asm ("csrw pmpaddr8, %0" :: "r" (in)); break;
		case (9): asm ("csrw pmpaddr9, %0" :: "r" (in)); break;
		case (10): asm ("csrw pmpaddr10, %0" :: "r" (in)); break;
		case (11): asm ("csrw pmpaddr11, %0" :: "r" (in)); break;
		case (12): asm ("csrw pmpaddr12, %0" :: "r" (in)); break;
		case (13): asm ("csrw pmpaddr13, %0" :: "r" (in)); break;
		case (14): asm ("csrw pmpaddr14, %0" :: "r" (in)); break;
		case (15): asm ("csrw pmpaddr15, %0" :: "r" (in)); break;
		#if PMP_REGIONS == 64
		case (16): asm ("csrw pmpaddr16, %0" :: "r" (in)); break;
		case (17): asm ("csrw pmpaddr17, %0" :: "r" (in)); break;
		case (18): asm ("csrw pmpaddr18, %0" :: "r" (in)); break;
		case (19): asm ("csrw pmpaddr19, %0" :: "r" (in)); break;
		case (20): asm ("csrw pmpaddr20, %0" :: "r" (in)); break;
		case (21): asm ("csrw pmpaddr21, %0" :: "r" (in)); break;
		case (22): asm ("csrw pmpaddr22, %0" :: "r" (in)); break;
		case (23): asm ("csrw pmpaddr23, %0" :: "r" (in)); break;
		case (24): asm ("csrw pmpaddr24, %0" :: "r" (in)); break;
		case (25): asm ("csrw pmpaddr25, %0" :: "r" (in)); break;
		case (26): asm ("csrw pmpaddr26, %0" :: "r" (in)); break;
		case (27): asm ("csrw pmpaddr27, %0" :: "r" (in)); break;
		case (28): asm ("csrw pmpaddr28, %0" :: "r" (in)); break;
		case (29): asm ("csrw pmpaddr29, %0" :: "r" (in)); break;
		case (30): asm ("csrw pmpaddr30, %0" :: "r" (in)); break;
		case (31): asm ("csrw pmpaddr31, %0" :: "r" (in)); break;
		case (32): asm ("csrw pmpaddr32, %0" :: "r" (in)); break;
		case (33): asm ("csrw pmpaddr33, %0" :: "r" (in)); break;
		case (34): asm ("csrw pmpaddr34, %0" :: "r" (in)); break;
		case (35): asm ("csrw pmpaddr35, %0" :: "r" (in)); break;
		case (36): asm ("csrw pmpaddr36, %0" :: "r" (in)); break;
		case (37): asm ("csrw pmpaddr37, %0" :: "r" (in)); break;
		case (38): asm ("csrw pmpaddr38, %0" :: "r" (in)); break;
		case (39): asm ("csrw pmpaddr39, %0" :: "r" (in)); break;
		case (40): asm ("csrw pmpaddr40, %0" :: "r" (in)); break;
		case (41): asm ("csrw pmpaddr41, %0" :: "r" (in)); break;
		case (42): asm ("csrw pmpaddr42, %0" :: "r" (in)); break;
		case (43): asm ("csrw pmpaddr43, %0" :: "r" (in)); break;
		case (44): asm ("csrw pmpaddr44, %0" :: "r" (in)); break;
		case (45): asm ("csrw pmpaddr45, %0" :: "r" (in)); break;
		case (46): asm ("csrw pmpaddr46, %0" :: "r" (in)); break;
		case (47): asm ("csrw pmpaddr47, %0" :: "r" (in)); break;
		case (48): asm ("csrw pmpaddr48, %0" :: "r" (in)); break;
		case (49): asm ("csrw pmpaddr49, %0" :: "r" (in)); break;
		case (50): asm ("csrw pmpaddr50, %0" :: "r" (in)); break;
		case (51): asm ("csrw pmpaddr51, %0" :: "r" (in)); break;
		case (52): asm ("csrw pmpaddr52, %0" :: "r" (in)); break;
		case (53): asm ("csrw pmpaddr53, %0" :: "r" (in)); break;
		case (54): asm ("csrw pmpaddr54, %0" :: "r" (in)); break;
		case (55): asm ("csrw pmpaddr55, %0" :: "r" (in)); break;
		case (56): asm ("csrw pmpaddr56, %0" :: "r" (in)); break;
		case (57): asm ("csrw pmpaddr57, %0" :: "r" (in)); break;
		case (58): asm ("csrw pmpaddr58, %0" :: "r" (in)); break;
		case (59): asm ("csrw pmpaddr59, %0" :: "r" (in)); break;
		case (60): asm ("csrw pmpaddr60, %0" :: "r" (in)); break;
		case (61): asm ("csrw pmpaddr61, %0" :: "r" (in)); break;
		case (62): asm ("csrw pmpaddr62, %0" :: "r" (in)); break;
		case (63): asm ("csrw pmpaddr63, %0" :: "r" (in)); break;
		#endif
	}
}

// Find the first empty PMP region.
// Returns -1 if not found.
static int findEmpty() {
	for (int i = 0; i < PMP_REGIONS; i += 4) {
		size_t cfgRaw = readCfgRaw(i);
		for (int j = 0; j < 4; j++) {
			uint8_t cfg = cfgRaw >> (j*8);
			if (!(cfg & 0x98)) return i+j;
		}
	}
	return -1;
}

// Find the first pair of two empty PMP regions.
// Returns index of lower PMP entry, -1 if not found.
static int findEmptyPair() {
	size_t pmpPrev = 0xffffffff;
	for (int i = 0; i < PMP_REGIONS; i += 4) {
		size_t cfgRaw = readCfgRaw(i/4);
		if (!(pmpPrev & 0x98000000) && !(cfgRaw & 0x98)) return i-1;
		for (int j = 0; j < 3; j++) {
			uint8_t cfg0 = cfgRaw >> (j*8);
			uint8_t cfg1 = cfgRaw >> (j*8+8);
			if (!(cfg0 & 0x98) && !(cfg1 & 0x98)) return i+j;
		}
		pmpPrev = cfgRaw;
	}
	return -1;
}


// Get current number of regions.
int regionCount() {
	return 0;
}

// Get maximum number of regions in current configuration.
int regionMax() {
	return PMP_REGIONS;
}

// Determine granularity.
size_t granularity() {
	int slot = findEmpty();
	writeAddr(slot, -1);
	auto gmask = readAddr(slot);
	auto granularity = ~gmask + 1;
	return granularity * 4;
}

// Determine whether two [start,start+len) ranges overlap.
static bool overlap(auto startA, auto lenA, auto startB, auto lenB) {
	// Base address of B is within A.
	if (startB >= startA && startB < startA + lenA) return true;
	// Base address of A is within B.
	if (startA >= startB && startA < startB + lenB) return true;
	// If neither was true, there is no overlap.
	return false;
}

// Determine whether two regions overlap.
static bool overlap(const Region &a, const Region &b) {
	return overlap(a.base, a.size, b.base, b.size);
}

// Find the bit index of the LSB that is zero.
static int zeroIndex(auto input) {
	int index = 0;
	while (input & 1) {
		input >>= 1;
		index ++;
	}
	return index;
}

// Decode a PMP region from a `pmpcfg` index.
static Region decodeRegion(int index, uint8_t *pmpcfg, size_t *pmpaddr) {
	// Grab config bit-field.
	auto config = PMPCFG::decode(pmpcfg[index]);
	
	if (config.a == PMPA::OFF) {
		// Region is off.
		return {0,0,0,0,0,0,0};
		
	} else if (config.a == PMPA::NA4) {
		// Region is 4 bytes long.
		return {
			pmpaddr[index] * 4, 4,
			PRIV_MIN,
			config.r, config.w, config.x,
			!PMP_LOCK || config.l
		};
		
	} else if (config.a == PMPA::NAPOT) {
		// Region is NAPOT.
		// Compute length.
		size_t zi   = zeroIndex(pmpaddr[index]);
		size_t len  = 8 << zi;
		// Compute bitmask for base address.
		size_t mask = (1 << zi) - 1;
		return {
			(pmpaddr[index] & ~mask) * 4, len,
			PRIV_MIN,
			config.r, config.w, config.x,
			!PMP_LOCK || config.l
		};
		
	} else /*if (config.a == PMPA::TOR)*/ {
		// Region is TOR.
		// Find base address (inclusive).
		size_t base = index ? pmpaddr[index-1] * 4 : 0;
		// And top address (exclusive).
		size_t top  = pmpaddr[index] * 4;
		return {
			base, top-base,
			PRIV_MIN,
			config.r, config.w, config.x,
			!PMP_LOCK || config.l
		};
	}
}


// Try to append or update an MPU region.
// If this partially overlaps with a preexisting region, the existing region is removed.
bool appendRegion(Region wdata) {
	wdata.read |= wdata.write | wdata.exec;
	
	// Determine whether two entries are required.
	bool isLong = wdata.base % wdata.size || (wdata.size & (wdata.size-1));
	
	// Find an empty slot.
	int slot = isLong ? findEmptyPair() : findEmpty();
	if (slot < 0) {
		std::cout << "MPU: Out of entries.\n";
		return false;
	}
	
	// Determine address granularity.
	std::cout << "Slot: " << std::dec << slot << '\n';
	writeAddr(slot, -1);
	auto gmask = readAddr(slot);
	auto granularity = ~gmask + 1;
	
	// Enforce granularity requirements.
	if ((wdata.base | wdata.size) & 3) {
		std::cout << "MPU: Granularity error (4).\n";
		return false;
	}
	if ((wdata.base >> 2) & ~gmask || (wdata.size >> 2) & ~gmask) {
		std::cout << "MPU: Granularity error (0x" << std::hex << granularity << std::dec << ").\n";
		return false;
	}
	
	// Choose strategy.
	if (isLong && PMP_TOR_MERGING && readAddr(slot-1) == (wdata.base >> 2)) {
		// Single-entry TOR strategy.
		std::cout << "TOR (1):\n";
		std::cout << "  Base: 0x" << std::hex << wdata.base << '\n';
		std::cout << "  Size: 0x" << std::hex << wdata.size << '\n';
		std::cout << "  Adr0: 0x" << (wdata.base >> 2) << '\n';
		std::cout << "  Adr1: 0x" << ((wdata.base >> 2) + (wdata.size >> 2)) << '\n';
		writeAddr(slot, (wdata.base >> 2) + (wdata.size >> 2));
		// Generate config.
		uint8_t config = PMP_LOCK * 0x80;
		if (wdata.read)  config |= 0x01;
		if (wdata.write) config |= 0x02;
		if (wdata.exec)  config |= 0x04;
		std::cout << "  Cfg:  0x" << (config | 0x08) << '\n';
		std::cout << '\n';
		writeCfg(slot, config | 0x08);
		
	} else if (isLong) {
		// Two-entry TOR strategy.
		std::cout << "TOR (2):\n";
		std::cout << "  Base: 0x" << std::hex << wdata.base << '\n';
		std::cout << "  Size: 0x" << std::hex << wdata.size << '\n';
		std::cout << "  Adr0: 0x" << (wdata.base >> 2) << '\n';
		std::cout << "  Adr1: 0x" << ((wdata.base >> 2) + (wdata.size >> 2)) << '\n';
		writeAddr(slot,   wdata.base >> 2);
		writeAddr(slot+1, (wdata.base >> 2) + (wdata.size >> 2));
		// Generate config.
		uint8_t config = PMP_LOCK * 0x80;
		if (wdata.read)  config |= 0x01;
		if (wdata.write) config |= 0x02;
		if (wdata.exec)  config |= 0x04;
		std::cout << "  Cfg:  0x" << (config | 0x08) << '\n';
		std::cout << '\n';
		writeCfg(slot, config | 0x10);
		writeCfg(slot+1, config | 0x08);
		
	} else if ((wdata.base | wdata.size) & 4) {
		// NA4 strategy.
		size_t addr  = wdata.base >> 2;
		std::cout << "NA4:\n";
		std::cout << "  Base: 0x" << std::hex << wdata.base << '\n';
		std::cout << "  Size: 0x" << std::hex << wdata.size << '\n';
		std::cout << "  Addr: 0x" << addr << '\n';
		writeAddr(slot, wdata.base >> 2);
		// Generate config.
		uint8_t config = PMP_LOCK * 0x80 | 0x10;
		if (wdata.read)  config |= 0x01;
		if (wdata.write) config |= 0x02;
		if (wdata.exec)  config |= 0x04;
		std::cout << "  Cfg:  0x" << (unsigned) config << '\n';
		std::cout << '\n';
		writeCfg(slot, config);
		
	} else {
		// NAPOT strategy.
		size_t addr  = wdata.base >> 2;
		size_t amask = (wdata.size >> 2) - 1;
		addr &= ~amask;
		addr |= amask >> 1;
		std::cout << "NAPOT:\n";
		std::cout << "  Base: 0x" << std::hex << wdata.base << '\n';
		std::cout << "  Size: 0x" << std::hex << wdata.size << '\n';
		std::cout << "  Mask: 0x" << amask << '\n';
		std::cout << "  Addr: 0x" << addr << '\n';
		writeAddr(slot, addr);
		// Generate config.
		uint8_t config = PMP_LOCK * 0x80 | 0x18;
		if (wdata.read)  config |= 0x01;
		if (wdata.write) config |= 0x02;
		if (wdata.exec)  config |= 0x04;
		std::cout << "  Cfg:  0x" << (unsigned) config << '\n';
		std::cout << '\n';
		writeCfg(slot, config);
	}
	
	return true;
}

// Get a list containing all regions present.
// On fail, returns an empty list.
// Entries may be in the list but empty.
std::vector<Region> readRegions() {
	std::vector<Region> list;
	list.reserve(PMP_REGIONS);
	
	// Buffer for pmpcfg* registers.
	size_t pmpcfgRaw[PMP_REGIONS/sizeof(size_t)];
	// Buffer for pmp*cfg values.
	auto   pmpcfg = (uint8_t *) &pmpcfgRaw;
	// Buffer for pmpaddr* registers.
	size_t pmpaddr[PMP_REGIONS];
	readAll(pmpcfgRaw, pmpaddr);
	
	// Translate register datas.
	for (int i = 0; i < PMP_REGIONS; i++) {
		list.push_back(decodeRegion(i, pmpcfg, pmpaddr));
	}
	std::cout << std::dec;
	
	return list;
}

};
