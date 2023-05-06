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

#include "relocation.hpp"
#include "elfloader_int.hpp"

/*
A			Addend field in the relocation entry associated with the symbol
B			Base address of a shared object loaded into memory
G			Offset of the symbol into the GOT (Global Offset Table)
GOT			Address of the GOT (Global Offset Table)
P			Position of the relocation
S			Value of the symbol in the symbol table
V			Value at the position of the relocation
GP			Value of __global_pointer$ symbol
TLSMODULE	TLS module index for the object containing the symbol
TLSOFFSET	TLS static block offset (relative to tp) for the object containing the symbol
*/

namespace elf {

// RISC-V relocation enum.
enum class Reloc {
	ABS32        = 1,
	ABS64        = 2,
	RELATIVE     = 3,
	COPY         = 4,
	JUMP_SLOT    = 5,
	TLS_DTPMOD32 = 6,
	TLS_DTPMOD64 = 7,
	TLS_DTPREL32 = 8,
	TLS_DTPREL64 = 9,
	TLS_TPREL32  = 10,
	TLS_TPREL64  = 11,
	IRELATIVE    = 58,
};

// STORE TEMPLATE.
template<typename T>
void store(uint8_t *ptr, T in) {
	for (int i = 0; i < sizeof(T); i++) {
		ptr[i] = in >> (8 * i);
	}
}



// Reads an ADDEND for a relocation.
Addr getAddend(const ELFFile &ctx, uint32_t relType, uint8_t *ptr) {
	// TODO.
	return 0;
}



#define A addend
#define S symVal
#define B program.vaddr_offset()

// Apply a relocation.
bool applyRelocation(const ELFFile &ctx, const Program &program, uint32_t relType, Addr symVal, Addr addend, uint8_t *ptr) {
	switch ((Reloc) relType) {
		case Reloc::ABS32:
			LOGD("Reloc R_RISCV_32");
			store<uint32_t>(ptr, S + A);
			return true;
			
		case Reloc::ABS64:
			LOGD("Reloc R_RISCV_64");
			store<uint64_t>(ptr, S + A);
			return true;
			
		case Reloc::RELATIVE:
			LOGD("Reloc R_RISCV_RELATIVE");
			store<Addr>(ptr, B + A);
			return true;
			
		case Reloc::JUMP_SLOT:
			LOGD("Reloc R_RISCV_JUMP_SLOT");
			store<Addr>(ptr, S);
			return true;
			
			// TODO: TLS_* relocations.
			
		default:
			LOGE("Invalid dynamic relocation type 0x%x", (int) relType);
			return false;
	}
}



} // namespace elf
