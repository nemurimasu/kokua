#ifndef NN_NNLUAHOST_H
#define NN_NNLUAHOST_H

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "llsingleton.h"
#include "llsd.h"

class NNLuaHost: public LLSingleton<NNLuaHost>
{
public:
	void start(void);
	void stop(void);
	void advance(void);
	void eval(const std::string &);
	NNLuaHost(void);
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
