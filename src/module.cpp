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

Napi::String StringFromFontString(Napi::Env env, font_info_string *name)
{
	if (name->length == 0 || name->buffer == NULL)
		return Napi::String::New(env, "");

	int buffer_length = name->length / 2;
	uint16_t *buffer = new uint16_t[buffer_length];
	memcpy(buffer, name->buffer, name->length);

	/* Flip from BE to host order */
	for (int i = 0; i < name->length / 2; ++i) {
		buffer[i] = be16toh(((uint16_t*)name->buffer)[i]);
	}

	Napi::String result =
		Napi::String::New(env, name->buffer, buffer_length);

	delete[] buffer;

	return result;
}

void SetByString(Napi::Object object, const char* key, Napi::Value value)
{
	object.Set(key, value);
}

Napi::Value getFontInfo(const Napi::CallbackInfo& info) {
	if (!info[0].IsString()) {
		Napi::Error::New(info.Env(), Napi::String::New(info.Env(), "getFontInfo: Argument 1 - expected string"));
		return info.Env().Undefined();
	}

	Napi::String filepath = info[0].ToString();

	font_info *f_info = font_info_create(filepath.Utf8Value().c_str());

	if (!f_info) {
		fprintf(stderr, "node-fontinfo: Failed to fetch font info %s\n", filepath.Utf8Value().c_str());
		return info.Env().Undefined();
	}

	if (f_info->family_name.length == 0 || f_info->family_name.buffer == NULL) {
		font_info_destroy(f_info);
		fprintf(stderr, "node-fontinfo: font info for %s does not have a family name\n", filepath.Utf8Value().c_str());
		return info.Env().Undefined();
	}

	Napi::Object result = Napi::Object::New(info.Env());
	result.Set("family_name", StringFromFontString(info.Env(), &f_info->family_name));
	result.Set("subfamily_name", StringFromFontString(info.Env(), &f_info->subfamily_name));
	result.Set("italic", Napi::Boolean::New(info.Env(), f_info->italic));
	result.Set("bold", Napi::Boolean::New(info.Env(), f_info->bold));

	font_info_destroy(f_info);

	return result;
}

Napi::Object main_node(Napi::Env env, Napi::Object exports) {
	exports.Set(
		Napi::String::New(env, "getFontInfo"),
		Napi::Function::New(env, getFontInfo));
    return exports;
}

NODE_API_MODULE(node_fontinfo, main_node)