sr = script.parent:GetComponent("ShapeRenderer")
tf = script.parent:GetComponent("Transform")
tr = script.parent:GetComponent("TextRenderer")
storage = script.parent.storage
default_color = sr.color:Clone()

speed = 3
function _process()

	storage.cooldown = storage.cooldown + GetDelta()
	move = GetCursorPos() - tf.position
	dir = move:Normalized()
	script.parent.storage.direction = dir
	
	if tf.position:Distance(GetCursorPos()) > 0.25 then tf.position = tf.position + move * GetDelta() * speed end
	tf.rotation = -math.atan(dir.y, dir.x)
	
	if GetMouseButton(0).Pressed and storage.cooldown > storage.fire_rate then
		
		bullet = FindObject("bullet"):Clone()
		bullet.active = true
		
		bullet.storage.direction = dir
		bullet:GetComponent("Transform").position = tf.position
		
		storage.cooldown = 0
	end

end