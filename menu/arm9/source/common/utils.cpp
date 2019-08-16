/*
 *   This file is part of PKSM
 *   Copyright (C) 2016-2019 Bernardo Giordano, Admiral Fish, piepie62
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
 *       * Requiring preservation of specified reasonable legal notices or
 *         author attributions in that material or in the Appropriate Legal
 *         Notices displayed by works containing it.
 *       * Prohibiting misrepresentation of the origin of that material,
 *         or requiring that modified versions of such material be marked in
 *         reasonable ways as different from the original version.
 */

#include "utils.hpp"
#include <algorithm>
#include <map>
#include <queue>
#include <vector>

static std::string utf16DataToUtf8(const char16_t* data, size_t size, char16_t delim = 0) {
	std::string ret;
	char addChar[4] = {0};
	for(size_t i = 0; i < size; i++) {
		if(data[i] == delim) {
			return ret;
		} else if(data[i] < 0x0080) {
			addChar[0] = data[i];
			addChar[1] = '\0';
		} else if(data[i] < 0x0800) {
			addChar[0] = 0xC0 | ((data[i] >> 6) & 0x1F);
			addChar[1] = 0x80 | (data[i] & 0x3F);
			addChar[2] = '\0';
		} else {
			addChar[0] = 0xE0 | ((data[i] >> 12) & 0x0F);
			addChar[1] = 0x80 | ((data[i] >> 6) & 0x3F);
			addChar[2] = 0x80 | (data[i] & 0x3F);
			addChar[3] = '\0';
		}
		ret.append(addChar);
	}
	return ret;
}

std::string StringUtils::UTF16toUTF8(const std::u16string& src) {
	return utf16DataToUtf8(src.data(), src.size());
}