project_settings = {
background_color = { 0.818627, 0.240773, 0.240773, 1 },
shipping_mode = false,
resizeable_window = true,
}
objects = {
{
name = "player",

active = true,
storage = {
{
name = "cooldown",
datatype = 2,
killdata = true,
data = 0.000000,
},
{
name = "fire_rate",
datatype = 2,
killdata = true,
data = 0.500000,
},
{
name = "direction",
datatype = 3,
killdata = true,
data = { 0.000000, 0.000000 },
},
},
components = {
{
scale_y = 1.000000,
__type = "Transform",
rotation = 0.000000,
y = 50.000000,
scale_x = 1.000000,
x = 50.000000,
},
{
color = {
__type = "colorarray",
__data = { 1.000000, 0.999990, 0.999990, 1.000000, },
},
__type = "ShapeRenderer",
visible = true,
points = {
__type = "vec2array",
__data = { {-50.000000, -50.000000}, {50.000000, 0.000000}, {-50.000000, 50.000000}, {-82.730003, 0.000000},  },
},
line_mode = true,
layer = 0.000000,
line_width = 3.000000,
},
{
active = true,
__type = "ScriptComponent",
filepath = "source/scripts/ship.lua",
},
}
},
{
name = "bullet",

active = false,
storage = {
{
name = "direction",
datatype = 3,
killdata = true,
data = { 0.000000, 0.000000 },
},
},
components = {
{
scale_y = 0.440000,
__type = "Transform",
rotation = 0.000000,
y = 0.000000,
scale_x = 0.480000,
x = 0.000000,
},
{
color = {
__type = "colorarray",
__data = { 1.000000, 1.000000, 1.000000, 1.000000, },
},
__type = "ShapeRenderer",
visible = true,
points = {
__type = "vec2array",
__data = { {-20.000000, -20.000000}, {20.000000, -20.000000}, {20.000000, 20.000000}, {-20.000000, 20.000000},  },
},
line_mode = true,
layer = 0.000000,
line_width = 3.000000,
},
{
active = true,
__type = "ScriptComponent",
filepath = "source/scripts/bullet.lua",
},
}
},
}