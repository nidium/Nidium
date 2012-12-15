var SCREEN_WIDTH = window.innerWidth;
var SCREEN_HEIGHT = window.innerHeight;

var container, stats;

var camera, scene, renderer;

var lightMesh;

var directionalLight, pointLight;

var mouseX = 0, mouseY = 0;

var windowHalfX = window.innerWidth / 2;
var windowHalfY = window.innerHeight / 2;

init();
animate();

function init() {
    // CAMERA

    camera = new THREE.PerspectiveCamera( 70, SCREEN_WIDTH / SCREEN_HEIGHT, 1, 10000 );
    camera.position.z = 1000;

    scene = new THREE.Scene();

    // LIGHTS

    var ambient = new THREE.AmbientLight( 0x020202 );
    scene.add( ambient );

    directionalLight = new THREE.DirectionalLight( 0xffffff );
    directionalLight.position.set( 1, 1, 0.5 ).normalize();
    scene.add( directionalLight );

    pointLight = new THREE.PointLight( 0xffaa00 );
    pointLight.position.set( 0, 0, 0 );
    scene.add( pointLight );

    sphere = new THREE.SphereGeometry( 100, 16, 8 );
    lightMesh = new THREE.Mesh( sphere, new THREE.MeshBasicMaterial( { color: 0xffaa00 } ) );
    lightMesh.scale.set( 0.05, 0.05, 0.05 );
    lightMesh.position = pointLight.position;
    scene.add( lightMesh );

    renderer = new THREE.WebGLRenderer();
    renderer.autoScaleCubemaps = false;// Not implemented by native
    renderer.setSize( SCREEN_WIDTH, SCREEN_HEIGHT );
    renderer.setFaceCulling( 0 );

    //document.addEventListener('mousemove', onDocumentMouseMove, false);
    canvas.onmousemove = onDocumentMouseMove;

    var r = "http://f.z.nf/three.js/examples/textures/cube/SwedishRoyalCastle/";
    var urls = [ r + "px.jpg", r + "nx.jpg",
        r + "py.jpg", r + "ny.jpg",
        r + "pz.jpg", r + "nz.jpg" ];

    var textureCube = THREE.ImageUtils.loadTextureCube( urls );

    var camaroMaterials = {

        body: [],
        chrome: new THREE.MeshLambertMaterial( { color: 0xffffff, envMap: textureCube } ),
        darkchrome: new THREE.MeshLambertMaterial( { color: 0x444444, envMap: textureCube } ),
        glass: new THREE.MeshBasicMaterial( { color: 0x223344, envMap: textureCube, opacity: 0.25, combine: THREE.MixOperation, reflectivity: 0.25, transparent: true } ),
        tire: new THREE.MeshLambertMaterial( { color: 0x050505 } ),
        interior: new THREE.MeshPhongMaterial( { color: 0x050505, shininess: 20 } ),
        black: new THREE.MeshLambertMaterial( { color: 0x000000 } )

    }

    camaroMaterials.body.push( [ "Orange", new THREE.MeshLambertMaterial( { color: 0xff6600, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.3 } ) ] );
    camaroMaterials.body.push( [ "Blue", new THREE.MeshLambertMaterial( { color: 0x226699, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.3 } ) ] );
    camaroMaterials.body.push( [ "Red", new THREE.MeshLambertMaterial( { color: 0x660000, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.5 } ) ] );
    camaroMaterials.body.push( [ "Black", new THREE.MeshLambertMaterial( { color: 0x000000, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.5 } ) ] );
    camaroMaterials.body.push( [ "White", new THREE.MeshLambertMaterial( { color: 0xffffff, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.5 } ) ] );

    camaroMaterials.body.push( [ "Carmine", new THREE.MeshPhongMaterial( { color: 0x770000, specular:0xffaaaa, envMap: textureCube, combine: THREE.MultiplyOperation } ) ] );
    camaroMaterials.body.push( [ "Gold", new THREE.MeshPhongMaterial( { color: 0xaa9944, specular:0xbbaa99, shininess:50, envMap: textureCube, combine: THREE.MultiplyOperation } ) ] );
    camaroMaterials.body.push( [ "Bronze", new THREE.MeshPhongMaterial( { color: 0x150505, specular:0xee6600, shininess:10, envMap: textureCube, combine: THREE.MixOperation, reflectivity: 0.5 } ) ] );
    camaroMaterials.body.push( [ "Chrome", new THREE.MeshPhongMaterial( { color: 0xffffff, specular:0xffffff, envMap: textureCube, combine: THREE.MultiplyOperation } ) ] );

    var loader = new THREE.BinaryLoader();
    loader.load( "http://f.z.nf/three.js/examples/obj/camaro/CamaroNoUv_bin.js", function( geometry ) { createScene( geometry, camaroMaterials ) } );

    //

    //window.addEventListener( 'resize', onWindowResize, false );

}

function onWindowResize() {

    windowHalfX = window.innerWidth / 2;
    windowHalfY = window.innerHeight / 2;

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();

    renderer.setSize( window.innerWidth, window.innerHeight );

}


function createScene( geometry, materials ) {

    var s = 75, m = new THREE.MeshFaceMaterial();

    m.materials[ 0 ] = materials.body[ 0 ][ 1 ]; // car body
    m.materials[ 1 ] = materials.chrome; // wheels chrome
    m.materials[ 2 ] = materials.chrome; // grille chrome
    m.materials[ 3 ] = materials.darkchrome; // door lines
    m.materials[ 4 ] = materials.glass; // windshield
    m.materials[ 5 ] = materials.interior; // interior
    m.materials[ 6 ] = materials.tire; // tire
    m.materials[ 7 ] = materials.black; // tireling
    m.materials[ 8 ] = materials.black; // behind grille

    var mesh = new THREE.Mesh( geometry, m );
    mesh.rotation.y = 1;
    mesh.scale.set( s, s, s );
    scene.add( mesh );

}

function onDocumentMouseMove(event) {
    mouseX = ( event.clientX * 2 - windowHalfX );
    mouseY = ( event.clientY * 2- windowHalfY );
}

//

function animate() {

    requestAnimationFrame( animate );

    render();

}

function render() {

    var timer = -0.0002 * Date.now();

    camera.position.x += ( mouseX - camera.position.x ) * .05;
    camera.position.y += ( - mouseY - camera.position.y ) * .05;

    camera.lookAt( scene.position );

    lightMesh.position.x = 1500 * Math.cos( timer );
    lightMesh.position.z = 1500 * Math.sin( timer );

    renderer.render( scene, camera );

}
