rng = Random.new()
player = FindObject("player")
enemy = FindObject("enemy")

storage = script.parent.storage
function _process()

	storage.cooldown = storage.cooldown + GetDelta()
	
	if storage.cooldown > storage.enemy_spawn_time then
		storage.cooldown = 0.0
		
		spawn_position = Vec2.new(0, 0)
		if rng:NextInteger(0, 1) == 0 then
			-- Vertical
			
			x_position = rng:NextNumber(0, GetWindowWidth())
			y_position = 0
			
			if rng:NextInteger(0, 1) == 1 then
				-- Top
				y_position = GetWindowHeight() + 50
			
			else
				-- Bottom
				y_position = -50
				
			end
			
			spawn_position.x = x_position
			spawn_position.y = y_position
			
		else
			-- Horizontal
		
			y_position = rng:NextNumber(0, GetWindowHeight())
			x_position = 0
			
			if rng:NextInteger(0, 1) == 0 then
				-- Left
				x_position = -50
				
			else
				-- Right
				x_position = GetWindowWidth() + 50
				
			end
		
			spawn_position.x = x_position
			spawn_position.y = y_position
			
		end
		
		obj = enemy:Clone()
		obj:GetComponent("Transform").position = spawn_position
		obj.active = true
	end
	
end