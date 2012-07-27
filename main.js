/*  canvas.fillRect(0,0,150,150);  
  canvas.translate(75,75);  
  
  // Create a circular clipping path  
  canvas.beginPath();  
  canvas.arc(0,0,60,0,Math.PI*2,true);  
  canvas.clip();  
  

    
  canvas.fillStyle = "blue";  
  canvas.fillRect(-75,-75,150,150);
*/
canvas.beginPath();
canvas.moveTo(20, 10);
canvas.lineTo(80, 10);
canvas.quadraticCurveTo(90, 10, 90, 20);
canvas.lineTo(90, 80);
canvas.quadraticCurveTo(90, 90, 80, 90);
canvas.lineTo(20, 90);
canvas.quadraticCurveTo(10, 90, 10, 80);
canvas.lineTo(10, 20);
canvas.quadraticCurveTo(10, 10, 20, 10);
canvas.stroke();