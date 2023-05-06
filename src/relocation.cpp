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

namespace elf {

// Try to look up a symbol's value.
static inline bool getDynSym(Addr &out, const ELFFile &ctx, const Program &program, const SectInfo &sect, const SymMap &map, uint32_t index) {
	if (index == 0) {
		// No symbol associated.
		out = 0;
		return true;
	}
	
	// Look up symbol.
	auto &sym = ctx.getDynSym()[index];
	if (sym.section >= 0xff00) {
		LOGW("TODO");
		return false;
		
	} else if (sym.section == 0) {
		// Look up in map.
		auto iter = map.find(sym.name);
		if (iter == map.end()) {
			// Not found.
			return false;
		}
		LOGD("Match found for %s", sym.name.c_str());
		out = iter->second;
		return true;
		
	} else {
		// Use defined value.
		out = sym.value + program.vaddr_offset();
		return true;
	}
}

// Apply explicit addend relocations.
static bool relocateExplicit(const ELFFile &ctx, const Program &program, const SectInfo &sect, const SymMap &map) {
	FILE *fd = ctx.fd;
	
	// Read relocation datas from the SECTION.
	LOGD("Relocating %lu entries", sect.file_size / sect.entry_size);
	for (size_t i = 0; i < sect.file_size / sect.entry_size; i++) {
		// Read raw data stuffs.
		RelaEntry entry;
		LOGD("%lu", sect.offset + i * sect.entry_size);
		SEEK(sect.offset + i * sect.entry_size);
		READ(&entry, sizeof(entry));
		LOGD("Read entry %zu/%lu", i+1, sect.file_size / sect.entry_size);
		
		// Bounds check.
		auto index = entry.symIndex();
		if (index >= ctx.getDynSym().size()) {
			LOGE("ELF file invalid (r_sym = 0x%x)", (int) index);
		}
		
		// Look up symbol value.
		Addr symVal;
		bool found = getDynSym(symVal, ctx, program, sect, map, index);
		if (!found) {
			auto &sym = ctx.getDynSym()[index];
			LOGE("Link error: Unresolved symbol '%s'", sym.name.c_str());
			return false;
		}
		
		// Calculate relocated address.
		Addr relocAddr = entry.offset + program.vaddr_offset();
		LOGD("Rela:\n  Offs: 0x%x (0x%x)\n  Type: 0x%x\n  Sym:  0x%d\n  Add.: 0x%d",
			(int) entry.offset, (int) relocAddr,
			(int) entry.type(),
			(int) entry.symIndex(),
			(int) entry.addend
		);
		bool res = applyRelocation(ctx, program, entry.type(), symVal, entry.addend, (uint8_t *) relocAddr);
		
		if (!res) return false;
	}
	
	return true;
}

// Apply all relocations for the loaded program.
bool relocate(const ELFFile &ctx, const Program &program, const SymMap &map) {
	if (!ctx.isValid()) return false;
	
	// Iterate sections looking for relocation sections.
	for (auto &sect: ctx.getSect()) {
		if (sect.type == (int) SHT::REL) {
			// Relocation (implicit addend).
			// if (!relocateImplicit(ctx, program, sect)) return false;
			LOGW("TODO");
			return false;
		} else if (sect.type == (int) SHT::RELA) {
			// Relocation (explicit addend).
			if (!relocateExplicit(ctx, program, sect, map)) return false;
		}
	}
	
	return true;
}


// Predicate for exporting le symbolé.
static bool filter(const SymInfo &sym, const SymMap &map) {
	if (sym.section >= 0xff00 || sym.section == 0) {
		return false;
	} else if (!sym.isFunction() && !sym.isObject()) {
		return false;
	} else if (sym.bind() == (int) STB::WEAK) {
		auto iter = map.find(sym.name);
		return iter == map.end();
	} else {
		return sym.bind() == (int) STB::GLOBAL;
	}
}

// Extract symbols from a loaded program into the map.
bool exportSymbols(const ELFFile &ctx, const Program &program, SymMap &map) {
	if (!ctx.isValid()) return false;
	
	// Check against duplication.
	for (auto &sym: ctx.getDynSym()) {
		auto existing = map.find(sym.name);
		if (filter(sym, map) && existing != map.end()) {
			LOGE("Duplicate symbol '%s'", sym.name.c_str());
			LOGD("0x%04x  0x%08x  0x%08x  0x%02x", sym.section, (int) sym.size, (int) sym.value, sym.info);
			return false;
		}
	}
	
	// Copy addresses into the map.
	for (auto &sym: ctx.getDynSym()) {
		if (filter(sym, map)) {
			map[sym.name] = sym.value + program.vaddr_offset();
		}
	}
	
	return true;
}

} // namespace elf
