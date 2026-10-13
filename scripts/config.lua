return {
    -- 资源文件
    textures = {
        {
            file = "assets/texture/grids_2026-10-02.png",
        },
        {
            file = "assets/texture/player01_64x64_2026-10-02_01.png",
        },
        {
            file = "assets/font/font_black.png",
        },
        {
            file = "assets/font/font_white.png",
        }
    },

    model3d = {

    },

    fonts = {
        {
            -- 黑色字体
            texture = 2,
            size = 16,
        },
        {
            -- 白色字体
            texture = 3,
            size = 16,
        }
    },

    -- 输入绑定
    input = {
        {
            player = 0,
            key = "i",
            action = "up",
            trigger = "hold"
        },
        {
            player = 0,
            key = "k",
            action = "down",
            trigger = "hold"
        },
        {
            player = 0,
            key = "j",
            action = "left",
            trigger = "hold"
        },
        {
            player = 0,
            key = "l",
            action = "right",
            trigger = "hold"
        },
        {
            player = 0,
            key = "z",
            action = "a",
            trigger = "hold"
        },
        {
            player = 0,
            key = "x",
            action = "b",
            trigger = "hold"
        },
        {
            player = 0,
            key = "shift",
            action = "c",
            trigger = "hold"
        },
        {
            player = 0,
            key = "enter",
            action = "start",
            trigger = "press"
        },
    }
}
