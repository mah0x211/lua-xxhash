/*
 *  Copyright (C) 2013 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */
/*
 *  xxhash_bind.c
 *  Created by Masatoshi Teruya on 13/05/24.
 */

#include <stdlib.h>
#include <errno.h>
#include <lauxlib.h>
#include <lualib.h>
#include "xxhash.h"

#define XXHASH_LUA "xxhash"
typedef struct {
    void *state;
    unsigned int seed;
} xxhash_t;


static int xxh32_lua( lua_State *L )
{
    size_t len = 0;
    const char *data = luaL_checklstring( L, 1, &len );
    unsigned int seed = luaL_checkinteger( L, 2 );
    unsigned int hash = XXH32( data, len, seed );
    
    lua_pushinteger( L, hash );
    
    return 1;
}

static int init_lua( lua_State *L )
{
    unsigned int seed = luaL_checkinteger( L, 1 );
    xxhash_t *xh = lua_newuserdata( L, sizeof( xxhash_t ) );
    
    if( !xh ){
        return luaL_error( L, "failed to init(): %s", lua_tostring( L, -1 ) );
    }
    xh->seed = seed;
    xh->state = XXH32_init( seed );
    luaL_getmetatable( L, XXHASH_LUA );
    lua_setmetatable( L, -2 );
    
    return 1;
}

static int update_lua( lua_State *L )
{
    xxhash_t *xh = luaL_checkudata( L, 1, XXHASH_LUA );
    size_t len = 0;
    const char *data = luaL_checklstring( L, 2, &len );
    
    XXH32_update( xh->state, data, len );
    
    return 0;
}

static int digest_lua( lua_State *L )
{
    xxhash_t *xh = luaL_checkudata( L, 1, XXHASH_LUA );
    // XXX: must not use XXH32_digest because GC will deallocate state.
    unsigned int hash = XXH32_intermediateDigest( xh->state );
    
    lua_pushinteger( L, hash );
    
    return 1;
}

static int reset_lua( lua_State *L )
{
    xxhash_t *xh = luaL_checkudata( L, 1, XXHASH_LUA );
    
    if( lua_gettop( L ) > 1 ){
        xh->seed = luaL_checkinteger( L, 2 );
    }
    
    XXH32_resetState( xh->state, xh->seed );
    
    return 0;
}

static int tostring_lua( lua_State *L )
{
    xxhash_t *xh = lua_touserdata( L, 1 );
    
    lua_pushfstring( L, XXHASH_LUA ": %p", xh );
    
    return 1;
}

static int gc_lua( lua_State *L )
{
    xxhash_t *xh = lua_touserdata( L, 1 );
    
    free( xh->state );
    
    return 0;
}

LUALIB_API int luaopen_xxhash( lua_State *L )
{
    struct luaL_Reg fnReg[] = {
        { "xxh32", xxh32_lua },
        { "init", init_lua },
        { NULL, NULL }
    };
    int i = 0;
    int top = lua_gettop( L );
    
    // metatable for userdata
    luaL_newmetatable( L, XXHASH_LUA );
    lua_pushstring( L, "__gc" );
    lua_pushcfunction( L, gc_lua );
    lua_rawset( L, -3 );
    // add name
    lua_pushstring( L, "__tostring" );
    lua_pushcfunction( L, tostring_lua );
    lua_rawset( L, -3 );
    // add method
    lua_pushstring( L, "__index" );
    lua_newtable( L );
    lua_pushstring( L, "update" );
    lua_pushcfunction( L, update_lua );
    lua_rawset( L, -3 );
    lua_pushstring( L, "digest" );
    lua_pushcfunction( L, digest_lua );
    lua_rawset( L, -3 );
    lua_pushstring( L, "reset" );
    lua_pushcfunction( L, reset_lua );
    lua_rawset( L, -3 );

    lua_rawset( L, -3 );
    lua_settop( L, top );
    
    // exports
    lua_newtable( L );
    for( i = 0; fnReg[i].name; i++ ){ 
        lua_pushstring( L, fnReg[i].name );
        lua_pushcfunction( L, fnReg[i].func );
        lua_rawset( L, -3 );
    }
    
    return 1;
}

