/**
 * @file nnluahost.h
 * @brief LuaHost declaration
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Kokua Viewer Source Code
 * Copyright (C) 2011, nemurimasu (at) gmail.com
 * for the Kokua Viewer Team.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * $/LicenseInfo$
 */

#ifndef NN_NNLUAHOST_H
#define NN_NNLUAHOST_H

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "llsingleton.h"
#include "llsd.h"
#include "llchat.h"

class NNLuaHost: public LLSingleton<NNLuaHost>
{
public:
	void start(void);
	void stop(void);
	void advance(void);
	void eval(const std::string &);
	NNLuaHost(void);
	LLChat filterChat(const LLChat &, const LLSD &);
private:
	lua_State *L;
	int pcall(int, int);
	void displayError(const std::string &);
	void popAndDisplayError(void);
	void pushLLSD(const LLSD &);
	LLSD toLLSD(LLSD::Type t = LLSD::TypeUndefined);
	bool isinteger(int);
	std::string toString(int);
	void pushString(const std::string &str);
};

#endif
