return {
    -- 资源文件
    textures = {
        {
            id = "grids",
            file = "assets/texture/grids_2026-10-02.png",
        },
        {
            id = "player01",
            file = "assets/texture/player01_64x64_2026-10-02_01.png",
        },
        {
            id = "player01_single",
            file = "assets/texture/player01_single_2026-10-02.png",
        },
        {
            id = "font_black",
            file = "assets/font/font_black.png",
        },
        {
            id = "font_white",
            file = "assets/font/font_white.png",
        }
    },

    model3d = {

    },

    fonts = {
        {
            -- 黑色字体
            texture = "font_black",
            size = 16,
        },
        {
            -- 白色字体
            texture = "font_white",
            size = 16,
        }
    },

    audios = {

    },

    -- 输入绑定
    input = {
        {
            player = 0,
            scancode = "i",
            action = "up",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "k",
            action = "down",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "j",
            action = "left",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "l",
            action = "right",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "z",
            action = "a",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "x",
            action = "b",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "lshift",
            action = "c",
            trigger = "hold"
        },
        {
            player = 0,
            scancode = "return",
            action = "start",
            trigger = "press"
        },
    },

    -- 动画设置
    animations = {
        {

        }
    }
}
