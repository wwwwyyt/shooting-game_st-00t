local config = require "scripts.config"
local sdl_scancode = require "scripts.sdl_scancode"

local M = {}

local textures = {}

local function table_dump(t)
    local level = 0
    local indent = 4

    function f(t, level)
        -- 缩进
        local space = " "
        print(space:rep(level * indent) .. "{")
        for k, v in pairs(t) do
            print(space:rep((level + 1) * indent) .. tostring(k) .. " = " .. tostring(v))
            if type(v) == "table" then
                f(v, level + 1)                
            end
        end
        print(space:rep(level * indent) .. "},")
    end
    
    f(t, level)
end

-- 将表 src_table 中所有为 key 的键对应的值按照 rplc_table 进行替换
-- 当 key 对应的值类型为 table 时，递归地对该 table 进行替换操作
local function replace_recursive(src_table, rplc_table, key)
    for k, v in pairs(src_table) do
        if k == key then
            local new_v = rplc_table[v]
            if new_v ~= nil then
                -- src_table[k] = new_v
            else
                print("Mapping for value '" .. tostring(v) .. "' (key='" .. tostring(k) .. "') does not exist")
            end
        elseif type(v) == "table" then
            replace_recursive(v, rplc_table, key)
        end
    end
end

-- 加载配置
function M.load_config()
    -- 构造纹理的 ID 到索引的映射表
    for i, v in ipairs(config["textures"]) do       
        textures[v["id"]] = i
    end

    -- 将 config 中的字符串转译为索引
    replace_recursive(config, textures, "texture")
    replace_recursive(config, sdl_scancode, "scancode")
    return config
end

return M
