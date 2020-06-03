#define _TEXTURES_ITEM_X(X, id, name, placeable, pfx, texname) X(id, texname, false, item, pfx)
#define _TEXTURES_TILE_X(X, id, texname) X(id, texname, true, tile, tile)
#define _TEXTURES(X) \
    _TILE_DEFS(_TEXTURES_TILE_X, X) \
    _ITEM_DEFS(_TEXTURES_ITEM_X, X)


struct Assets {
    Texture tile_textures[N_TILE_TYPES];
    Texture item_textures[N_ITEM_TYPES];

    void init() {
        #define _X(id, name, mips, type, pfx) type##_textures[id] = load_texture_from_file(#pfx, #name, mips);
        _TEXTURES(_X)
        #undef _X
    }

    void deinit() {
        #define _X(id, name, mips, type, pfx) type##_textures[id].deinit();
        _TEXTURES(_X)
        #undef _X
    }
};