keys = {right = 79, left = 80, down = 81, up = 82}

print("Called init! Hurray!")

print(get_current_frame())

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

    local dist = math.sqrt(x^2 + y^2)
    if dist == 0 then
        dist = 1
    end

    local speed = 100
    local dt = get_dt()
    move_player((x / dist) * speed * dt, (y / dist) * speed * dt)
end