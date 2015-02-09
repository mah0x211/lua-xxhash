package = "xxhash"
version = "scm-1"
source = {
    url = "git://github.com/mah0x211/lua-xxhash.git"
}
description = {
    summary = "xxhash binding",
    homepage = "https://github.com/mah0x211/lua-xxhash", 
    license = "MIT/X11",
    maintainer = "Masatoshi Teruya"
}
dependencies = {
    "lua >= 5.1"
}
build = {
    type = "builtin",
    modules = {
        xxhash = {
            sources = {
                "src/xxhash_bind.c",
                "src/xxhash.c"
            }
        }
    }
}

