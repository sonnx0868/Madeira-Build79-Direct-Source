function love.load()
    local name, version, vendor, device = love.graphics.getRendererInfo()
    print(string.format("ANGLE_SMOKE renderer=%s version=%s vendor=%s device=%s",
        tostring(name), tostring(version), tostring(vendor), tostring(device)))
    love.event.quit()
end
