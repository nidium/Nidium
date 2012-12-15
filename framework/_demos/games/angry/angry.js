/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

load("core/plugins/box2d.inc.js");

Native.showFPS(true);

// Init som useful stuff for easier access (don't need 'em all)
var b2Vec2 = Box2D.Common.Math.b2Vec2,
	b2AABB = Box2D.Collision.b2AABB,
	b2BodyDef = Box2D.Dynamics.b2BodyDef,
	b2Body = Box2D.Dynamics.b2Body,
	b2FixtureDef = Box2D.Dynamics.b2FixtureDef,
	b2Fixture = Box2D.Dynamics.b2Fixture,
	b2World = Box2D.Dynamics.b2World,
	b2MassData = Box2D.Collision.Shapes.b2MassData,
	b2PolygonShape = Box2D.Collision.Shapes.b2PolygonShape,
	b2CircleShape = Box2D.Collision.Shapes.b2CircleShape,
	b2RevoluteJointDef = Box2D.Dynamics.Joints.b2RevoluteJointDef,
	b2MouseJointDef = Box2D.Dynamics.Joints.b2MouseJointDef;


var world, fixDef, 
	shapes = {},
	ctx = canvas,
	vw = 1024,
	vh = 768,
	SCALE = 30;

var init = {
	start : function(){
		box2d.create.world();
		box2d.create.defaultFixture();

		this.createWalls();
		this.addEvents();
		
		setTimeout(function(){ add.random(); }, 0);
		setTimeout(function(){ add.random(); }, 100);
		setTimeout(function(){ add.random(); }, 500);
		setTimeout(function(){ add.random(); }, 700);
		setTimeout(function(){ add.random(); }, 1000);

		canvas.requestAnimationFrame(function(){
			loop.update();
			loop.draw();
		});

	},
	
	createWalls : function(){
		// leftWall
		add.box({
			x : -0.6,
			y : 11.8,		// 768px / 30 / 2
			height : 25.6,	// 768px / 30
			width : 2,
			isStatic : true,
			color : "#335599"
		});

		// rightWall
		add.box({
			x : 34.7,		// 1024 / 30 + 1.1
			y : 11.8,		// 768px / 30 / 2
			height : 25.6,	// 768px / 30
			width : 2,
			isStatic : true,
			color : "#335599"
		});

		// ground
		add.box({
			x : 17.07,		// 1024 / 30 / 2
			y : 25.6,
			height : 2,
			width : 34.15,	// 1024 / 30
			isStatic : true,
			color : "rgba(0, 0, 0, 0.9)"
		});

		// level bar
		add.box({
			x : 20,
			y : 13,
			height : 0.3,
			width : 18,
			angle : -0.20,
			isStatic : true,
			color : "#335599"
		});

		
	},
	
	addEvents : function(){
		canvas.onmousedown = function(e){
			add.random({
				x: (vw / SCALE) * (e.x / vw),
				y: 0
			});
		};
	}
};


var add = {
	random : function(options){
		options = options || {};
		
		if (Math.random() < 0.5){
			this.circle(options);
		} else {
			this.box(options);
		}
	},
	
	circle : function(options){
		options.radius = 1.0 + Math.random();
		var shape = new Circle(options);
		shapes[shape.id] = shape;
		box2d.add(shape);
	},
	
	box : function(options){
		options.width = options.width || 0.5 + Math.random() * 2;
		options.height = options.height || 0.5 + Math.random() * 2;
		var shape = new Box(options);
		shapes[shape.id] = shape;
		box2d.add(shape);
	}
};

var box2d = {
	add : function(shape){
		var bodyDef = this.create.bodyDef(shape);
		var body = world.CreateBody(bodyDef);
		if (shape.radius){
			fixDef.shape = new b2CircleShape(shape.radius);
		} else {
			fixDef.shape = new b2PolygonShape;
			fixDef.shape.SetAsBox(shape.width / 2, shape.height / 2);
		}
		body.CreateFixture(fixDef);
	},
	
	create : {
		world : function(){
			world = new b2World(
				new b2Vec2(0, 30),	// gravity vector
				true 				// do not allow sleep
			);
		},
		
		defaultFixture : function(){
			fixDef = new b2FixtureDef;
			fixDef.density = 1.0;
			fixDef.friction = 0.8;
			fixDef.restitution = 0.3;
		},
		
		bodyDef : function(shape){
			var bodyDef = new b2BodyDef;

			if (shape.isStatic == true) {
				bodyDef.type = b2Body.b2_staticBody;
			} else {
				bodyDef.type = b2Body.b2_dynamicBody;
			}
			bodyDef.position.x = shape.x;
			bodyDef.position.y = shape.y;
			bodyDef.userData = shape.id;
			bodyDef.angle = shape.angle || 0;

			return bodyDef;
		}
	},
	
	get : function(body){
		return {
			x : body.GetPosition().x,
			y : body.GetPosition().y,
			angle : body.GetAngle(),
			center : {
				x : body.GetWorldCenter().x,
				y : body.GetWorldCenter().y
			}
		};
	}
};


var bird = new Image();
bird.src = "demos/angry.png";

var background = new Image();
background.src = "demos/assets/bg02.png";


var loop = {
	update : function(){
		var stepRate = 1 / 60;
		world.Step(stepRate, 10, 10);
		world.ClearForces();

		for (var b = world.GetBodyList(); b; b = b.m_next){
			var id = b.GetUserData();
			if (b.IsActive() && id){
				shapes[id].update(box2d.get(b));
			}
		}
	},
	
	draw : function(){
		ctx.drawImage(background, 0, 0); //ctx.clearRect(0, 0, vw, vh);

		for (var i in shapes) {
			shapes[i].draw();
		}
	}
};


var helpers = {
	randomColor : function(){
		var letters = '0123456789ABCDEF'.split(''),
			color = '#';

		for (var i = 0; i < 6; i++) {
			color += letters[Math.round(Math.random() * 15)];
		}
		return color;
	}
};

/* Shapes down here */

var Shape = function(v){
	this.id = Math.round(Math.random() * 1000000);
	this.x = v.x || Math.random() * 23 + 1;
	this.y = v.y || 0;
	this.angle = v.angle || 0;
	this.color = v.color || helpers.randomColor();
	
	this.center = {
		x : null,
		y : null
	};
	
	this.isStatic = v.isStatic || false;

	this.update = function(options){
		this.angle = options.angle;
		this.center = options.center;
		this.x = options.x;
		this.y = options.y;
	};
};


var Circle = function(options){
	Shape.call(this, options);
	this.radius = options.radius || 1;

	this.draw = function(){
		ctx.save();
		
		ctx.translate(this.x * SCALE, this.y * SCALE);
		ctx.rotate(this.angle);
		ctx.translate(-(this.x) * SCALE, -(this.y) * SCALE);

		ctx.fillStyle = "rgba(44, 55, 255, 0.1)";
		ctx.beginPath();
		
		ctx.arc(this.x * SCALE, this.y * SCALE, this.radius * SCALE, 0, Math.PI * 2, true);
		ctx.closePath();
		
		ctx.fill();
		ctx.drawImage(bird, this.x * SCALE - 24, this.y * SCALE - 24);
		
		ctx.restore();
	};
};
Circle.prototype = Shape;

var Box = function(options){
	Shape.call(this, options);
	this.width = options.width || Math.random() * 2 + 0.5;
	this.height = options.height || Math.random() * 2 + 0.5;

	this.draw = function(){
		ctx.save();
		
		ctx.translate(this.x * SCALE, this.y * SCALE);
		ctx.rotate(this.angle);
		ctx.translate(-(this.x) * SCALE, -(this.y) * SCALE);
		ctx.fillStyle = this.color;
		ctx.fillRect((this.x - (this.width / 2)) * SCALE, (this.y - (this.height / 2)) * SCALE, this.width * SCALE, this.height * SCALE);
		ctx.restore();
	};
};

Box.prototype = Shape;

init.start();
