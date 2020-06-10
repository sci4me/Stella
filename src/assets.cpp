#define _TEXTURES_ITEM_X(X, id, name, placeable, pfx, texname) X(id, texname, false, item, pfx)
#define _TEXTURES_TILE_X(X, id, texname) X(id, texname, true, tile, tile)
#define _TEXTURES(X) \
    _ITEM_DEFS(_TEXTURES_ITEM_X, X) \
    _TILE_DEFS(_TEXTURES_TILE_X, X)


#define _ANCILLARY_TEXTURES(X) \
    X(TEX_TUBE_NORTH,       tile,       tube_north,        true  )


typedef u8 Ancillary_Texture_ID;
enum Ancillary_Texture_ID_ : Ancillary_Texture_ID {
    #define _X(id, pfx, texname, mips) id,
    _ANCILLARY_TEXTURES(_X)
    #undef _X

    N_ANCILLARY_TEXTURES
};


struct Assets {
    Texture tile_textures[N_TILE_TYPES];
    Texture item_textures[N_ITEM_TYPES];
    Texture ancillary_textures[N_ANCILLARY_TEXTURES];

    void init() {
        #define _X(id, name, mips, type, pfx) type##_textures[id] = load_texture_from_file(#pfx, #name, mips);
        _TEXTURES(_X)
        #undef _X

        #define _X(id, pfx, texname, mips) ancillary_textures[id] = load_texture_from_file(#pfx, #texname, mips);
        _ANCILLARY_TEXTURES(_X)
        #undef _X
    }

    void deinit() {
        #define _X(id, name, mips, type, pfx) type##_textures[id].deinit();
        _TEXTURES(_X)
        #undef _X

        #define _X(id, pfx, texname, mips) ancillary_textures[id].deinit();
        _ANCILLARY_TEXTURES(_X)
        #undef _X
    }
};


#undef _TEXTURES_ITEM_X
#undef _TEXTURES_TILE_X
#undef _TEXTURES
#undef _ANCILLARY_TEXTURES