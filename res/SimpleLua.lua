
function update_entities(frame)
    if frame % 10 == 0 then
        set_position(1, 100 * frame, 200 * frame) -- Move entity 1
    end
    local a = 5
    local b = 150

    local c = a + b
end

function onCollision(me, other)
    myPos = GetComponent(me,"PositionC")
    otherPos = GetComponent(other, "PositionC")


end