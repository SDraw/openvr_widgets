#include "stdafx.h"
#include "Utils/TexturePooler.h"

sf::Texture *TexturePooler::ms_lastTexture = nullptr;
sf::RenderTexture *TexturePooler::ms_lastRenderTexture = nullptr;

sf::Texture* TexturePooler::CreateTexture(unsigned int f_width, unsigned int f_height)
{
    sf::Texture *l_texture = new sf::Texture();
    l_texture->create(f_width, f_height);
    if(ms_lastTexture)
    {
        delete ms_lastTexture;
        ms_lastTexture = nullptr;
    }
    return l_texture;
}
void TexturePooler::DeleteTexture(sf::Texture *f_texture)
{
    if(ms_lastTexture) delete ms_lastTexture;
    ms_lastTexture = f_texture;
}

sf::RenderTexture* TexturePooler::CreateRenderTexture(unsigned int f_width, unsigned int f_height)
{
    sf::RenderTexture *l_rt = new sf::RenderTexture();
    l_rt->create(f_width, f_height);
    if(ms_lastRenderTexture)
    {
        delete ms_lastRenderTexture;
        ms_lastRenderTexture = nullptr;
    }
    return l_rt;
}
void TexturePooler::DeleteRenderTexture(sf::RenderTexture *f_rt)
{
    if(ms_lastRenderTexture) delete ms_lastRenderTexture;
    ms_lastRenderTexture = f_rt;
}
