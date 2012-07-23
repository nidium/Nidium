
  var i = 0;
  var sin = Math.sin(Math.PI/6);
  var cos = Math.cos(Math.PI/6);
  canvas.translate(200, 200);
  var c = 0;
  for (i; i <= 12; i++) {
    c = Math.floor(255 / 12 * i);
    canvas.fillStyle = "rgb(" + c + "," + c + "," + c + ")";
    canvas.fillRect(0, 0, 100, 10);
    canvas.transform(cos, sin, -sin, cos, 0, 0);
  }
  
  canvas.setTransform(-1, 0, 0, 1, 200, 200);
  canvas.fillStyle = "rgba(255, 128, 255, 0.5)";
  canvas.fillRect(0, 50, 100, 100);
