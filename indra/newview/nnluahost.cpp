#include "stdtypes.h"
#include "llerror.h"
#include "llnotificationmanager.h"
#include "llagentdata.h"

#include "nnluahost.h"

#define LLSD_TYPE_KEY "_LLSD_TYPES"

void NNLuaHost::displayError(const std::string &msg)
{
	LLChat chat;
	chat.mText = msg;
	chat.mFromName = "LuaHost";
	chat.mFromID = gAgentID;
	chat.mOwnerID = gAgentID;
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	chat.mChatType = CHAT_TYPE_DEBUG_MSG;
	LLSD args;
	args["type"] = LLNotificationsUI::NT_NEARBYCHAT;
	LLNotificationsUI::LLNotificationManager::instance().onChat(chat, args);
	LL_WARNS("Lua") << chat.mText << LL_ENDL;
}

void NNLuaHost::popAndDisplayError(void)
{
	std::string msg = toString(-1);
	lua_pop(L, 1);
	displayError(msg);
}

int NNLuaHost::pcall(int nargs, int nret)
{
	int lret = lua_pcall(L, nargs, nret, 0);
	if (lret)
	{
		popAndDisplayError();
	}
	return lret;
}

void NNLuaHost::pushString(const std::string &str)
{
	lua_pushlstring(L, str.data(), str.size());
}

std::string NNLuaHost::toString(int index)
{
	size_t len;
	const char *chars = lua_tolstring(L, index, &len);
	return std::string(chars, len);
}

void NNLuaHost::pushLLSD(const LLSD &llsd)
{
	switch (llsd.type())
	{
		case LLSD::TypeUndefined:
		{
			lua_pushnil(L);
			break;
		}
		case LLSD::TypeBoolean:
		{
			lua_pushboolean(L, llsd.asBoolean());
			break;
		}
		case LLSD::TypeInteger:
		{
			lua_pushinteger(L, llsd.asInteger());
			break;
		}
		case LLSD::TypeReal:
		{
			lua_pushnumber(L, llsd.asReal());
			break;
		}
		case LLSD::TypeString:
		case LLSD::TypeUUID: // FIXME: uuids need their own type
		case LLSD::TypeDate: // FIXME: dates need their own type
		case LLSD::TypeURI: // FIXME: URIs need their own type
		{
			pushString(llsd.asString());
			break;
		}
		case LLSD::TypeBinary:
		{
			LLSD::Binary v = llsd.asBinary();
			void *data = lua_newuserdata(L, v.size());
			memcpy(data, &(v[0]), v.size());
			break;
		}
		case LLSD::TypeMap:
		{
			LLSD::map_const_iterator it = llsd.beginMap();
			LLSD::map_const_iterator stop = llsd.endMap();
			lua_createtable(L, 0, llsd.size() + 1);
			lua_pushstring(L, LLSD_TYPE_KEY);
			lua_createtable(L, 0, llsd.size());
			while (it != stop)
			{
				// table, typekey, typetable
				pushString(it->first);
				// table, typekey, typetable, key
				lua_pushvalue(L, -1);
				// table, typekey, typetable, key, key
				pushLLSD(it->second);
				// table, typekey, typetable, key, key, value
				lua_rawset(L, -6);
				// table, typekey, typetable, key
				lua_pushinteger(L, it->second.type());
				// table, typekey, typetable, key, type
				lua_rawset(L, -3);
				it++;
			}
			// table, typekey, typetable
			lua_rawset(L, -3);
			// table
			break;
		}
		case LLSD::TypeArray:
		{
			LLSD::array_const_iterator it = llsd.beginArray();
			LLSD::array_const_iterator stop = llsd.endArray();
			int i = 1; // lua arrays are 1-indexed
			lua_createtable(L, llsd.size(), 1);
			lua_pushstring(L, LLSD_TYPE_KEY);
			lua_createtable(L, llsd.size(), 0);
			while (it != stop)
			{
				// array, typekey, typetable
				lua_pushinteger(L, i++);
				// array, typekey, typetable, index
				lua_pushvalue(L, -1);
				// array, typekey, typetable, index, index
				LLSD d = *it++;
				pushLLSD(d);
				// array, typekey, typetable, index, index, value
				lua_rawset(L, -6);
				// array, typekey, typetable, index
				lua_pushinteger(L, d.type());
				// array, typekey, typetable, index, type
				lua_rawset(L, -3);
			}
			// array, typekey, typetable
			lua_rawset(L, -3);
			// table
			break;
		}
	}
}

bool NNLuaHost::isinteger(int index)
{
	return fabs(lua_tonumber(L, index) - lua_tointeger(L, index)) < 0.000001;
}

// the type is guessed when type is Undefined
LLSD NNLuaHost::toLLSD(LLSD::Type type)
{
	LLSD ret;

	// nothing to pop. should this be an error?
	if (lua_gettop(L) == 0)
	{
		LL_WARNS("Lua") << "Trying to pop LLSD from an empty stack." << LL_ENDL;
		return LLSD();
	}

	// Guess type
	if (type == LLSD::TypeUndefined)
	{
		switch (lua_type(L, -1))
		{
			case LUA_TNUMBER:
			{
				if (isinteger(-1))
				{
					ret = lua_tointeger(L, -1);
				}
				else
				{
					ret = lua_tonumber(L, -1);
				}
				break;
			}
			case LUA_TBOOLEAN:
			{
				type = LLSD::TypeBoolean;
				break;
			}
			case LUA_TSTRING:
			{
				type = LLSD::TypeString;
				break;
			}
			case LUA_TTABLE:
			{
				bool allIntegers = TRUE;
				bool empty = TRUE;
				lua_pushnil(L);
				while (allIntegers && lua_next(L, -1) != 0)
				{
					// table, key, value
					lua_pop(L, 1);
					// table, key
					// we don't care about the type table here
					int ltype = lua_type(L, -1);
					if (ltype == LUA_TSTRING && toString(-1) == LLSD_TYPE_KEY)
					{
						continue;
					}
					empty = FALSE;
					if (ltype != LUA_TNUMBER || !isinteger(-1))
					{
						allIntegers = FALSE;
					}
				}
				lua_pop(L, 1);
				if (empty || !allIntegers)
				{
					type = LLSD::TypeMap;
				}
				else
				{
					type = LLSD::TypeArray;
				}
				break;
			}
			case LUA_TUSERDATA:
			{
				type = LLSD::TypeBinary;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	switch (type)
	{
		case LLSD::TypeUndefined:
		{
			// the value is already set(either undefined or it was set when guessed)
			break;
		}
		case LLSD::TypeBoolean:
		{
			ret = lua_toboolean(L, -1);
			break;
		}
		case LLSD::TypeInteger:
		{
			ret = lua_tointeger(L, -1);
			break;
		}
		case LLSD::TypeReal:
		{
			ret = lua_tonumber(L, -1);
			break;
		}
		case LLSD::TypeString:
		{
			ret = toString(-1);
			break;
		}
		case LLSD::TypeUUID:
		{
			ret = LLSD::UUID(toString(-1));
			break;
		}
		case LLSD::TypeDate:
		{
			ret = LLSD::Date(toString(-1));
			break;
		}
		case LLSD::TypeURI:
		{
			ret = LLSD::URI(toString(-1));
			break;
		}
		case LLSD::TypeBinary:
		{
			void *data = lua_touserdata(L, -1);
			LLSD::Binary v;
			size_t len = lua_objlen(L, -1);
			v.resize(len);
			memcpy(&(v[0]), data, len);
			ret = v;
			break;
		}
		case LLSD::TypeMap:
		{
			ret = LLSD::emptyMap();

			pushString(LLSD_TYPE_KEY);
			// table, typekey
			lua_gettable(L, -2);
			// table, typetable
			bool hasTypeTable = lua_istable(L, -1);

			lua_pushnil(L);
			// table, typetable, nil
			while (lua_next(L, -3) != 0)
			{
				// table, typetable, key, value
				std::string key;
				// toString when the object is not a string will break lua_next
				if (lua_type(L, -2) != LUA_TSTRING)
				{
					lua_pushvalue(L, -2);
					// table, typetable, key, value, key
					key = toString(-1);
					lua_pop(L, 1);
					// table, typetable, key, value
				}
				else
				{
					key = toString(-2);
				}

				if (key == LLSD_TYPE_KEY)
				{
					lua_pop(L, 1);
					// table, typetable, key
					continue;
				}

				LLSD::Type vtype = LLSD::TypeUndefined;
				if (hasTypeTable)
				{
					lua_pushvalue(L, -2);
					// table, typetable, key, value, key
					lua_gettable(L, -4);
					// table, typetable, key, value, type
					if (!lua_isnil(L, -1))
					{
						vtype = (LLSD::Type)lua_tointeger(L, -1);
					}
					lua_pop(L, 1);
					// table, typetable, key, value
				}

				ret.insert(key, toLLSD(vtype));

				lua_pop(L, 1);
				// table, typetable, key
			}
			// table, typetable (lua_next popped the key)
			lua_pop(L, 1);
			// table
			break;
		}
		case LLSD::TypeArray:
		{
			ret = LLSD::emptyArray();

			pushString(LLSD_TYPE_KEY);
			// table, typekey
			lua_gettable(L, -2);
			// table, typetable
			bool hasTypeTable = lua_istable(L, -1);

			lua_pushnil(L);
			// table, typetable, nil
			while (lua_next(L, -3) != 0)
			{
				// table, typetable, index, value
				if (lua_type(L, -2) == LUA_TSTRING && toString(-2) == LLSD_TYPE_KEY)
				{
					lua_pop(L, 1);
					continue;
				}

				LLSD::Type vtype = LLSD::TypeUndefined;
				if (hasTypeTable)
				{
					lua_pushvalue(L, -2);
					// table, typetable, index, value, index
					lua_gettable(L, -4);
					// table, typetable, index, value, type
					if (!lua_isnil(L, -1))
					{
						vtype = (LLSD::Type)lua_tointeger(L, -1);
					}
					lua_pop(L, 1);
					// table, typetable, index, value
				}

				int index = lua_tointeger(L, -2);
				ret.set(index - 1, toLLSD(vtype)); // lua arrays are 1-indexed

				lua_pop(L, 1);
				// table, typetable, index
			}
			// table, typetable (lua_next popped the index)
			lua_pop(L, 1);
			// table
			break;
		}
		default:
		{
			displayError("Invalid type code " + type);
			break;
		}
	}
	return ret;
}

NNLuaHost::NNLuaHost(void)
{
	L = NULL;
}

void NNLuaHost::start(void)
{
	L = lua_open();
	luaL_openlibs(L);
	pushString(LLSD_TYPE_KEY);
	lua_setglobal(L, "LLSD_TYPE_KEY");
}

void NNLuaHost::stop(void)
{
	lua_close(L);
	L = NULL;
}

void NNLuaHost::advance(void)
{
	if (L)
	{
		lua_getglobal(L, "advance");
		if (lua_isfunction(L, -1))
		{
			pcall(0, 0);
		}
		else
		{
			lua_pop(L, 1);
		}
	}
}

LLChat NNLuaHost::filterChat(const LLChat &chat, const LLSD &args)
{
	if (L)
	{
		lua_getglobal(L, "filter_chat");
		if (lua_isfunction(L, -1))
		{
			pushLLSD(chat.asLLSD());
			pushLLSD(args);
			pcall(2, 1);
			LLChat ret;
			ret.assign(toLLSD(LLSD::TypeMap));
			lua_pop(L, 1);
			return ret;
		}
	}
	return LLChat(chat);
}

void NNLuaHost::eval(const std::string &code)
{
	int lret = luaL_loadbuffer(L, code.data(), code.size(), "eval");

	if (!lret)
	{
		pcall(0, 0);
	}
	else
	{
		popAndDisplayError();
	}
}
