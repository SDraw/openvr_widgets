#pragma once

// Specific class for working with textures that aren't static during application runtime
// Refer to https://github.com/ValveSoftware/openvr/issues/1255 for more information
class TexturePooler
{
    static sf::Texture *ms_lastTexture;
    static sf::RenderTexture *ms_lastRenderTexture;
public:
    static sf::Texture* CreateTexture(unsigned int f_width, unsigned int f_height);
    static void DeleteTexture(sf::Texture *f_texture);

    static sf::RenderTexture* CreateRenderTexture(unsigned int f_width, unsigned int f_height);
    static void DeleteRenderTexture(sf::RenderTexture *f_rt);
};
