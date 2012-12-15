/* 
 * RealTime JavaScript Raytracer DEMO
 * Forked from Jonas Wagner's work
 * http://29a.ch/2010/6/2/realtime-raytracing-in-javascript
 */

var abs = Math.abs,
	sqrt = Math.sqrt,
	floor = Math.floor,
	min = Math.min;

var V3 = function(x, y, z) {
	this.x = x;
	this.y = y;
	this.z = z;
}
V3.prototype = {
	// iop -> inplace
	// ops -> scalar
	add: function(v) {
		return new V3(this.x + v.x, this.y + v.y, this.z + v.z);
	},
	
	iadd: function(v) {
		this.x += v.x;
		this.y += v.y;
		this.z += v.z;
	},
	
	sub: function(v) {
		return new V3(this.x - v.x, this.y - v.y, this.z - v.z);
	},
	
	isub: function(v) {
		this.x -= v.x;
		this.y -= v.y;
		this.z -= v.z;
	},
	
	mul: function(v) {
		return new V3(this.x * v.x, this.y * v.y, this.z * v.z);
	},
	
	div: function(v) {
		return new V3(this.x / v.x, this.y / v.y, this.z / v.z);
	},
	
	muls: function(s) {
		return new V3(this.x * s, this.y * s, this.z * s);
	},
	
	imuls: function(s) {
		this.x *= s;
		this.y *= s;
		this.z *= s;
	},
	
	divs: function(s) {
		return this.muls(1.0 / s);
	},
	
	dot: function(v) {
		return this.x * v.x + this.y * v.y + this.z * v.z;
	},
	
	normalize: function() {
		var s = 1.0 / sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
		return new V3(this.x * s, this.y * s, this.z * s);
	},
	
	magnitude: function() {
		return sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
	},
	
	magnitude2: function() {
		return this.x * this.x + this.y * this.y + this.z * this.z;
	},
	
	copy: function() {
		return new V3(this.x, this.y, this.z);
	}
};


var Camera = function(origin, topleft, topright, bottomleft) {
	this.origin = origin;
	this.topleft = topleft;
	this.topright = topright;
	this.bottomleft = bottomleft;
	this.update();
}
Camera.prototype = {
	update: function() {
		this.xd = this.topright.sub(this.topleft);
		this.yd = this.bottomleft.sub(this.topleft);
	},
	
	getMagnitude: function(x, y) {
		var p = this.topleft.add(this.xd.muls(x));
		p.iadd(this.yd.muls(y));
		return p.sub(this.origin).magnitude();
	},
	
	getRay: function(x, y) {
		// point on screen plane
		var p = this.topleft.add(this.xd.muls(x));
		p.iadd(this.yd.muls(y));
		p.isub(this.origin);
		return {
			origin: this.origin,
			direction: p.normalize()
		};
	}
};


var Sphere = function(center, radius) {
	this.center = center;
	this.radius = radius;
	this.radius2 = radius * radius;
};

Sphere.prototype = {
	// returns distance when ray intersects with sphere surface
	intersect: function(ray) {
		var distance = ray.origin.sub(this.center);
		var b = distance.dot(ray.direction);
		var c = b * b - distance.magnitude2() + this.radius2;
		return c > 0.0 ? -b - sqrt(c) : -1.0;
	},
	
	getNormal: function(point) {
		return point.sub(this.center).normalize();
	}
};


var Body = function(shape, material) {
	this.shape = shape;
	this.material = material;
}

var CubeMap = function(img) {
	img = getDataFromImage(img);
	var s = this.size = img.width / 3;
	this.width = img.width;
	this.left = s * img.width;
	this.front = (s + s * img.width);
	this.right = (s * 2 + s * img.width);
	this.back = (s + s * 3 * img.width);
	this.up = s;
	this.down = (s + s * 2 * img.width);
	this.colors = [];
	var i = 0;
	var data = img.data;
	for (var i = 0; i < data.length; i++) {
		var color = new V3(data[i++], data[i++], data[i++]);
		color.imuls(0.00390625);
		// fake hdri
		if (color.x * color.y * color.z > 0.95) {
			color.imuls(2.5);
		}
		this.colors.push(color);
	}
}
CubeMap.prototype = {
	sample: function(ray) {
		var d = ray.direction,
			ax = abs(d.x),
			ay = abs(d.y),
			az = abs(d.z),
			s = this.size,
			u = 0.0,
			v = 0.0,
			o;

		if (ax >= ay && ax >= az) {
			// right
			if (d.x > 0.0) {
				u = 1.0 - (d.z / d.x + 1.0) * 0.5;
				v = (d.y / d.x + 1.0) * 0.5;
				o = this.right;
			}
			// left
			else {
				u = 1.0 - (d.z / d.x + 1.0) * 0.5
				v = 1.0 - (d.y / d.x + 1.0) * 0.5;
				o = this.left;
			}
		} else if (ay >= ax && ay >= az) {
			// up
			if (d.y <= 0.0) {
				u = (d.x / d.y + 1.0) * 0.5;
				v = 1.0 - (d.z / d.y + 1.0) * 0.5;
				o = this.up;
			}
			// down
			else {
				u = (d.x / d.y + 1.0) * 0.5;
				v = 1.0 - (d.z / d.y + 1.0) * 0.5;
				o = this.down;
			}
		} else {
			// front
			if (d.z > 0.0) {
				u = (d.x / d.z + 1.0) * 0.5;
				v = (d.y / d.z + 1.0) * 0.5;
				o = this.front;
			}
			// back
			else {
				u = (d.x / d.z + 1.0) * 0.5;
				v = (d.y / d.z + 1.0) * 0.5;
				o = this.back;
			}
		}
		o += floor(u * s) + floor(v * s) * this.width;
		return this.colors[o];
	}
}

var Renderer = function(scene, ctx) {
	this.scene = scene;
	this.ctx = ctx;
	this.img = ctx.createImageData(scene.output.width, scene.output.height);
	this.data = this.img.data;
}

Renderer.prototype = {
	render: function() {
		var w = this.img.width,
			h = this.img.height,
			i = 0,
			data = this.data;

		for (var y = 0.0, ystep = 1.0 / h; y < 0.99999; y += ystep) {
			for (var x = 0, xstep = 1.0 / w; x < 0.99999; x += xstep) {
				
				var ray = this.scene.camera.getRay(x, y),
					color = this.trace(ray, 1);

				data[i++] = min(floor(color.x * 256), 255);
				data[i++] = min(floor(color.y * 256), 255);
				data[i++] = min(floor(color.z * 256), 255);
				data[i++] = 255;
			}
		}
		return this.img;
	},
	
	trace: function(ray, n) {
		var mint = Infinity;
		var hit = null;

		for (var i = 0; i < this.scene.objects.length; i++) {
			var o = this.scene.objects[i];
			var t = o.shape.intersect(ray);
			if (t > 0.0 && t < mint) {
				mint = t;
				hit = o;
			}
		}

		if (hit) {
			var origin = ray.origin.add(ray.direction.muls(mint)),
				normal = hit.shape.getNormal(origin),
				direction = hit.material.bounce(ray, normal);

			// if the ray is refracted move the intersection in a bit
			if (direction.dot(ray.direction) > 0.0) {
				n -= 1;
				origin = ray.origin.add(ray.direction.muls(1.000001 * mint));
			}
			var newray = {
				origin: origin,
				direction: direction
			}
			if (n > 1) {
				return this.scene.sky.sample(ray);
			}
			return this.trace(newray, n + 1).mul(hit.material.color);
		}
		return this.scene.sky.sample(ray);
	}
}

var Chrome = function(color) {
	this.color = color;
}
Chrome.prototype = {
	bounce: function(ray, normal) {
		var theta1 = abs(ray.direction.dot(normal));
		return ray.direction.add(normal.muls(theta1 * 2.0));
	}
};


var Glass = function(color, ior) {
	this.color = color;
	this.ior = ior;
}
Glass.prototype = {
	bounce: function(ray, normal) {
		var theta1 = abs(ray.direction.dot(normal));
		if (theta1 >= 0.0) {
			var internalIndex = this.ior;
			var externalIndex = 1.0;
		} else {
			var internalIndex = 1.0;
			var externalIndex = this.ior;
		}
		var eta = externalIndex / internalIndex;
		var theta2 = sqrt(1.0 - (eta * eta) * (1.0 - (theta1 * theta1)));
		// reflection
		return ray.direction.muls(eta).sub(normal.muls(theta2 - eta * theta1));

	}
}

var Animator = function() {
	this.t = 0.0;
	this.points = [];
}
Animator.prototype = {
	add: function(point, speed) {
		this.points.push({
			point: point,
			radius: point.magnitude(),
			alpha: Math.atan2(1, 0) - Math.atan2(point.x, point.z),
			speed: speed
		});
	},
	
	tick: function(td) {
		this.t += td;
		for (var i = 0; i < this.points.length; i++) {
			var p = this.points[i];
			p.point.x = Math.cos(p.alpha + this.t * p.speed) * p.radius;
			p.point.z = Math.sin(p.alpha + this.t * p.speed) * p.radius;
		}
		//for(var i = 0; i < objects.lenght; i++){
		//    var shape = objects[i].shape;
		//    this.transform(shape.center);
		//}
	}
}

function main(cw, ch, img, quality, motionblur) {

	var scene = {

		output: {
			width: Math.round(cw * quality),
			height: Math.round(ch * quality)
		},
		
		sky: new CubeMap(img),

		camera: new Camera(
			new V3(0.0, 0.0, -8.4), 
			new V3(-1.3, -1.0, -7.0), 
			new V3(1.3, -1.0, -7.0), 
			new V3(-1.3, 1.0, -7.0)
		),

		objects: [
			new Body(
				new Sphere(new V3(0.0, 0.0, 3.0), 1.0), 
				new Glass(new V3(0.0, 1.0, 1.0), 1.5)
			),

			new Body(
				new Sphere(new V3(0.0, 0.0, 0.0), 1.5), 
				new Chrome(new V3(0.5, 0.5, 0.8))
			), 

			new Body(
				new Sphere(new V3(0.0, 0.5, 6.0), 0.6), 
				new Chrome(new V3(0.8, 0.1, 0.2))
			)

		]
	}

	var ctx = new Canvas(cw, ch);

	var animator = new Animator(scene),
		speed = 0;

	animator.add(scene.camera.origin, 0.15);
	animator.add(scene.camera.topleft, 0.15);
	animator.add(scene.camera.topright, 0.15);
	animator.add(scene.camera.bottomleft, 0.15);

	for (var i = 0; i < scene.objects.length; i++) {
		speed = 2 / (i + 1);
		if (i & 1) speed = -speed;
		animator.add(scene.objects[i].shape.center, speed);
	}

	var renderer = new Renderer(scene, ctx),
		frame = null;

	canvas.globalAlpha = 1.00 - motionblur;
	canvas.fillStyle = "#222222";
	
	canvas.requestAnimationFrame(function(){
		animator.tick(0.15);
		scene.camera.update();
		
		frame = renderer.render();
		canvas.fillRect(0, 0, cw, 768);
		canvas.putImageData(frame, 0, 0);

	});


}

function init() {
	var img = new Image(),
		q = 1.00; // 0.2   ....   2.0

	img.src = 'demos/demo.realtimeRayTracer.jpeg';
	img.onload = function(){
		
	};

	var cw = 160,
		ch = 120;

	main(cw, ch, img, q, 0.2);
}

function getDataFromImage(img){
	var c = new Canvas(img.width, img.height);
	c.drawImage(img, 0, 0, img.width, img.height);
	return c.getImageData(0, 0, img.width, img.height);
}

init();
