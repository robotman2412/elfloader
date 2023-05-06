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

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <vector>

#include "elfloader.hpp"

namespace mpu {

// Whether this platform has an MPU.
bool supported();
// Whether this platform has a privilege-based MPU.
bool hasPrivilege();

// Minimum (least trusted) privilege level.
// Not necessarily lower than `MAX_PRIVILEGE`.
extern const int PRIV_MIN;
// Maximum (most trusted) privilege level.
// Not necessarily higher than `MIN_PRIVILEGE`.
extern const int PRIV_MAX;

// Representation of an MPU region.
// Region addresses may be either constant or dynamic based on the type of MPU.
struct Region {
	// Region starting address.
	uint64_t base;
	// Region length.
	uint64_t size;
	
	// The memory's privilege requirement.
	int privilege;
	
	// Allow read.
	// May become implicitly true if either `write` or `exec` are true.
	bool read;
	// Allow write.
	bool write;
	// Allow execute.
	bool exec;
	
	// Whether this region is active (i.e. being enforced).
	bool active;
	
	// Combines read, write and execute into a single number.
	// Read is bit 0, write bit 1 and execute bit 2.
	uint8_t rwx() {
		return read | (write << 1) | (exec << 2);
	}
};

// Determine whether the MPU is currently enabled.
bool isActive();
// Try to activate the MPU.
// Returns success status.
bool setActive(bool activity);

// Get current number of regions.
int regionCount();
// Get maximum number of regions in current configuration.
int regionMax();
// Determine granularity.
size_t granularity();

// Try to append or update an MPU region.
// If this partially overlaps with a preexisting region, the existing region is removed.
bool appendRegion(Region wdata);
// Get a list containing all regions present.
// On fail, returns an empty list.
// Entries may be in the list but empty.
std::vector<Region> readRegions();

// Merge regions of the same type into a new list.
std::vector<Region> pureMerge(const std::vector<Region> &in);
// Aggressively merge regions, even if that means losing information.
std::vector<Region> lossyMerge(const std::vector<Region> &in);
// Set memory protections based on program headers.
bool applyPH(const elf::ELFFile &ctx, const elf::Program &program);
// Set memory protections based on section headers.
bool applySect(const elf::ELFFile &ctx, const elf::Program &program);

};
