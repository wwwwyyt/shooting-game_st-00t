#include "text.h"
#include "graphic.h"
#include <SDL3/SDL_log.h>

static int fontCnt;
static Font *fonts;

/** @brief 用于获取字符在对应的文本框中的对应矩形
 *
 *  空间不足，无法显示完整字符时，丢弃该文本，返回大小为 0 的矩形
 *  @param tbox 用于指定文本框的矩形以及文本的对齐方式
 *  @param fontSize 字符的字体大小
 *  @param charCnt 当前渲染文本的字符总数，由此可确定各字符所需的矩形位置
 *  @param charIdx 当前渲染文本的字符索引，从 0 开始
 *
 *  @return SDL_Rect 当前字符的目标渲染矩形
 */
static SDL_Rect getDstRect(SDL_Rect *textbox, int fontSize, int charCnt, int charIdx)
{

        SDL_Rect dstrect = {0, 0, 0, 0};

        /* 条件不满足，返回大小为 0 矩形 */
        if (!textbox || fontSize <= 0 || charCnt <= 0 || charIdx < 0 || charIdx >= charCnt)
                return dstrect;

        /* 获取字符的行列数 */
        const int colCnt = (int)textbox->w / fontSize;

        /* 获取在左对齐情况下，当前字符所在的行列 */
        const int charCol = charIdx % colCnt;
        const int charRow = charIdx / colCnt;

        /* 构造目标矩形 */
        dstrect.w = dstrect.h = fontSize;
        dstrect.x = charCol * fontSize;
        dstrect.y = charRow * fontSize;

        return dstrect;
}

int TEXT_Init(lua_State *L)
{
        /* 加载配置 */
        lua_getfield(L, -1, "fonts");
        if (!lua_istable(L, -1))
        {
                SDL_Log("Field 'fonts' is not a table\n");
                lua_close(L);
                return 1;
        }

        fontCnt = (int)luaL_len(L, -1);
        fonts = SDL_calloc(fontCnt, sizeof *fonts);

        for (lua_Integer i = 1; i <= fontCnt; ++i)
        {
                lua_rawgeti(L, -1, i);

                /* 读取字库文件索引 */
                lua_getfield(L, -1, "texture");
                fonts[i - 1].texID = lua_tointeger(L, -1);
                lua_pop(L, 1);

                /* 读取字符像素大小 */
                lua_getfield(L, -1, "size");
                fonts[i - 1].size = lua_tointeger(L, -1);
                lua_pop(L, 1);

                /* 弹出 fonts[i] */
                lua_pop(L, 1);
        }
        /* 弹出 fonts */
        lua_pop(L, 1);

        return 0;
}

int TEXT_RenderText(int fontIdx, int charCnt, const int *charIdxArr, SDL_Rect *textbox)
{
        /* 获取字体 */
        Font *font = &(fonts[fontIdx]);

        /* 获取字库纹理相关信息 */
        int texID = font->texID;
        int texW, texH;
        GRAPHIC_GetTextureSize(texID, &texW, &texH);

        /* 获取字库纹理每列的字符数 */
        int colCnt = texW / font->size;

        /* 按照字符的编码绘制字符 */
        SDL_Rect srcrect = {0, 0, font->size, font->size};
        for (int i = 0; i < charCnt; ++i)
        {
                srcrect.x = (charIdxArr[i] % colCnt) * font->size;
                srcrect.y = (charIdxArr[i] / colCnt) * font->size;
                SDL_Rect dstrect = getDstRect(textbox, font->size, charCnt, i);
                GRAPHIC_DrawSprite(texID, &srcrect, &dstrect);
        }

        return 0;
}
