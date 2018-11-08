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

#include <napi.h>
#include <cstdio>
#include "fontinfo/fontinfo.h"
#include "fontinfo/endian.h"

using namespace Napi;

namespace {

String StringFromFontString(Env env, font_info_string *name)
{
	char16_t *buffer = new char16_t[name->length];
	memcpy(buffer, name->buffer, name->length);

	/* Flip from BE to host order */
	char16_t *end_iter = (char16_t*)&name->buffer[name->length];
	char16_t *buffer_iter = buffer;

	for (char16_t *iter = (char16_t*)name->buffer; iter != end_iter; ++iter) {
		*buffer_iter = be16toh(iter[0]);

		++buffer_iter;
	}

	String result = String::New(env, buffer, name->length);

	delete[] buffer;

	return result;
}

static Value getFontInfo(const CallbackInfo& info)
{
	Env env = info.Env();
	std::string filepath(info[0].As<String>());

	font_info *f_info = font_info_create(filepath.c_str());

	if (!f_info) {
		throw Error::New(env, "node-fontinfo: Failed to fetch font info\n");
	}

	/* We now have UTF16 and the size of the buffer, let V8 deal with the rest */
	Object result = Object::New(env);

	result.Set(
		"family_name",
		StringFromFontString(env, &f_info->family_name)
	);

	result.Set(
		"subfamily_name",
		StringFromFontString(env, &f_info->subfamily_name)
	);

	result.Set("italic", f_info->italic);
	result.Set("bold", f_info->bold);

	font_info_destroy(f_info);

	return result;
}

}

Object Init(Env env, Object exports)
{
	exports.Set(
		"getFontInfo",
		Napi::Function::New(env, getFontInfo)
	);

	return exports;
}

NODE_API_MODULE(node_fontinfo, Init)