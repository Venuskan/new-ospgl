local glm = require("glm")
local assets = require("assets")


local hmap = assets.get_image("rss_textures:earth/hmap.png")
local cmap = assets.get_image("rss_textures:earth/color.png")

function projected_to_pixel(prj)

	return glm.vec2.new(prj.x / glm.two_pi + 0.5, prj.y / glm.pi);

end

function generate(info, out)

	local pix = projected_to_pixel(info.coord_2d);
	local earth = hmap:get():sample_bilinear(pix).x - 0.0941;
	if earth < 0.0 then

		earth = earth - earth * earth * 160.0;
	end

	out.height = (earth * info.radius * 0.002);
	out.color = cmap:get():sample_bilinear(pix):to_vec3();
end

