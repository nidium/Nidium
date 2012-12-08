load("libs/wrapper.js");
load("libs/three.js");

var camera, scene, renderer;
var geometry, material, mesh;

init();
animate();

function init() {

    camera = new THREE.PerspectiveCamera( 75, window.innerWidth / window.innerHeight, 1, 10000 );
    camera.position.z = 500;

    scene = new THREE.Scene();

    geometry = new THREE.CubeGeometry( 200, 200, 200 );
    /*
    var materials = [];
    var colors = [0xffffff, 0x00ffff, 0xff00ff, 0xffff00, 0x0fffff, 0xff0fff, 0xffff0f];
    for (var i=0; i<6; i++) {
        var mat = new THREE.MeshBasicMaterial({color: colors[i]});
        materials.push(mat);
    }
    */
    material = new THREE.MeshBasicMaterial({ 
        map: THREE.ImageUtils.loadTexture("http://f.z.nf/crate.png")
    });

    mesh = new THREE.Mesh( geometry, material );
    scene.add( mesh );

    renderer = new THREE.WebGLRenderer();
    renderer.setSize( window.innerWidth, window.innerHeight );
}

function animate() {
    canvas.requestAnimationFrame( animate );

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.02;

    renderer.render( scene, camera );

}
