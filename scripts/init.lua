local config = require "scripts.config"
local sdl_scancode = require "scripts.sdl_scancode"

local M = {}

local textures = {}

-- 将表 src_table 中所有为 key 的键（key 的类型不为 table）对应的值按照 rplc_table 进行替换
local function replace_recursive(src_table, rplc_table, key)
    for k, v in pairs(src_table) do
        if k == key then
            local new_v = rplc_table[v]
            if new_v ~= nil then
                src_table[k] = new_v
            else
                print("Mapping for value '" .. tostring(v) .. "' does not exist")
            end
        elseif type(v) == "table" then
            replace_recursive(v, rplc_table, key)
        end
    end
end

function M.load_config()
    -- 在这个阶段将 config 中的字符串转译为索引
    for i, v in ipairs(config["textures"]) do
        textures[v["id"]] = i
    end
    replace_recursive(config, textures, "texture")
    replace_recursive(config, sdl_scancode, "scancode")
    return config
end

M.load_config()

return M
