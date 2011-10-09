/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2011 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

/**
 *  \file SDL_render.h
 *  
 *  Header file for SDL 2D rendering functions.
 *
 *  This API supports the following features:
 *      * single pixel points
 *      * single pixel lines
 *      * filled rectangles
 *      * texture images
 *
 *  The primitives may be drawn in opaque, blended, or additive modes.
 *
 *  The texture images may be drawn in opaque, blended, or additive modes.
 *  They can have an additional color tint or alpha modulation applied to
 *  them, and may also be stretched with linear interpolation.
 *
 *  This API is designed to accelerate simple 2D operations. You may
 *  want more functionality such as rotation and particle effects and
 *  in that case you should use SDL's OpenGL/Direct3D support or one
 *  of the many good 3D engines.
 */

#ifndef _SDL_render_h
#define _SDL_render_h

#include "SDL_stdinc.h"
#include "SDL_rect.h"
#include "SDL_video.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
extern "C" {
/* *INDENT-ON* */
#endif

/**
 *  \brief Flags used when creating a rendering context
 */
typedef enum
{
    SDL_RENDERER_SOFTWARE = 0x00000001,         /**< The renderer is a software fallback */ 
    SDL_RENDERER_ACCELERATED = 0x00000002,      /**< The renderer uses hardware 
                                                     acceleration */
    SDL_RENDERER_PRESENTVSYNC = 0x00000004      /**< Present is synchronized 
                                                     with the refresh rate */
} SDL_RendererFlags;

/**
 *  \brief Information on the capabilities of a render driver or context.
 */
typedef struct SDL_RendererInfo
{
    const char *name;           /**< The name of the renderer */
    Uint32 flags;               /**< Supported ::SDL_RendererFlags */
    Uint32 num_texture_formats; /**< The number of available texture formats */
    Uint32 texture_formats[16]; /**< The available texture formats */
    int max_texture_width;      /**< The maximimum texture width */
    int max_texture_height;     /**< The maximimum texture height */
} SDL_RendererInfo;

/**
 *  \brief The access pattern allowed for a texture.
 */
typedef enum
{
    SDL_TEXTUREACCESS_STATIC,    /**< Changes rarely, not lockable */
    SDL_TEXTUREACCESS_STREAMING  /**< Changes frequently, lockable */
} SDL_TextureAccess;

/**
 *  \brief The texture channel modulation used in SDL_RenderCopy().
 */
typedef enum
{
    SDL_TEXTUREMODULATE_NONE = 0x00000000,     /**< No modulation */
    SDL_TEXTUREMODULATE_COLOR = 0x00000001,    /**< srcC = srcC * color */
    SDL_TEXTUREMODULATE_ALPHA = 0x00000002     /**< srcA = srcA * alpha */
} SDL_TextureModulate;

/**
 *  \brief A structure representing rendering state
 */
struct SDL_Renderer;
typedef struct SDL_Renderer SDL_Renderer;

/**
 *  \brief An efficient driver-specific representation of pixel data
 */
struct SDL_Texture;
typedef struct SDL_Texture SDL_Texture;


/* Function prototypes */

/**
 *  \brief Get the number of 2D rendering drivers available for the current 
 *         display.
 *  
 *  A render driver is a set of code that handles rendering and texture
 *  management on a particular display.  Normally there is only one, but
 *  some drivers may have several available with different capabilities.
 *  
 *  \sa SDL_GetRenderDriverInfo()
 *  \sa SDL_CreateRenderer()
 */
extern DECLSPEC int SDLCALL SDL_GetNumRenderDrivers(void);

/**
 *  \brief Get information about a specific 2D rendering driver for the current 
 *         display.
 *  
 *  \param index The index of the driver to query information about.
 *  \param info  A pointer to an SDL_RendererInfo struct to be filled with 
 *               information on the rendering driver.
 *  
 *  \return 0 on success, -1 if the index was out of range.
 *  
 *  \sa SDL_CreateRenderer()
 */
extern DECLSPEC int SDLCALL SDL_GetRenderDriverInfo(int index,
                                                    SDL_RendererInfo * info);

/**
 *  \brief Create a 2D rendering context for a window.
 *  
 *  \param window The window where rendering is displayed.
 *  \param index    The index of the rendering driver to initialize, or -1 to 
 *                  initialize the first one supporting the requested flags.
 *  \param flags    ::SDL_RendererFlags.
 *  
 *  \return A valid rendering context or NULL if there was an error.
 *  
 *  \sa SDL_CreateSoftwareRenderer()
 *  \sa SDL_GetRendererInfo()
 *  \sa SDL_DestroyRenderer()
 */
extern DECLSPEC SDL_Renderer * SDLCALL SDL_CreateRenderer(SDL_Window * window,
                                               int index, Uint32 flags);

/**
 *  \brief Create a 2D software rendering context for a surface.
 *  
 *  \param surface The surface where rendering is done.
 *  
 *  \return A valid rendering context or NULL if there was an error.
 *  
 *  \sa SDL_CreateRenderer()
 *  \sa SDL_DestroyRenderer()
 */
extern DECLSPEC SDL_Renderer * SDLCALL SDL_CreateSoftwareRenderer(SDL_Surface * surface);

/**
 *  \brief Get the renderer associated with a window.
 */
extern DECLSPEC SDL_Renderer * SDLCALL SDL_GetRenderer(SDL_Window * window);

/**
 *  \brief Get information about a rendering context.
 */
extern DECLSPEC int SDLCALL SDL_GetRendererInfo(SDL_Renderer * renderer,
                                                SDL_RendererInfo * info);

/**
 *  \brief Create a texture for a rendering context.
 *  
 *  \param format The format of the texture.
 *  \param access One of the enumerated values in ::SDL_TextureAccess.
 *  \param w      The width of the texture in pixels.
 *  \param h      The height of the texture in pixels.
 *  
 *  \return The created texture is returned, or 0 if no rendering context was 
 *          active,  the format was unsupported, or the width or height were out
 *          of range.
 *  
 *  \sa SDL_QueryTexture()
 *  \sa SDL_UpdateTexture()
 *  \sa SDL_DestroyTexture()
 */
extern DECLSPEC SDL_Texture * SDLCALL SDL_CreateTexture(SDL_Renderer * renderer,
                                                        Uint32 format,
                                                        int access, int w,
                                                        int h);

/**
 *  \brief Create a texture from an existing surface.
 *  
 *  \param surface The surface containing pixel data used to fill the texture.
 *  
 *  \return The created texture is returned, or 0 on error.
 *  
 *  \note The surface is not modified or freed by this function.
 *  
 *  \sa SDL_QueryTexture()
 *  \sa SDL_DestroyTexture()
 */
extern DECLSPEC SDL_Texture * SDLCALL SDL_CreateTextureFromSurface(SDL_Renderer * renderer, SDL_Surface * surface);

/**
 *  \brief Query the attributes of a texture
 *  
 *  \param texture A texture to be queried.
 *  \param format  A pointer filled in with the raw format of the texture.  The 
 *                 actual format may differ, but pixel transfers will use this 
 *                 format.
 *  \param access  A pointer filled in with the actual access to the texture.
 *  \param w       A pointer filled in with the width of the texture in pixels.
 *  \param h       A pointer filled in with the height of the texture in pixels.
 *  
 *  \return 0 on success, or -1 if the texture is not valid.
 */
extern DECLSPEC int SDLCALL SDL_QueryTexture(SDL_Texture * texture,
                                             Uint32 * format, int *access,
                                             int *w, int *h);

/**
 *  \brief Set an additional color value used in render copy operations.
 *  
 *  \param texture The texture to update.
 *  \param r       The red color value multiplied into copy operations.
 *  \param g       The green color value multiplied into copy operations.
 *  \param b       The blue color value multiplied into copy operations.
 *  
 *  \return 0 on success, or -1 if the texture is not valid or color modulation 
 *          is not supported.
 *  
 *  \sa SDL_GetTextureColorMod()
 */
extern DECLSPEC int SDLCALL SDL_SetTextureColorMod(SDL_Texture * texture,
                                                   Uint8 r, Uint8 g, Uint8 b);


/**
 *  \brief Get the additional color value used in render copy operations.
 *  
 *  \param texture The texture to query.
 *  \param r         A pointer filled in with the current red color value.
 *  \param g         A pointer filled in with the current green color value.
 *  \param b         A pointer filled in with the current blue color value.
 *  
 *  \return 0 on success, or -1 if the texture is not valid.
 *  
 *  \sa SDL_SetTextureColorMod()
 */
extern DECLSPEC int SDLCALL SDL_GetTextureColorMod(SDL_Texture * texture,
                                                   Uint8 * r, Uint8 * g,
                                                   Uint8 * b);

/**
 *  \brief Set an additional alpha value used in render copy operations.
 *  
 *  \param texture The texture to update.
 *  \param alpha     The alpha value multiplied into copy operations.
 *  
 *  \return 0 on success, or -1 if the texture is not valid or alpha modulation 
 *          is not supported.
 *  
 *  \sa SDL_GetTextureAlphaMod()
 */
extern DECLSPEC int SDLCALL SDL_SetTextureAlphaMod(SDL_Texture * texture,
                                                   Uint8 alpha);

/**
 *  \brief Get the additional alpha value used in render copy operations.
 *  
 *  \param texture The texture to query.
 *  \param alpha     A pointer filled in with the current alpha value.
 *  
 *  \return 0 on success, or -1 if the texture is not valid.
 *  
 *  \sa SDL_SetTextureAlphaMod()
 */
extern DECLSPEC int SDLCALL SDL_GetTextureAlphaMod(SDL_Texture * texture,
                                                   Uint8 * alpha);

/**
 *  \brief Set the blend mode used for texture copy operations.
 *  
 *  \param texture The texture to update.
 *  \param blendMode ::SDL_BlendMode to use for texture blending.
 *  
 *  \return 0 on success, or -1 if the texture is not valid or the blend mode is
 *          not supported.
 *  
 *  \note If the blend mode is not supported, the closest supported mode is
 *        chosen.
 *  
 *  \sa SDL_GetTextureBlendMode()
 */
extern DECLSPEC int SDLCALL SDL_SetTextureBlendMode(SDL_Texture * texture,
                                                    SDL_BlendMode blendMode);

/**
 *  \brief Get the blend mode used for texture copy operations.
 *  
 *  \param texture   The texture to query.
 *  \param blendMode A pointer filled in with the current blend mode.
 *  
 *  \return 0 on success, or -1 if the texture is not valid.
 *  
 *  \sa SDL_SetTextureBlendMode()
 */
extern DECLSPEC int SDLCALL SDL_GetTextureBlendMode(SDL_Texture * texture,
                                                    SDL_BlendMode *blendMode);

/**
 *  \brief Update the given texture rectangle with new pixel data.
 *  
 *  \param texture   The texture to update
 *  \param rect      A pointer to the rectangle of pixels to update, or NULL to 
 *                   update the entire texture.
 *  \param pixels    The raw pixel data.
 *  \param pitch     The number of bytes between rows of pixel data.
 *  
 *  \return 0 on success, or -1 if the texture is not valid.
 *  
 *  \note This is a fairly slow function.
 */
extern DECLSPEC int SDLCALL SDL_UpdateTexture(SDL_Texture * texture,
                                              const SDL_Rect * rect,
                                              const void *pixels, int pitch);

/**
 *  \brief Lock a portion of the texture for pixel access.
 *  
 *  \param texture   The texture to lock for access, which was created with 
 *                   ::SDL_TEXTUREACCESS_STREAMING.
 *  \param rect      A pointer to the rectangle to lock for access. If the rect 
 *                   is NULL, the entire texture will be locked.
 *  \param pixels    This is filled in with a pointer to the locked pixels, 
 *                   appropriately offset by the locked area.
 *  \param pitch     This is filled in with the pitch of the locked pixels.
 *  
 *  \return 0 on success, or -1 if the texture is not valid or was not created with ::SDL_TEXTUREACCESS_STREAMING.
 *  
 *  \sa SDL_UnlockTexture()
 */
extern DECLSPEC int SDLCALL SDL_LockTexture(SDL_Texture * texture,
                                            const SDL_Rect * rect,
                                            void **pixels, int *pitch);

/**
 *  \brief Unlock a texture, uploading the changes to video memory, if needed.
 *  
 *  \sa SDL_LockTexture()
 */
extern DECLSPEC void SDLCALL SDL_UnlockTexture(SDL_Texture * texture);

/**
 *  \brief Set the drawing area for rendering on the current target.
 *
 *  \param rect The rectangle representing the drawing area, or NULL to set the viewport to the entire target.
 *
 *  The x,y of the viewport rect represents the origin for rendering.
 *
 *  \note When the window is resized, the current viewport is automatically
 *        centered within the new window size.
 */
extern DECLSPEC int SDLCALL SDL_RenderSetViewport(SDL_Renderer * renderer,
                                                  const SDL_Rect * rect);

/**
 *  \brief Get the drawing area for the current target.
 */
extern DECLSPEC void SDLCALL SDL_RenderGetViewport(SDL_Renderer * renderer,
                                                   SDL_Rect * rect);

/**
 *  \brief Set the color used for drawing operations (Fill and Line).
 *  
 *  \param r The red value used to draw on the rendering target.
 *  \param g The green value used to draw on the rendering target.
 *  \param b The blue value used to draw on the rendering target.
 *  \param a The alpha value used to draw on the rendering target, usually 
 *           ::SDL_ALPHA_OPAQUE (255).
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDL_SetRenderDrawColor(SDL_Renderer * renderer,
                                           Uint8 r, Uint8 g, Uint8 b,
                                           Uint8 a);

/**
 *  \brief Get the color used for drawing operations (Fill and Line).
 *  
 *  \param r A pointer to the red value used to draw on the rendering target.
 *  \param g A pointer to the green value used to draw on the rendering target.
 *  \param b A pointer to the blue value used to draw on the rendering target.
 *  \param a A pointer to the alpha value used to draw on the rendering target, 
 *           usually ::SDL_ALPHA_OPAQUE (255).
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDL_GetRenderDrawColor(SDL_Renderer * renderer,
                                           Uint8 * r, Uint8 * g, Uint8 * b,
                                           Uint8 * a);

/**
 *  \brief Set the blend mode used for drawing operations (Fill and Line).
 *  
 *  \param blendMode ::SDL_BlendMode to use for blending.
 *  
 *  \return 0 on success, or -1 on error
 *  
 *  \note If the blend mode is not supported, the closest supported mode is 
 *        chosen.
 *  
 *  \sa SDL_GetRenderDrawBlendMode()
 */
extern DECLSPEC int SDLCALL SDL_SetRenderDrawBlendMode(SDL_Renderer * renderer,
                                                       SDL_BlendMode blendMode);

/**
 *  \brief Get the blend mode used for drawing operations.
 *  
 *  \param blendMode A pointer filled in with the current blend mode.
 *  
 *  \return 0 on success, or -1 on error
 *  
 *  \sa SDL_SetRenderDrawBlendMode()
 */
extern DECLSPEC int SDLCALL SDL_GetRenderDrawBlendMode(SDL_Renderer * renderer,
                                                       SDL_BlendMode *blendMode);

/**
 *  \brief Clear the current rendering target with the drawing color
 *
 *  This function clears the entire rendering target, ignoring the viewport.
 */
extern DECLSPEC int SDLCALL SDL_RenderClear(SDL_Renderer * renderer);

/**
 *  \brief Draw a point on the current rendering target.
 *  
 *  \param x The x coordinate of the point.
 *  \param y The y coordinate of the point.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawPoint(SDL_Renderer * renderer,
                                                int x, int y);

/**
 *  \brief Draw multiple points on the current rendering target.
 *  
 *  \param points The points to draw
 *  \param count The number of points to draw
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawPoints(SDL_Renderer * renderer,
                                                 const SDL_Point * points,
                                                 int count);

/**
 *  \brief Draw a line on the current rendering target.
 *  
 *  \param x1 The x coordinate of the start point.
 *  \param y1 The y coordinate of the start point.
 *  \param x2 The x coordinate of the end point.
 *  \param y2 The y coordinate of the end point.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawLine(SDL_Renderer * renderer,
                                               int x1, int y1, int x2, int y2);

/**
 *  \brief Draw a series of connected lines on the current rendering target.
 *  
 *  \param points The points along the lines
 *  \param count The number of points, drawing count-1 lines
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawLines(SDL_Renderer * renderer,
                                                const SDL_Point * points,
                                                int count);

/**
 *  \brief Draw a rectangle on the current rendering target.
 *  
 *  \param rect A pointer to the destination rectangle, or NULL to outline the entire rendering target.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawRect(SDL_Renderer * renderer,
                                               const SDL_Rect * rect);

/**
 *  \brief Draw some number of rectangles on the current rendering target.
 *  
 *  \param rects A pointer to an array of destination rectangles.
 *  \param count The number of rectangles.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderDrawRects(SDL_Renderer * renderer,
                                                const SDL_Rect * rects,
                                                int count);

/**
 *  \brief Fill a rectangle on the current rendering target with the drawing color.
 *  
 *  \param rect A pointer to the destination rectangle, or NULL for the entire 
 *              rendering target.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderFillRect(SDL_Renderer * renderer,
                                               const SDL_Rect * rect);

/**
 *  \brief Fill some number of rectangles on the current rendering target with the drawing color.
 *  
 *  \param rects A pointer to an array of destination rectangles.
 *  \param count The number of rectangles.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderFillRects(SDL_Renderer * renderer,
                                                const SDL_Rect * rects,
                                                int count);

/**
 *  \brief Copy a portion of the texture to the current rendering target.
 *  
 *  \param texture The source texture.
 *  \param srcrect   A pointer to the source rectangle, or NULL for the entire 
 *                   texture.
 *  \param dstrect   A pointer to the destination rectangle, or NULL for the 
 *                   entire rendering target.
 *  
 *  \return 0 on success, or -1 on error
 */
extern DECLSPEC int SDLCALL SDL_RenderCopy(SDL_Renderer * renderer,
                                           SDL_Texture * texture,
                                           const SDL_Rect * srcrect,
                                           const SDL_Rect * dstrect);

/**
 *  \brief Read pixels from the current rendering target.
 *  
 *  \param rect   A pointer to the rectangle to read, or NULL for the entire 
 *                render target.
 *  \param format The desired format of the pixel data, or 0 to use the format
 *                of the rendering target
 *  \param pixels A pointer to be filled in with the pixel data
 *  \param pitch  The pitch of the pixels parameter.
 *  
 *  \return 0 on success, or -1 if pixel reading is not supported.
 *  
 *  \warning This is a very slow operation, and should not be used frequently.
 */
extern DECLSPEC int SDLCALL SDL_RenderReadPixels(SDL_Renderer * renderer,
                                                 const SDL_Rect * rect,
                                                 Uint32 format,
                                                 void *pixels, int pitch);

/**
 *  \brief Update the screen with rendering performed.
 */
extern DECLSPEC void SDLCALL SDL_RenderPresent(SDL_Renderer * renderer);

/**
 *  \brief Destroy the specified texture.
 *  
 *  \sa SDL_CreateTexture()
 *  \sa SDL_CreateTextureFromSurface()
 */
extern DECLSPEC void SDLCALL SDL_DestroyTexture(SDL_Texture * texture);

/**
 *  \brief Destroy the rendering context for a window and free associated
 *         textures.
 *  
 *  \sa SDL_CreateRenderer()
 */
extern DECLSPEC void SDLCALL SDL_DestroyRenderer(SDL_Renderer * renderer);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
/* *INDENT-OFF* */
}
/* *INDENT-ON* */
#endif
#include "close_code.h"

#endif /* _SDL_render_h */

/* vi: set ts=4 sw=4 expandtab: */
