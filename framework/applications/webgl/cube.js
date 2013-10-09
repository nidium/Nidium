var ctx = window.canvas.getContext('2d');

ctx.fillStyle = "red";
ctx.fillRect(0, 0, 1024, 768);

window.innerWidth = 800;
window.innerHeight = 600;

var texture = new THREE.ImageUtils.loadTexture( 'img/checkerboard.jpg' );
var scene = new THREE.Scene();
var camera = new THREE.PerspectiveCamera(75, window.innerWidth/window.innerHeight, 0.1, 1000);
var c = new Canvas(window.innerWidth, window.innerHeight);
window.canvas.add(c);

var renderer = new THREE.WebGLRenderer({canvas:c});
    renderer.setClearColor( 0xFF00FF, 1 );
renderer.setSize(window.innerWidth, window.innerHeight);

var geometry = new THREE.CubeGeometry(1,1,1);
var material = new THREE.MeshBasicMaterial({map: texture});
var cube = new THREE.Mesh(geometry, material);
scene.add(cube);

camera.position.z = 5;

var render = function () {
    requestAnimationFrame(render);

    cube.rotation.x += 0.1;
    cube.rotation.y += 0.1;

    renderer.render(scene, camera);
};

render();
//Native.showFPS(true);
