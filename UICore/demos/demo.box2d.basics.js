/* -------------------------- */
/* Native (@) 2012 Stight.com */
/* -------------------------- */

load("core/plugins/box2d.inc.js");

Native.showFPS(true);
  
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
	b2DebugDraw = Box2D.Dynamics.b2DebugDraw,
	b2MouseJointDef = Box2D.Dynamics.Joints.b2MouseJointDef;

var world = new b2World(
	new b2Vec2(0, 50),	// gravity
	true				// allow sleep
);

var fixDef = new b2FixtureDef;
fixDef.density = 1.0;
fixDef.friction = 0.5;
fixDef.restitution = 0.3;

var bodyDef = new b2BodyDef;

//create ground
bodyDef.type = b2Body.b2_staticBody;

fixDef.shape = new b2PolygonShape;
fixDef.shape.SetAsBox(12, 0.25);

/* level 3 */
bodyDef.position.Set(12.5, 8);
bodyDef.angle = 0.1;
world.CreateBody(bodyDef).CreateFixture(fixDef);

/* level 2 */
bodyDef.position.Set(17.8, 15);
bodyDef.angle = -0.25;
world.CreateBody(bodyDef).CreateFixture(fixDef);

/* level 1 (ground) */
bodyDef.angle = 0;
bodyDef.position.Set(12.5, 768 / 30 - 2.5);
world.CreateBody(bodyDef).CreateFixture(fixDef);

/* left pane */
fixDef.shape.SetAsBox(2.2, 14);

bodyDef.position.Set(-1.8, 13);
world.CreateBody(bodyDef).CreateFixture(fixDef);

/* right pane */
bodyDef.position.Set(32.125, 13);
world.CreateBody(bodyDef).CreateFixture(fixDef);


//create some objects
bodyDef.type = b2Body.b2_dynamicBody;

for (var i = 0; i < 50; ++i) {
	if (Math.random() > 0.5) {
		fixDef.shape = new b2PolygonShape;
		fixDef.shape.SetAsBox(
			Math.random() + 0.1,	// half width
			Math.random() + 0.1 	// half height
		);
	} else {
		fixDef.shape = new b2CircleShape(
			Math.random() + 0.1 //radius
		);
	}
	bodyDef.position.x = Math.random() * 20;
	bodyDef.position.y = Math.random() * 7;
	world.CreateBody(bodyDef).CreateFixture(fixDef);
}

//setup debug draw
var debugDraw = new b2DebugDraw();

debugDraw.SetSprite(canvas);
debugDraw.SetDrawScale(30.0);
debugDraw.SetFillAlpha(0.5);
debugDraw.SetLineThickness(1.0);
debugDraw.SetFlags(b2DebugDraw.e_shapeBit | b2DebugDraw.e_jointBit);
world.SetDebugDraw(debugDraw);


//mouse
var mouseX, mouseY, mousePVec, isMouseDown, selectedBody, mouseJoint;

var canvasPosition = {
	x : 0,
	y : 0
};

function getBodyAtMouse() {
	mousePVec = new b2Vec2(mouseX, mouseY);
	var aabb = new b2AABB();
	
	aabb.lowerBound.Set(mouseX - 0.001, mouseY - 0.001);
	aabb.upperBound.Set(mouseX + 0.001, mouseY + 0.001);

	// Query the world for overlapping shapes.
	selectedBody = null;
	world.QueryAABB(getBodyCB, aabb);
	return selectedBody;
}

function getBodyCB(fixture) {
	if (fixture.GetBody().GetType() != b2Body.b2_staticBody) {
		if (fixture.GetShape().TestPoint(fixture.GetBody().GetTransform(), mousePVec)) {
			selectedBody = fixture.GetBody();
			
			return false;
		}
	}
	return true;
}

canvas.onmousedown = function(e){
	isMouseDown = true;
	mouseX = (e.x - canvasPosition.x) / 30;
	mouseY = (e.y - canvasPosition.y) / 30;
};

canvas.onmousemove = function(e){
	mouseX = (e.x - canvasPosition.x) / 30;
	mouseY = (e.y - canvasPosition.y) / 30;
};

canvas.onmouseup = function(){
	isMouseDown = false;
	mouseX = undefined;
	mouseY = undefined;
};


canvas.requestAnimationFrame(function(){

	if (isMouseDown && (!mouseJoint)) {
		var body = getBodyAtMouse();
		
		if (body) {
			var md = new b2MouseJointDef();
			
			md.bodyA = world.GetGroundBody();
			
			md.bodyB = body;
			md.target.Set(mouseX, mouseY);
			md.collideConnected = true;
			md.maxForce = 500.0 * body.GetMass();
			
			mouseJoint = world.CreateJoint(md);
			body.SetAwake(true);
		}
	}

	if (mouseJoint) {
		if (isMouseDown) {
			mouseJoint.SetTarget(new b2Vec2(mouseX, mouseY));
		} else {
			world.DestroyJoint(mouseJoint);
			mouseJoint = null;
		}
	}

	world.Step(1 / 60, 10, 10);
	world.DrawDebugData();
	
	world.ClearForces();

});


