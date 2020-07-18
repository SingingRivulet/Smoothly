voxelDrawer = {}
function voxelDrawer.drawLine3D(from_position, to_position, memo, onPoint, onComplete)
    local temp

    -- safty first kids
    local x0 = math.floor(from_position.x)
    local y0 = math.floor(from_position.y)
    local z0 = math.floor(from_position.z)
    local x1 = math.floor(to_position.x)
    local y1 = math.floor(to_position.y)
    local z1 = math.floor(to_position.z)

    --'steep' xy Line, make longest delta x plane
    local swap_xy = math.abs(y1 - y0) > math.abs(x1 - x0)
    if swap_xy then
        temp = x0
        x0 = y0
        y0 = temp
        --swap(x0, y0)
        
        temp = x1
        x1 = y1
        y1 = temp
        --swap(x1, y1)
    end
    
    --do same for xz
    local swap_xz = math.abs(z1 - z0) > math.abs(x1 - x0)
    if swap_xz then
        temp = x0
        x0 = z0
        z0 = temp
        --swap(x0, z0)
        temp = x1
        x1 = z1
        z1 = temp
        --swap(x1, z1)
    end

    --delta is Length in each plane
    local delta_x = math.abs(x1 - x0)
    local delta_y = math.abs(y1 - y0)
    local delta_z = math.abs(z1 - z0)
    
    --drift controls when to step in 'shallow' planes
    --starting value keeps Line centred
    local drift_xy = (delta_x / 2)
    local drift_xz = (delta_x / 2)

    --direction of line
    local step_x = 1
    if x0 > x1 then
        step_x = -1
    end
    
    local step_y = 1
    if y0 > y1 then
        step_y = -1
    end
    
    local step_z = 1
    if z0 > z1 then
        step_z = -1
    end

    --starting point
    local y = y0
    local z = z0

    local cx, cy, cz

    --step through longest delta (which we have swapped to x)
    for  x = x0 , x1 , step_x  do
        
        --copy position
        cx = x
        cy = y
        cz = z

        --unswap (in reverse)
        if (swap_xz) then
            temp = cx
            cx = cz
            cz = temp
            --swap(cx, cz);
        end
        if (swap_xy) then
            temp = cx
            cx = cy
            cy = temp
            --swap(cx, cy);
        end
        
        --passes through this point
        if onPoint then
            onPoint({["x"] = cx,
                     ["y"] = cy,
                     ["z"] = cz}, memo)
        end
        
        --update progress in other planes
        drift_xy = drift_xy - delta_y
        drift_xz = drift_xz - delta_z

        --step in y plane
        if drift_xy < 0 then
            y = y + step_y
            drift_xy = drift_xy + delta_x
        end
        
        --same in z
        if drift_xz < 0 then
            z = z + step_z
            drift_xz = drift_xz + delta_x
        end
    end
    if onComplete then
        onComplete(memo)
    end
end
