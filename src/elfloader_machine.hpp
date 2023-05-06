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

// x86
#define ELFLOADER_MACHINE_X86 0x03
// x86-64
#define ELFLOADER_MACHINE_X64 0x3E
// RISC-V
#define ELFLOADER_MACHINE_RISCV 0xF3



// GCC architecture detection.
#ifndef ELFLOADER_MACHINE
#if defined(__i386__)
#define ELFLOADER_MACHINE ELFLOADER_MACHINE_X86
#elif defined(__x86_64__)
#define ELFLOADER_MACHINE ELFLOADER_MACHINE_X64
#elif defined(__riscv)
#define ELFLOADER_MACHINE ELFLOADER_MACHINE_RISCV
#else
#error "Unable to detect architecture or unsupported architecture."
#endif
#endif
