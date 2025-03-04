---@meta

local toml = {}

---@class toml.table
toml.table = {}

---@return toml.table
function toml.table.new() end
---@param key string
---@param table toml.table
function toml.table:insert_table(key, table) end
---@param key string
---@param num number
function toml.table:insert_number(key, num) end
---@param key string
---@param str string
function toml.table:insert_string(key, str) end
---@param key string
---@param bool boolean
function toml.table:insert_bool(key, bool) end
---@param key string
---@param vec glm.vec3
function toml.table:insert_vec3(key, vec) end

---@return toml.table
---@param key string
function toml.table:get_table(key) end
---@return number
---@param key string
function toml.table:get_number(key) end
---@return number
---@param key string
---@param def number
function toml.table:get_number_or(key, def) end
---@return string
---@param key string
function toml.table:get_string(key) end
---@return string
---@param key string
---@param def string
function toml.table:get_string_or(key, def) end
---@return boolean
---@param key string
function toml.table:get_bool(key) end
---@return boolean
---@param key string
---@param def boolean
function toml.table:get_bool_or(key, def) end
---@return glm.vec3
---@param key string
function toml.table:get_vec3(key) end
---@return glm.vec3
---@param key string
---@param def glm.vec3
function toml.table:get_vec3_or(key, def) end

---@return table
---@param key string
function toml.table:get_array_of_string(key) end

---@return table
---@param key string
function toml.table:get_array_of_number(key) end

---@return table
---@param key string
function toml.table:get_array_of_table(key) end

---@param target vehicle
function toml.table:read_to_vehicle(target) end

---@param key string
function toml.table:erase(key) end


return toml
