#ifndef __C_GUI_TTFONT_H_INCLUDED__
#define __C_GUI_TTFONT_H_INCLUDED__

#include <ft2build.h>
#include <freetype/freetype.h>
#include <irrlicht.h>
#include <vector>

namespace irr
{
namespace gui
{
class CGUITTFace : public irr::IReferenceCounted
{
public:
	CGUITTFace();
	virtual ~CGUITTFace();
	bool loaded;
	FT_Library	library;
	FT_Face		face;
	bool load(const c8* filename);
};
class CGUITTGlyph
{
public:
	bool cached;
	video::IVideoDriver* Driver;
	CGUITTGlyph();
    virtual ~CGUITTGlyph();
    void init();
	void cache(u32 idx);
	FT_Face *face;
	u32 size;
	u32 top;
	u32 left;
	u32 texw;
	u32 texh;
	u32 imgw;
	u32 imgh;
	video::ITexture *tex;
	u32 top16;
	u32 left16;
	u32 texw16;
	u32 texh16;
	u32 imgw16;
	u32 imgh16;
	video::ITexture *tex16;
	s32 offset;
	u8 *image;
};
class CGUITTFont:public irr::IReferenceCounted
{
public:
	u32 size;

	//! constructor
    CGUITTFont(video::IVideoDriver* Driver);

	//! destructor
	virtual ~CGUITTFont();

	//! loads a truetype font file
	bool attach(CGUITTFace *Face,u32 size);

	//! draws an text and clips it to the specified rectangle if wanted
    virtual void draw(const wchar_t* text, const core::rect<s32>& position, video::SColor color, bool hcenter=false, bool vcenter=false, const core::rect<s32>* clip=0);

	//! returns the dimension of a text
	virtual core::dimension2d<s32> getDimension(const wchar_t* text);

	//! Calculates the index of the character in the text which is on a specific position.
	virtual s32 getCharacterFromPos(const wchar_t* text, s32 pixel_x);

	scene::ISceneNode *createBillboard(const wchar_t* text,scene::ISceneManager *scene,scene::ISceneNode *parent = 0,s32 id = -1);

	bool AntiAlias;
	bool TransParency;
	bool attached;
private:
    s32 getWidthFromCharacter(wchar_t c);
	u32 getGlyphByChar(wchar_t c);
	video::IVideoDriver* Driver;
    std::vector< CGUITTGlyph > Glyphs;
	CGUITTFace *tt_face;
};

} // end namespace gui
} // end namespace irr

#endif

