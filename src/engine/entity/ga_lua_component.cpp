/*
** RPI Game Architecture Engine
**
** Portions adapted from:
** Viper Engine - Copyright (C) 2016 Velan Studios - All Rights Reserved
**
** This file is distributed under the MIT License. See LICENSE.txt.
*/

#include "ga_lua_component.h"
#include "entity/ga_entity.h"

#include "framework/ga_frame_params.h"

#include <iostream>

extern "C" 
{
	static int component_get_entity(lua_State *L) 
	{
		ga_lua_component* comp = (ga_lua_component*)lua_topointer(L, 1);
		lua_pushlightuserdata(L, comp->get_entity());
		return 1;
	}

	static int entity_translate(lua_State *L) 
	{
		ga_entity* ent = (ga_entity*)lua_topointer(L, 1);
		float x = (float)lua_tonumber(L, 2);
		float y = (float)lua_tonumber(L, 3);
		float z = (float)lua_tonumber(L, 4);
		ent->translate({ x, y, z });
		return 1;
	}

	static int frame_params_get_input_left(lua_State *L) 
	{
		ga_frame_params* params = (ga_frame_params*)lua_topointer(L, 1);
		if (params->_button_mask & k_button_k)
		{
			lua_pushboolean(L, true);
		}
		else
		{
			lua_pushboolean(L, false);
		}
		
		return 1;
	}

	static int frame_params_get_input_right(lua_State *L) 
	{
		ga_frame_params* params = (ga_frame_params*)lua_topointer(L, 1);
		if (params->_button_mask & k_button_l)
		{
			lua_pushboolean(L, true);
		}
		else
		{
			lua_pushboolean(L, false);
		}
		
		return 1;
	}
}

ga_lua_component::ga_lua_component(ga_entity* ent, const char* path) : ga_component(ent)
{
	// TODO: Homework 7
	_lua = luaL_newstate();
	luaL_openlibs(_lua);

	extern char g_root_path[256];
	std::string fullpath = g_root_path;
	fullpath += path;

	int status = luaL_loadfile(_lua, fullpath.c_str());
	if (status)
	{
		std::cerr << "Failed to load script " << path << ": " << lua_tostring(_lua, -1);
		lua_close(_lua);
	}

	if (lua_pcall(_lua, 0, LUA_MULTRET, 0)) {
		std::cerr << "Failed execution" << std::endl;
		std::cerr << lua_tostring(_lua, -1) << std::endl;
		lua_pop(_lua, 1);
	}

	lua_pushcfunction(_lua, component_get_entity);
	lua_setglobal(_lua, "component_get_entity");

	lua_pushcfunction(_lua, entity_translate);
	lua_setglobal(_lua, "entity_translate");

	lua_pushcfunction(_lua, frame_params_get_input_left);
	lua_setglobal(_lua, "frame_params_get_input_left");

	lua_pushcfunction(_lua, frame_params_get_input_right);
	lua_setglobal(_lua, "frame_params_get_input_right");
}

ga_lua_component::~ga_lua_component()
{
	// TODO: Homework 7
	delete _lua;
}

void ga_lua_component::update(ga_frame_params* params)
{
	// TODO: Homework 7

	lua_getglobal(_lua, "update");
	lua_pushlightuserdata(_lua, this);
	lua_pushlightuserdata(_lua, params);
	lua_pcall(_lua, 2, 0, 0);

	lua_pop(_lua, 1);
}
