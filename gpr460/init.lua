keys = {space = 44, right = 79, left = 80, down = 81, up = 82}

print("Called init! Hurray!")

print(get_current_frame())

Vector3 = {}

Vector3.mt = {
   __index = function(t, k)
      if k == "x" then return t[1] end
      if k == "y" then return t[2] end
      if k == "z" then return t[3] end
      return t[k]
   end
}

function Vector3.new(x, y, z)
   local v3 = {x or 0, y or 0, z or 0}
   setmetatable(v3, Vector3.mt)
   return v3
end

Vector3.mt.__add = function (a, b)
   return Vector3.new(a.x+b.x, a.y+b.y, a.z+b.z)
end

Vector3.mt.__mul = function (a, b)
   return Vector3.new(a.x*b, a.y*b, a.z*b)
end

function Vector3.dot(u, v)
   return u.x * v.x + u.y * v.y + u.z * v.z
end

function Vector3.zero()
   return Vector3.new(0, 0, 0)
end

function Vector3.right()
   return Vector3.new(1, 0, 0)
end

function Update()
    -- print(get_current_frame())
    local x = 0
    local y = 0
    if get_key(keys.right) then
        x = x + 1
    end
    if get_key(keys.left) then
        x = x - 1
    end
    if get_key(keys.down) then
        y = y + 1
    end
    if get_key(keys.up) then
        y = y - 1
    end

    if get_key_down(keys.space) then
        local object = spawn(100, 100)
        add_render_component(object)
    end

    local dist = math.sqrt(x^2 + y^2)
    if dist == 0 then
        dist = 1
    end

    local speed = 100
    local dt = get_dt()
    move_player((x / dist) * speed * dt, (y / dist) * speed * dt)
end