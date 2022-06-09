tf = script.parent:GetComponent("Transform")

speed = 10
life = 0

function _process()
	
	dir = script.parent.storage.direction
	life = life + GetDelta()
	tf.position = tf.position + dir * speed
	tf.rotation = -math.atan(dir.y, dir.x)
	
	if life > 2 then
		script.parent:Destroy()
	end
	
	enemies = FindObjects("enemy")
	for i = 1, #enemies do
		if enemies[i].active == true then
		
			if (enemies[i]:GetComponent("Transform").position - tf.position):Length() < 20 then
				enemies[i].storage.dead = true
				script.parent:Destroy()
			end
		
		end
	end
end