list(APPEND WebCore_PRIVATE_INCLUDE_DIRECTORIES
    "${WEBCORE_DIR}/platform/graphics/texmap"
)

list(APPEND WebCore_SOURCES
    platform/graphics/qt/texmap/BitmapTexture.cpp
    platform/graphics/qt/texmap/BitmapTexturePool.cpp
    platform/graphics/qt/texmap/GraphicsLayerTextureMapper.cpp
    platform/graphics/qt/texmap/NicosiaAnimation.cpp
    platform/graphics/qt/texmap/TextureMapper.cpp
    platform/graphics/qt/texmap/TextureMapperBackingStore.cpp
    platform/graphics/qt/texmap/TextureMapperFPSCounter.cpp
    platform/graphics/qt/texmap/TextureMapperLayer.cpp
    platform/graphics/qt/texmap/TextureMapperTile.cpp
    platform/graphics/qt/texmap/TextureMapperTiledBackingStore.cpp
)

list(APPEND WebCore_PRIVATE_FRAMEWORK_HEADERS
    platform/graphics/qt/texmap/BitmapTexture.h
    platform/graphics/qt/texmap/GraphicsLayerTextureMapper.h
    platform/graphics/qt/texmap/NicosiaAnimation.h
    platform/graphics/qt/texmap/TextureMapper.h
    platform/graphics/qt/texmap/TextureMapperBackingStore.h
    platform/graphics/qt/texmap/TextureMapperFPSCounter.h
    platform/graphics/qt/texmap/TextureMapperLayer.h
    platform/graphics/qt/texmap/TextureMapperPlatformLayer.h
    platform/graphics/qt/texmap/TextureMapperSolidColorLayer.h
    platform/graphics/qt/texmap/TextureMapperTile.h
    platform/graphics/qt/texmap/TextureMapperTiledBackingStore.h
)
