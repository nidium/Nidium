  canvas.fillStyle = "red";

  canvas.beginPath();
  canvas.moveTo(30, 30);
  canvas.lineTo(150, 150);
  canvas.bezierCurveTo(60, 70, 60, 70, 70, 150);
  canvas.lineTo(30, 30);
  canvas.fill();