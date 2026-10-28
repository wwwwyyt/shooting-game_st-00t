#include "player.h"

static const int PLAYER_CNT = 2;
static Player *players;

static int move()
{
}

int PLAYER_Init(lua_State *L)
{
        players = SDL_calloc(PLAYER_CNT, sizeof *players);

        /* 加载配置 */
}

void PLAYER_Update(float dt)
{
        for (int i = 0; i < PLAYER_CNT; ++i)
        {
                if (players[i].active)
                {
                        OBJ_Update(&(players[i].object), dt);
                }
        }
}

int PLAYER_Add(void)
{
        for (int i = 0; i < PLAYER_CNT; ++i)
        {
                if (0 == players[i].active)
                {
                        players[i].active = 1;
                }
        }
}
