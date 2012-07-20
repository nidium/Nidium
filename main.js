canvas.fillStyle = "red";
canvas.strokeStyle = "black";
canvas.lineWidth = 2;

canvas.beginPath();  

canvas.arc(100, 100, 100, Math.PI, Math.PI*1.8, true);


canvas.stroke();

canvas.fillText("Fuck yeah, ça rox", 10, 10);
