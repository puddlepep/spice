player = FindObject("player")
player_tf = player:GetComponent("Transform")
tf = script.parent:GetComponent("Transform")
storage = script.parent.storage

speed = 400
rot_speed = 5
function _process()
	
	if not storage.dead then
		dir = (player_tf.position - tf.position):Normalized()
		tf.position = tf.position + dir * speed * GetDelta()
		tf.rotation = tf.rotation + rot_speed * GetDelta()
	else
		script.parent:GetComponent("ShapeRenderer").color = Color.FromRGB(0.25, 0.25, 0.25, 0.25)
	end
end