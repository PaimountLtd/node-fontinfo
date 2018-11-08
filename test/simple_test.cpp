/*
    Copyright (C) 2018  Zachary Lund

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string>
#include <codecvt>
#include <windows.h>
#include <fontinfo/fontinfo.h>
#include <fontinfo/endian.h>

/* Once upon a time, I tried to use C++ standard mechanisms for this
 * but VC++ incorrectly implemented wstring_convert so fuck me I guess */
#ifdef _WIN32

std::string utf16_to_utf8(char *buffer, size_t buffer_length)
{
	DWORD nCodePoints = buffer_length / 2;

	if (buffer_length % 2 != 0) {
		return {};
	}

	DWORD size = WideCharToMultiByte(
		CP_UTF8, 0,
		(WCHAR*)buffer, nCodePoints,
		NULL, 0,
		NULL, NULL
	);

	std::string result(size, char());

	size = WideCharToMultiByte(
		CP_UTF8, 0,
		(WCHAR*)buffer, nCodePoints,
		&result[0], result.size(),
		NULL, NULL
	);

	return result;
}

#else
	#error Not using Windows? Well too bad I guess. I haven't implemented POSIX yet.
#endif

void print_name(struct font_info_string *name, const char *message)
{
	/* The freetype string is UTF-16BE encoded */

	/* We're not guaranteed to control
	 * the memory so make a copy instead. */
	char *buffer = new char[name->length];
	memcpy(buffer, name->buffer, name->length);

	/* Flip from BE to host order */
	char16_t *iter = (char16_t*)&buffer[0];
	char16_t *end_iter = (char16_t*)&buffer[name->length];

	for (; iter != end_iter; ++iter) {
		iter[0] = be16toh(iter[0]);
	}

	std::string converted(utf16_to_utf8(buffer, name->length));

	printf("%s (%u): %s\n", message, name->length, converted.c_str());

	delete [] buffer;
}

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("Usage: %s <font_path>", argv[0]);
		return 1;
	}

	struct font_info *info = font_info_create(argv[1]);

	if (!info) {
		fprintf(stderr, "Failed to fetch font info for %s\n", argv[0]);
		return 1;
	}

	print_name(&info->family_name, "Font Family Name");
	print_name(&info->subfamily_name, "Font Subfamily Name");

	font_info_destroy(info);

	return 0;
}