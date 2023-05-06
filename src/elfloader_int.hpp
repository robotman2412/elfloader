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

// #define ELFLOADER_LINENUMBERS

#define lololol__FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#ifdef ELFLOADER_LINENUMBERS
#define LOG_LNFMT "%s:%d: "
#define LOG_LNARG , lololol__FILENAME__, __LINE__
#else
#define LOG_LNFMT ""
#define LOG_LNARG
#endif

#ifdef ESP_PLATFORM
#include <esp_log.h>
static const char *TAG = "elfloader";
#define LOGE(fmt, ...) ESP_LOGE(TAG, LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGI(fmt, ...) ESP_LOGI(TAG, LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGW(fmt, ...) ESP_LOGW(TAG, LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGD(fmt, ...) ESP_LOGD(TAG, LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#else
static void _elf_log_helper(const char *prefix, const char *fmt, ...) {
	fputs(prefix, stdout);
	va_list ls;
	va_start(ls, fmt);
	vprintf(fmt, ls);
	va_end(ls);
	fputs("\033[0m", stdout);
}
#define LOGE(fmt, ...) _elf_log_helper("\033[31mErr  elfloader: ", LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGI(fmt, ...) _elf_log_helper( "\033[0mInfo elfloader: ", LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGW(fmt, ...) _elf_log_helper("\033[33mWarn elfloader: ", LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#define LOGD(fmt, ...) _elf_log_helper("\033[34mDeb  elfloader: ", LOG_LNFMT fmt LOG_LNARG __VA_OPT__(,) __VA_ARGS__)
#endif



// A magic tester for multi boits
static bool _elf_expect(FILE *fd, size_t len, const void *magic) {
	uint8_t tmp[len];
	size_t read = fread(tmp, 1, len, fd);
	if (read != len) return false;
	return !memcmp(tmp, magic, len);
}
#define EXPECT(count, magic) do { errno = 0; if (!_elf_expect(fd, count, magic)) {LOGE("I/O error: %s", strerror(errno)); return false;} } while(0)

// A INTEGER READING DEVICE
static bool _elf_readint(FILE *fd, uint64_t *out, size_t size, bool is_signed, bool is_le) {
	uint64_t tmp = 0;
	
	if (is_le) {
		for (size_t i = 0; i < size; i++) {
			int c = fgetc(fd);
			if (c == EOF) return false;
			tmp |= (uint64_t) (c & 0xff) << (i*8);
		}
	} else {
		for (size_t i = 0; i < size; i++) {
			int c = fgetc(fd);
			if (c == EOF) return false;
			tmp |= c << ((size-i-1)*8);
		}
	}
	
	if (is_signed && (tmp >> (size*8-1))) {
		for (size_t i = size; i < 8; i++) {
			tmp |= 0xffllu << (i*8);
		}
	}
	
	*out = tmp;
	return true;
}
#define READUINT(dest, size) do { errno = 0; uint64_t tmp; if (!_elf_readint(fd, &tmp, size, false, is_little_endian)) {LOGE("I/O error: %s", strerror(errno)); return false;} (dest) = tmp; } while(0)

// SKIPS PADDING.
static bool _elf_skip(FILE *fd, size_t len) {
	long pre = ftell(fd);
	fseek(fd, len, SEEK_CUR);
	return ftell(fd) - pre == len;
}
#define SKIP(size) do { errno = 0; if (!_elf_skip(fd, (size))) {LOGE("I/O error: %s", strerror(errno)); return false;} } while(0)

// SEEKD LOKATON.
static bool _elf_seek(FILE *fd, size_t idx) {
	fseek(fd, idx, SEEK_SET);
	return ftell(fd) == idx;
}
#define SEEK(idx) do { errno = 0; if (!_elf_seek(fd, (idx))) {LOGE("I/O error: %s", strerror(errno)); return false;} } while(0)

// READ BIANIER.
#define READ(ptr, len) do { auto tmplen = (len); errno = 0; if (fread(ptr, 1, tmplen, fd) != tmplen) {LOGE("I/O error: %s", strerror(errno)); return false;} } while(0)
